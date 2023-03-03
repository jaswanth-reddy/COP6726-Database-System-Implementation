#pragma once

#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "MetaDataHandler.h"

class GenericDBFile
{
protected:
    File file;
    Page writePage;
    Page readPage;
    int currentPage;

public:
    GenericDBFile()
    {
        currentPage = 0;
    }

    ~GenericDBFile()
    {}
    friend class DBFile;
    virtual int Add(Record &addme) {};
    //int Create(const char *fpath, fType file_type, void *startup);
    virtual int Close();
    void Load(Schema &myschema, const char *loadpath);
    void MoveFirst();
    int GetNext(Record &fetchme);
    virtual int GetNext(Record &fetchme, CNF &cnf, Record &literal);
    virtual void SetAttribute(OrderMaker *o, int run) {};
    const char *RelPath;
    File GetFile();
    int GetLength();
    void check_write()
    {
        if (!writePage.empty())
        {
            int pos = !file.GetLength() ? 0 : file.GetLength() - 1;
            file.AddPage(&writePage, pos);
            writePage.EmptyItOut();
        }
    }
};