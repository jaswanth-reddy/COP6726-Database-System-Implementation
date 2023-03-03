#pragma once

#include "GenericDBFile.h"
#include "Record.h"
#include <string.h>

class HeapDBFile : public GenericDBFile
{
public:
    HeapDBFile();
    ~HeapDBFile();
    int Add(Record &addme) override;
    int Create(char *f_path);
};