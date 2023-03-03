#include "BigQ.h"

using namespace std;

OrderMaker BigQ::m_attOrder;
SortOrder BigQ::m_sortMode = Ascending; 


BigQ::BigQ(Pipe &inp_Pipe, Pipe &out_Pipe, OrderMaker &attri_order, int running_Length) {
    if (running_Length <= 0) {
        throw invalid_argument("In BigQ::BigQ, 'running_Length' cannot be zero or negative");
    }
    m_inputPipe = &inp_Pipe;
    m_outputPipe = &out_Pipe;
    m_attOrder = attri_order;
    m_runLength = running_Length; 
    m_numRuns = 0;

    m_sortTmpFilePath = "intermediate.tempor";

    typedef void * (*THREADFUNCPTR)(void *);
    pthread_t worker;
	pthread_create (&worker, NULL, (THREADFUNCPTR) &BigQ::externalSort, this);
    pthread_join (worker, NULL);
}

// Destructor for the BigQ
BigQ::~BigQ() {}


// Perform the Comparision using the ComparisionEngine
bool BigQ::compare4Sort(Record *left_side, Record *right_side) {
    
    // Refer the Comaprision Engine to compare the records.
    ComparisonEngine comparisionEng; 
    int comp_result = comparisionEng.Compare(left_side, right_side, &m_attOrder);
    
    // check the order for comparision
    if (m_sortMode == Ascending){ 
        return (comp_result > 0);  
    }
    else if (m_sortMode == Descending){ 
        return (comp_result < 0); 
    }   
}


// Perform the sorts using the sort function.
void BigQ::sortRecords(vector<Record*> &records, const OrderMaker &attri_order, SortOrder mode){
    sort(records.begin(), records.end(), compare4Sort);
}

//Function call to read the records from the Input File
void BigQ::readFromPipe(File &outputFile){
    
    vector<Record*> running_Buffer;
    Record current_Record;
    Page temporary_Page;
    int currentPgIndexFl = 0;
    Record* remainRec = NULL; 
    
    while (true){
        long long ttlRead_Bytes = 0;
        if (remainRec != NULL) {
            running_Buffer.push_back(remainRec);
            ttlRead_Bytes += remainRec->length();
            remainRec = NULL;
        }
        while (ttlRead_Bytes < PAGE_SIZE * m_runLength && m_inputPipe->Remove(&current_Record) != 0){
            Record* tempor = new Record();
            tempor->Copy(&current_Record);
            running_Buffer.push_back(tempor);
            ttlRead_Bytes += current_Record.length();
        }
        if (ttlRead_Bytes > PAGE_SIZE * m_runLength) {
            remainRec = running_Buffer.back();
            running_Buffer.pop_back();
            ttlRead_Bytes -= current_Record.length();
        }
        sortRecords(running_Buffer, m_attOrder, m_sortMode); 
        if (!running_Buffer.empty()) {
            m_runStartEndLoc.push_back(pair<int, int>(currentPgIndexFl, currentPgIndexFl)); 
            m_numRuns++;
        }
        while (!running_Buffer.empty()) {   
            temporary_Page.EmptyItOut();
            while (!running_Buffer.empty() && temporary_Page.Append(running_Buffer.back())){
                delete running_Buffer.back();
                running_Buffer.pop_back();
            }
            outputFile.AddPage(&temporary_Page, currentPgIndexFl);
            currentPgIndexFl++;
        }
        if (!m_runStartEndLoc.empty() && currentPgIndexFl > m_runStartEndLoc.back().second) {         
            m_runStartEndLoc[m_numRuns - 1].second = currentPgIndexFl;
        }      
        if (m_inputPipe->isClosed() && m_inputPipe->isEmpty() && remainRec == NULL) {
            return;
        }
    } 
}


// Perform the record push into the Heap Structure 
void BigQ::safeHeapPush(int ind, Record* pushit) {
    m_heap.push(pair<int, Record*>(ind, pushit));
    pushit = NULL;
}

// Perform the record pop from the block
int BigQ::nextPopBlock(vector<Block>& blockstopop) {
    if (blockstopop.empty() || m_heap.empty()) {
        return -1;
    }
    return m_heap.top().first;
}


