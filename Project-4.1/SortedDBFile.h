#pragma once

#include "GenericDBFile.h"
#include "Record.h"
#include "Pipe.h"
#include "BigQ.h"
#include "Schema.h"

class BigQ;

class SortedDBFile : public GenericDBFile
{
    static const size_t BUFFERSIZE = 100;
    OrderMaker *myOrder;
    int runLength;
    pthread_t threadres;
    BigQ *bigQ;

    static void *DumpPipeContentsToPage(void *args)
    {
        SortedDBFile *sb = (SortedDBFile *) args;
        Record rec;
        while (sb->out->Remove(&rec))
        {
            if (!sb->writePage.Append(&rec))
            {
                int pos = sb->file.GetLength() == 0 ? 0 : sb->file.GetLength() - 1;
                sb->file.AddPage(&sb->writePage, pos);
                sb->writePage.EmptyItOut();
                sb->writePage.Append(&rec);
            }
        }
        return NULL;
    }

public:
    Pipe *in, *out;
    const char *relPath;
    SortedDBFile();
    ~SortedDBFile();
    int Close() override;
    int GetNext(Record &fetchme, CNF &cnf, Record &literal) override;
    int Add(Record &addme) override;
    void SetAttribute(OrderMaker *o, int run) override;
    int PerformBinarySearch(Record &fetchme, OrderMaker &queryorder, Record &literal, OrderMaker &cnforder, ComparisonEngine &cmp);
};