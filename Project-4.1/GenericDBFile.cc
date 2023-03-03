#include "GenericDBFile.h"

// Move to the first record on the first page.
void GenericDBFile::MoveFirst()
{
    check_write();
    this->readPage.EmptyItOut(); // to remove anything in the array
    this->file.GetPage(&this->readPage, this->currentPage = 0);
}

// Closes the file
int GenericDBFile::Close()
{
    check_write();
    this->file.Close();
    return 1;
}

// Fetches the next record on that page. If page is full, retrieves first record from next page.
// Returns 1 on successful retrieval, otherwise, returns 0.
int GenericDBFile::GetNext(Record &fetchme)
{
    check_write(); //to check if we need to write before getNext in case some records have been written in the file
    while (!this->readPage.GetFirst(&fetchme))
    {
        if (++this->currentPage >= this->file.GetLength() - 1)
            return 0;
        this->file.GetPage(&this->readPage, this->currentPage);
    }
    return 1;
}

// Retrives next record according to the given CNF expression
int GenericDBFile::GetNext(Record &fetchme, CNF &cnf, Record &literal)
{
    ComparisonEngine comp;
    while (GetNext(fetchme))
    {
        if (comp.Compare(&fetchme, &literal, &cnf))
        {
            return 1;
        }
    }
    return 0;
}

// Load the file with respect to the appropriate schema
void GenericDBFile::Load(Schema &myschema, const char *loadpath)
{
    FILE *fileLoad = fopen(loadpath, "r");\
    Record pull;
    while (pull.SuckNextRecord(&myschema, fileLoad))
        Add(pull);
}

// Returns the file
File GenericDBFile::GetFile()
{
    return this->file;
}

// Returns the current length of the file
int GenericDBFile::GetLength()
{
    return this->file.GetLength();
}