// Function call to merge the blocks with the conditions to check if there is enough space for merging
void BigQ::mergeBlocks(vector<Block>& blockstomerge) {
    int nextblocktoPop = nextPopBlock(blockstomerge);
    if (nextblocktoPop == -1) {
        throw runtime_error("In BigQ::mergeBlocks, both the 'Blocks' & 'm_heap' should be non-empty initially to proceed further.");
    }
    while (nextblocktoPop >= 0) {
        Record* frontRecrd = new Record();
        blockstomerge[nextblocktoPop].getFrontRecord(*frontRecrd);
        blockstomerge[nextblocktoPop].popFrontRecord();
        m_outputPipe->Insert(frontRecrd);
        delete m_heap.top().second;
        m_heap.pop();
        if (blockstomerge[nextblocktoPop].getFrontRecord(*frontRecrd)) {
            safeHeapPush(nextblocktoPop, frontRecrd);
        }
        else {
            delete frontRecrd; 
            frontRecrd = NULL;
        }
        nextblocktoPop = nextPopBlock(blockstomerge);
    }
}


// Function to write the contents of the input file to the pipe
void BigQ::writeToPipe(File &inputFileToWrite){
    int blk_Size = (MaxMemorySize / m_numRuns) / PAGE_SIZE;
    
    // Throw error if size of block is not sufficient
    if (blk_Size < 1) {
        throw runtime_error("In BigQ::writeToPipe, has no enough memory to process the file!");
    }
    vector<Block> blks;

    // Iteratively perform the write operation over the # of runs
    for (int i = 0; i < m_numRuns; i++) {
        Block newBlk(blk_Size, m_runStartEndLoc[i], inputFileToWrite);

        blks.push_back(newBlk);

        Record* tempor = new Record();
        newBlk.getFrontRecord(*tempor);
        safeHeapPush(i, tempor);
    }
    mergeBlocks(blks);
    m_outputPipe->ShutDown();
}


// Function call to perform the External sorting for the large files which tend to be slow and can be improved with it.
void BigQ::externalSort(){
    
    // Temporaty file contents for performing the operations
    File tmpFileToExtSort;
    tmpFileToExtSort.Open(0, (char*)(m_sortTmpFilePath.c_str()));

    // Read the contents from the pipe
    readFromPipe(tmpFileToExtSort);
    tmpFileToExtSort.Close();
    tmpFileToExtSort.Open(1, (char*)(m_sortTmpFilePath.c_str()));

    // Write it back to the pipe
    writeToPipe(tmpFileToExtSort);
    tmpFileToExtSort.Close();
    tmpFileToExtSort.Open(0, (char*)(m_sortTmpFilePath.c_str()));
    tmpFileToExtSort.Close();

}

// Class reference to perform the necessary operations
Block::Block() {}

// Initialize the Block and assign the variables to the class reference.
Block::Block(int blk_size, pair<int, int> runStartEndPgIndex, File &inFile) {

    // Block size
    m_blockSize = blk_size;
    m_nextLoadPageIdx = runStartEndPgIndex.first;
    m_runEndPageIdx = runStartEndPgIndex.second;
    m_inputFile = inFile;

    // Load the page when there exists
    while (loadPage());
}

// Return a boolen variable to return True/ False if there are no more pages
bool Block::noMorePages() {
    return (m_nextLoadPageIdx >= m_runEndPageIdx);
}

// Return a boolean variable to check if the block is full
bool Block::isFull() {
    return (m_pages.size() >= m_blockSize);
}

// Return a boolean variable to check if the block is empty
bool Block::isEmpty() {
    return m_pages.empty();
}

// Load the block with the respective page
int Block::loadPage() {
    if (noMorePages() || isFull()) {
        return 0;
    }
    m_pages.push_back(new Page());
    m_inputFile.GetPage(m_pages.back(), m_nextLoadPageIdx);
    m_nextLoadPageIdx++;
    return 1;
}

// Obtain the front record from the Block
int Block::getFrontRecord(Record& front) {
    if (isEmpty()) {
        return 0;
    }
    return m_pages[0]->GetFirstNoConsume(front);
}

// Remove the front record from the Block
int Block::popFrontRecord() {
    
    // Check if it is empty
    if (isEmpty()) {
        return 0;
    }

    // Temporary Record to hold and perform operations
    Record tempor;

    // Obtain the pages from the temporary record.
    m_pages[0]->GetFirst(&tempor);

    // If pages are not empty
    if (m_pages[0]->IsEmpty()) {
        delete m_pages[0];
        m_pages[0] = NULL;
        m_pages.erase(m_pages.begin());
        loadPage();
    }
    return 1;
}