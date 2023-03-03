#include "HeapDBFile.h"
#include <iostream>

using namespace std;

HeapDBFile::HeapDBFile()
{
}

HeapDBFile::~HeapDBFile()
{
}

/*
    Creates the corresponding database file from the path mentioned below
    Returns 1 on successful open; 0 on failed
*/
int HeapDBFile::Create(char *f_path)
{
    try
    {
        char* fileName = strdup(f_path);
        file.Open(0, fileName);
        cout << "File created successfully\n";
    }
    catch (...)
    {
        cerr << "Error occured while creating DBFileHeap. \n";
        return 0;
    }
    return 1;
}

// Adds a record to the file
int HeapDBFile::Add(Record &rec)
{
    if (!writePage.Append(&rec))
    {
        int pos = file.GetLength() == 0 ? 0 : file.GetLength() - 1;
        file.AddPage(&writePage, pos);
        writePage.EmptyItOut();
        writePage.Append(&rec);
    }
    return 1;
}

