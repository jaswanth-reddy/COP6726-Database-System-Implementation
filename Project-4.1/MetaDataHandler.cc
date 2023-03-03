#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "MetaDataHandler.h"
#include "Defs.h"
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

MetaDataHandler::MetaDataHandler()
{
}

MetaDataHandler::MetaDataHandler(const char *fPath)
{
    this->mPath = string(fPath) + ".meta";
    this->myOrder = new OrderMaker();
}

MetaDataHandler::MetaDataHandler(const char *fPath, fType m, int n, int run = 0, OrderMaker *o = NULL)
{
    this->mode = m;
    this->numPages = n;
    this->runLength = run;
    this->myOrder = o;
    this->mPath = string(fPath) + ".meta";
}

MetaDataHandler::~MetaDataHandler()
{
}

// Opens the .meta file and reads its contents
int MetaDataHandler::Open()
{
    ifstream ifile(this->mPath);
    if (!ifile)
        return 0;

    string line;
    getline(ifile, line);
    this->mode = static_cast<fType>(stoi(line));
    if (this->mode != heap)
    {
        ifile >> *this->myOrder;
        ifile >> this->runLength;
    }
    return 1;
}

// Closes the .meta file
int MetaDataHandler::Close()
{
    ofstream ofile(this->mPath);
    if (!ofile)
        return 0;

    ofile << this->mode << endl;
    if (this->mode != heap)
    {
        ofile << *this->myOrder;
        ofile << this->runLength << endl;
    }
    ofile.close();
    return 1;
}

// Increments number of pages
void MetaDataHandler::IncrementNumberOfPages()
{
    this->numPages++;
}

// returns number of pages
int MetaDataHandler::GetPages()
{
    return this->numPages;
}

// returns whether type is heap or sorted
fType MetaDataHandler::GetType()
{
    return this->mode;
}

// returns the sort order of file
OrderMaker *MetaDataHandler::GetOrderMaker()
{
    return this->myOrder;
}

// returns run length
int MetaDataHandler::GetRunLength()
{
    return this->runLength;
}