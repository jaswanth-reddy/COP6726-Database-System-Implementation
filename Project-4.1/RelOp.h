#ifndef REL_OP_H
#define REL_OP_H
#pragma once

#include "DBFile.h"
#include "Record.h"
#include "Function.h"
#include <pthread.h>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include "Defs.h"
#include <vector>
#include <thread>
#include <memory>
#include <fstream>

using namespace std;

class RelationalOp
{
protected:
    int runLen;
    pthread_t worker;

public:
    // blocks the caller until the particular relational operator
    // has run to completion
    virtual void WaitUntilDone()
    {
        pthread_join(worker, NULL);
    }

    // tell us how much internal memory the operation can use
    void Use_n_Pages(int n)
    {
        runLen = n;
    }

    virtual void DoWork() = 0;
};

class SelectFile : public RelationalOp
{
private:
    DBFile *file;
    Pipe *outputPipe;
    CNF *cnf;
    Record *lit;
public:
    void Run(DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal);
    void DoWork() override;
};

class SelectPipe : public RelationalOp
{
private:
    Pipe *inputPipe, *outputPipe;
    CNF *cnf;
    Record *lit;
public:
    void Run(Pipe &inPipe, Pipe &outPipe, CNF &selOp, Record &literal);
    void DoWork() override;
};

class Project : public RelationalOp
{
private:
    Pipe *inputPipe, *outputPipe;
    int *attsToKeep;
    int numAttsIn, numAttsOut;
public:
    void Run(Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput);
    void DoWork() override;
};

class Join : public RelationalOp
{
private:
    Pipe *inputLeftPipe, *inputRightPipe, *outputPipe;
    CNF *cnf;
    Record *lit;
public:
    void Run(Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selOp, Record &literal);
    void DoWork() override;
};

class DuplicateRemoval : public RelationalOp
{
private:
    Pipe *inputPipe, *outputPipe;
    Schema *schema;
public:
    void Run(Pipe &inPipe, Pipe &outPipe, Schema &mySchema);
    void DoWork() override;
};

class Sum : public RelationalOp
{
private:
    Pipe *inputPipe, *outputPipe;
    Function *compute;
public:
    void Run(Pipe &inPipe, Pipe &outPipe, Function &computeMe);
    void DoWork() override;
};

class GroupBy : public RelationalOp
{
private:
    Pipe *inputPipe, *outputPipe;
    OrderMaker *order;
    Function *compute;
public:
    void Run(Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe);
    void DoWork() override;
};

class WriteOut : public RelationalOp
{
private:
    Pipe *inputPipe;
    FILE *file;
    Schema *schema;
public:
    void Run(Pipe &inPipe, FILE *outFile, Schema &mySchema);
    void DoWork() override;
};

#endif