#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "Defs.h"
#include <iostream>

using namespace std;

DBFile::DBFile()
{
    //dbfile->currentPage =0;
    this->isFileOpened = false;

}

DBFile::~DBFile()
{}

// Creates the file based on the .meta file.
// return 0 on failure and 1 on success
int DBFile::Create(const char *f_path, fType f_type, void *startup)
{
    if (this->isFileOpened)
        return 0;

    this->isFileOpened = true;
    ViewStateHandler::SetCurrentFilePath(f_path);
    switch (static_cast<fType>(f_type))
    {
        case 0:
            this->metaDataHandler = MetaDataHandler(f_path, heap, 0, 0, NULL);
            this->genericDbFile = new HeapDBFile();
            this->genericDbFile->file.Open(0, const_cast<char *>(f_path));
            break;
        case 1:
            typedef struct
            {
                OrderMaker *o;
                int l;
            } *pOrder;
            pOrder po = (pOrder) startup;
            this->metaDataHandler = MetaDataHandler(f_path, sorted, 100, po->l, po->o);
            this->genericDbFile = new SortedDBFile();
            this->genericDbFile->RelPath = f_path;
            this->genericDbFile->file.Open(0, const_cast<char *>(f_path));
            break;
            //case 2:
            //default: std::cout<<"The mode has not been implemented"<<endl;
    }
    return 1;
}


// This is done to load all the records in the database
void DBFile::Load(Schema &f_schema, const char *loadpath)
{
    if (!this->isFileOpened)
        return;
    this->genericDbFile->Load(f_schema, loadpath);
    return;
}

// Opens the .meta file
int DBFile::Open(const char *f_path)
{
    if (this->isFileOpened)
        return 0;
    this->metaDataHandler = MetaDataHandler(f_path);
    if (!this->metaDataHandler.Open())
    {
        std::cout << "We had some error: E(1)\n" << f_path << "\n";
        return 0;
    }
    this->isFileOpened = true;
    ViewStateHandler::SetCurrentFilePath(f_path);
    switch (this->metaDataHandler.GetType())
    {
        case heap:
            this->genericDbFile = new HeapDBFile();
            break;
        case sorted:
            this->genericDbFile = new SortedDBFile();
            this->genericDbFile->RelPath = f_path;
            this->genericDbFile->SetAttribute(this->metaDataHandler.GetOrderMaker(), this->metaDataHandler.GetRunLength());
            break;
    }
    this->genericDbFile->file.Open(1, const_cast<char *>(f_path));
    return 1;
}

// Moves to the first record in the first page.
void DBFile::MoveFirst()
{
    if (!this->isFileOpened || !this->genericDbFile)
        return;

    //this is done to write the record which are still in the cache and needs to be written in file before any things starts
    this->genericDbFile->MoveFirst();
}

// Closes the file
int DBFile::Close()
{
    if (!this->isFileOpened || !this->genericDbFile)
        return 0;
    this->isFileOpened = false;
    this->genericDbFile->Close();
    this->metaDataHandler.Close();
    return 1;
}

// Adds a new record to the file
int DBFile::Add(Record &rec)
{
    if (!this->isFileOpened || !this->genericDbFile)
        return 0;
    return this->genericDbFile->Add(rec);
}

// Gets the next record in the page
int DBFile::GetNext(Record &fetchme)
{
    if (!this->isFileOpened || !this->genericDbFile)
        return 0;
    return this->genericDbFile->GetNext(fetchme);
}

// Gets the next record according to the CNF expression
int DBFile::GetNext(Record &fetchme, CNF &cnf, Record &literal)
{
    if (!this->isFileOpened || !this->genericDbFile)
        return 0;
    return this->genericDbFile->GetNext(fetchme, cnf, literal);
}

// Gets the DBFile
GenericDBFile *DBFile::GetDBfile()
{
    return this->genericDbFile;
}

// Gets the current length of the file
int DBFile::GetLength()
{
    return this->genericDbFile -> GetLength();
}