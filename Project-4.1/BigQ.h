#ifndef BIGQ_H
#define BIGQ_H
#pragma once

#include <pthread.h>
#include <iostream>
#include "Pipe.h"
#include "File.h"
#include "Record.h"
#include <vector>
#include <stdio.h>
#include "ComparisonEngine.h"
#include <algorithm>
#include "DBFile.h"
#include <string>
#include <queue>
#include <string.h>
#include <fstream>
#include "ViewStateHandler.h"

//class HeapDBFile;

using namespace std;

class BigQ
{
    Pipe &inputPipe;
    Pipe &outputPipe;
    pthread_t threadID;
    OrderMaker &orderMaker;
    int runLength;
    string tempFile;
    File file;
    int noOfRecordsPerPage;

    std::string random_string( size_t length )
    {
        auto randchar = []() -> char
        {
            const char charset[] =
                    "0123456789"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[ rand() % max_index ];
        };
        std::string str(length,0);
        std::generate_n( str.begin(), length, randchar );
        return str;
    }

public:

    BigQ(Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen);

    ~BigQ();

    int OpenFile();

    int CloseFile();

//	bool CompareRecords(const Record *p, const Record *q);
    //bool ComparePairOfRecords(pair<int, Record *> p, pair<int, Record *> q);
    static void *SortAndDumpToPipe(void *bigQ);
};

#endif