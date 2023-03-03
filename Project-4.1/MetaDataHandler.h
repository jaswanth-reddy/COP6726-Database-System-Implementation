#pragma once

#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "Schema.h"
#include <string>

typedef enum
{
    heap, sorted, tree
} fType;

class MetaDataHandler
{

private:
    fType mode;
    int numPages;
    OrderMaker *myOrder;
    int runLength;
    string mPath;

public:
    MetaDataHandler();
    ~MetaDataHandler();
    MetaDataHandler(const char *fpath, fType m, int n, int r, OrderMaker *o);
    MetaDataHandler(const char *fpath);
    fType GetType();
    OrderMaker *GetOrderMaker();
    int GetRunLength();
    int Open();
    int Close();
    void IncrementNumberOfPages();
    int GetPages();
};