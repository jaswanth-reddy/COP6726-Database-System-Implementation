#pragma once
#ifndef DBFILE_H
#define DBFILE_H

#include <iostream>
#include "HeapDBFile.h"
#include "SortedDBFile.h"
#include "GenericDBFile.h"
#include "ViewStateHandler.h"

class DBFile
{
    GenericDBFile *genericDbFile;
    bool isFileOpened;
    MetaDataHandler metaDataHandler;

public:
    DBFile();
    ~DBFile();
    int Create(const char *fpath, fType file_type, void *startup);
    int Open(const char *fpath);
    int Close();
    void Load(Schema &myschema, const char *loadpath);
    void MoveFirst();
    int Add(Record &addme);
    int GetNext(Record &fetchme);
    int GetNext(Record &fetchme, CNF &cnf, Record &literal);
    GenericDBFile *GetDBfile();
    int GetLength();
};

#endif
