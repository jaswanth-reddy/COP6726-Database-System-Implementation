#include "BigQ.h"

using namespace std;

OrderMaker BigQ::m_attribOrd;
SortOrder BigQ::m_sortingMode = Ascending; 

DBlock::DBlock() {}

DBlock::DBlock(int isize, pair<int, int> FirstLastIndex, File &finputFile) {
    m_blkSize = isize;
    m_inFile = finputFile;
    m_nxtPgIndx = FirstLastIndex.first;
    m_EndPgIndx = FirstLastIndex.second;
    while (LoadThePage());
}

bool DBlock::no_morePgs() {
    return (m_nxtPgIndx >= m_EndPgIndx);
}

bool DBlock::IsBlkFull() {
    return (m_vectpgs.size() >= m_blkSize);
}

bool DBlock::IsEmpty() {
    return m_vectpgs.empty();
}

int DBlock::LoadThePage() {
    if (no_morePgs() || IsBlkFull()) {
        return 0;
    }
    m_vectpgs.push_back(new Page());
    m_inFile.GetPage(m_vectpgs.back(), m_nxtPgIndx);
    m_nxtPgIndx++;
    return 1;
}

int DBlock::GetFirstRec(Record& front) {
    if (IsEmpty()) {
        return 0;
    }
    return m_vectpgs[0]->GetFirstNoConsume(front);
}

int DBlock::popFrontRec() {
    if (IsEmpty()) {
        return 0;
    }
    
    Record tmp;
    m_vectpgs[0]->GetFirst(&tmp);
    
    if (m_vectpgs[0]->IsEmpty()) {

        delete m_vectpgs[0];
        m_vectpgs[0] = NULL;
        m_vectpgs.erase(m_vectpgs.begin());
        LoadThePage();
    }

    return 1;
}

BigQ::BigQ(Pipe &inputPipe, Pipe &outputPipe, OrderMaker &order, int Len) {
    if (Len <= 0) {
        throw invalid_argument("In BigQ::BigQ, 'runLen' cannot be zero or negative");
    }
    m_numbofRuns = 0;
    m_attribOrd = order;
    m_runLeng = Len; 
    m_inpPipe = &inputPipe;
    m_outPipe = &outputPipe;

    m_sortTmpFilepth = "intermediate.tmp";

    pthread_t worker;
	pthread_create (&worker, NULL, (THREADFUNCPTR) &BigQ::PerformExtSort, this);

}

BigQ::~BigQ() {}

bool BigQ::CompRecForSort(Record *left, Record *right) {

    ComparisonEngine compEngine; 
    int res = compEngine.Compare(left, right, &m_attribOrd);
    if (m_sortingMode == Ascending){ 
        return (res > 0);  
    }
    else if (m_sortingMode == Descending){ 
        return (res < 0); 
    }
    
}

void BigQ::sortRecords(vector<Record*> &recs, const OrderMaker &order, SortOrder mode){
    sort(recs.begin(), recs.end(), CompRecForSort);
}

void BigQ::readFromPipe(File &outputFile){
    
    vector<Record*> runBuffer;
    Record curRecord;
    Page tmpPage;
    int curPageIdxInFile = 0;

    Record* remainRecord = NULL; 

    while (true){

        long long totalReadBytes = 0;
        
        if (remainRecord != NULL) {
            runBuffer.push_back(remainRecord);
            totalReadBytes += remainRecord->length();
            remainRecord = NULL;
        }

        while (totalReadBytes < PAGE_SIZE * m_runLeng && m_inpPipe->Remove(&curRecord) != 0){
            Record* tmp = new Record();
            tmp->Copy(&curRecord);
            runBuffer.push_back(tmp);
            totalReadBytes += curRecord.length();
        }
        
        if (totalReadBytes > PAGE_SIZE * m_runLeng) {
            remainRecord = runBuffer.back();
            runBuffer.pop_back();
            totalReadBytes -= curRecord.length();
        }

        sortRecords(runBuffer, m_attribOrd, m_sortingMode); 
        
        if (!runBuffer.empty()) {
            m_vecRunFirstLastLocation.push_back(pair<int, int>(curPageIdxInFile, curPageIdxInFile)); 
            m_numbofRuns++;
        }

        while (!runBuffer.empty()) {   
            tmpPage.EmptyItOut();
            while (!runBuffer.empty() && tmpPage.Append(runBuffer.back())){
                delete runBuffer.back();
                runBuffer.pop_back();
            }
            outputFile.AddPage(&tmpPage, curPageIdxInFile);
            curPageIdxInFile++;
        }

        if (!m_vecRunFirstLastLocation.empty() && curPageIdxInFile > m_vecRunFirstLastLocation.back().second) {            
            m_vecRunFirstLastLocation[m_numbofRuns - 1].second = curPageIdxInFile;
        }      

        if (m_inpPipe->isClosed() && m_inpPipe->isEmpty() && remainRecord == NULL) {
            return;
        }

    } 

}

void BigQ::HeapToPush(int index, Record* recordtobePush) {
    m_heap.push(pair<int, Record*>(index, recordtobePush));
    recordtobePush = NULL;
}

int BigQ::nextBlockToPop(vector<DBlock>& blocks) {
    if (blocks.empty() || m_heap.empty()) {
        return -1;
    }
    
    return m_heap.top().first;
}

void BigQ::mergeBlks(vector<DBlock>& blocks) {
    int nextPop = nextBlockToPop(blocks);
    if (nextPop == -1) {
        throw runtime_error("In BigQ::mergeBlocks, 'blocks' and 'm_heap' must be both non-empty initially.");
    }
    
    while (nextPop >= 0) {

        Record* front = new Record();
        
        blocks[nextPop].GetFirstRec(*front);
        blocks[nextPop].popFrontRec();

        m_outPipe->Insert(front);

        delete m_heap.top().second;
        m_heap.pop();

        if (blocks[nextPop].GetFirstRec(*front)) {
            HeapToPush(nextPop, front);
        }
        else {
            delete front; 
            front = NULL;
        }
        nextPop = nextBlockToPop(blocks);
    }

}

void BigQ::wtToPipe(File &inputFile){
    int blockSize = (MaxMemorySize / m_numbofRuns) / PAGE_SIZE;
    if (blockSize < 1) {
        throw runtime_error("In BigQ::writeToPipe, no enough memory!");
    }

    vector<DBlock> blocks;
    for (int i = 0; i < m_numbofRuns; i++) {
        DBlock newBlock(blockSize, m_vecRunFirstLastLocation[i], inputFile);
        blocks.push_back(newBlock);

        Record* tmp = new Record();
        newBlock.GetFirstRec(*tmp);
        HeapToPush(i, tmp);
    }
    
    mergeBlks(blocks);

    m_outPipe->ShutDown();

}

void BigQ::PerformExtSort(){
    File temp;
    temp.Open(0, (char*)(m_sortTmpFilepth.c_str()));
    readFromPipe(temp);
    temp.Close();
    temp.Open(1, (char*)(m_sortTmpFilepth.c_str()));
    wtToPipe(temp);
    temp.Close();
    temp.Open(0, (char*)(m_sortTmpFilepth.c_str()));
    temp.Close();
}


