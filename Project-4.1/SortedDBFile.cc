#include "SortedDBFile.h"
#include <iostream>

using namespace std;

SortedDBFile::SortedDBFile()
{
    myOrder = new OrderMaker();
    in = new Pipe(BUFFERSIZE), out = new Pipe(BUFFERSIZE);
}

SortedDBFile::~SortedDBFile()
{}

// Adds a record to the input pipe
int SortedDBFile::Add(Record &addme)
{
    in->Insert(&addme);
}

// Creates an instance of Bigq class
void SortedDBFile::SetAttribute(OrderMaker *o, int run)
{
    myOrder = o;
    runLength = run;
    bigQ = new BigQ(*in, *out, *myOrder, runLength);
    int rs = pthread_create(&threadres, NULL, SortedDBFile::DumpPipeContentsToPage, (void *) this);
}

// Shuts down the input pipe and closes the file
int SortedDBFile::Close()
{
    in->ShutDown();
    pthread_join(threadres, NULL);
    check_write();
    file.Close();
    return 1;
}

// Gets the next record in the sorted file according to cnf
int SortedDBFile::GetNext(Record &fetchme, CNF &cnf, Record &literal)
{
    OrderMaker queryorder, cnforder;
    OrderMaker::queryOrderMaker(*myOrder, cnf, queryorder, cnforder);
    ComparisonEngine cmp;
    if (!PerformBinarySearch(fetchme, queryorder, literal, cnforder, cmp)) return 0;
    do
    {
        if (cmp.Compare(&fetchme, &queryorder, &literal, &cnforder)) return 0;
        if (cmp.Compare(&fetchme, &literal, &cnf)) return 1;
    } while (GenericDBFile::GetNext(fetchme));
    return 0;
    return GenericDBFile::GetNext(fetchme, cnf, literal);
};

// Performs a binary search according to the cnf expression and the sort order of the file
int SortedDBFile::PerformBinarySearch(Record &fetchme, OrderMaker &queryorder, Record &literal, OrderMaker &cnforder, ComparisonEngine &cmp)
{
    if (!GenericDBFile::GetNext(fetchme))
        return 0;
    int result = cmp.Compare(&fetchme, &queryorder, &literal, &cnforder);
    if (result > 0)
        return 0;
    else if (result == 0)
        return 1;
    int low = currentPage, high = file.GetLength() - 2, mid = (low + high) / 2;
    for (; low < mid; mid = (low + high) / 2)
    {
        file.GetPage(&readPage, mid);
        if (!GenericDBFile::GetNext(fetchme))
        {
            std::cout << "Empty Page Found";
            return 0;
        }
        result = cmp.Compare(&fetchme, &queryorder, &literal, &cnforder);
        if (result < 0) low = mid;
        else if (result > 0) high = mid - 1;
        else high = mid;
    }
    file.GetPage(&readPage, low);
    do
    {
        if (!GenericDBFile::GetNext(fetchme))
            return 0;
        result = cmp.Compare(&fetchme, &queryorder, &literal, &cnforder);
    } while (result < 0);
    return (result == 0);
}