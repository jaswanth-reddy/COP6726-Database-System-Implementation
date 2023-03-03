#ifndef BIGQ_H
#define BIGQ_H

#include <algorithm>
#include <iostream>
#include <pthread.h>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "Defs.h"
#include "Pipe.h"
#include "Schema.h"

using namespace std;

enum SortOrder {Ascending, Descending};

typedef void * (*THREADFUNCPTR)(void *);

class DBlock {
    private:
        int m_blkSize;
        int m_nxtPgIndx;
        int m_EndPgIndx;
        File m_inFile;
        vector<Page*> m_vectpgs; 
        
    public:
        DBlock();
        DBlock(int siz, pair<int, int> FromToIdx, File &inFile);
        
        bool no_morePgs(); 

        bool IsBlkFull();

        bool IsEmpty();

        int LoadThePage();

        int GetFirstRec(Record& first);
        
        int popFrontRec();

};

class BigQ {

private:
    
    string m_sortTmpFilepth;
    int m_runLeng;
    int m_numbofRuns; 
    static OrderMaker m_attribOrd;
    
    Pipe *m_inpPipe; 
    Pipe *m_outPipe;

    static SortOrder m_sortingMode;
    

    vector< pair<int, int> > m_vecRunFirstLastLocation; 
    
    static bool CompRecForSort(Record *left, Record *right);
    
    struct compare4PQ {
        bool operator() (pair<int, Record*>& left, pair<int, Record*>& right) {
            return CompRecForSort(left.second, right.second);
        }
    };
    
    void sortRecords(vector<Record*> &recs, const OrderMaker &order, SortOrder mode);
    void readFromPipe(File &outputFile);

public:  

    priority_queue< pair<int, Record*>, vector< pair<int, Record*> >, compare4PQ > m_heap;
    
    void HeapToPush(int index, Record* recToPush);

    int nextBlockToPop(vector<DBlock>& blocks);

    void mergeBlks(vector<DBlock>& blocks);

    void wtToPipe(File &inputFile);
    
    void PerformExtSort();

    void Sort(vector<Record*> &recs, const OrderMaker &order, SortOrder mode);
    void GetNextRecFromPipe(File &outputFile);
 
    BigQ() {}
  
    BigQ (Pipe &inputPipe, Pipe &outputPipe, OrderMaker &order, int Len);
    
    ~BigQ();

};


#endif