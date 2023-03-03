#include "RelOp.h"

void *BeginWorkerThread(void *args)
{
    ((RelationalOp *) args)->DoWork();
    return NULL;
}

int buffSize = 100;

void SelectFile::Run(DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal)
{
    file = &inFile;
    outputPipe = &outPipe;
    cnf = &selOp;
    lit = &literal;
    if (pthread_create(&worker, NULL, BeginWorkerThread, (void *) this))
    {
        std::cout << "Thread creation unsuccessful\n";
    }
}

void SelectFile::DoWork()
{
    file->MoveFirst();
    Record next;
    int i=0;
    cout << "\n";
    while(file->GetNext(next,*(cnf),*(lit)))
    {
//        i += 1;
//        if (i % 100 == 0)
//        cout << i << " records have been processed" << std::endl;
        outputPipe->Insert(&next);
    }
    outputPipe->ShutDown();
}

void SelectPipe::Run(Pipe &inPipe, Pipe &outPipe, CNF &selOp, Record &literal)
{
    inputPipe = &inPipe;
    outputPipe = &outPipe;
    cnf = &selOp;
    lit = &literal;
    if (pthread_create(&worker, NULL, BeginWorkerThread, (void *) this))
    {
        std::cout << "Thread creation unsuccessful\n";
    }
}

void SelectPipe::DoWork()
{
    ComparisonEngine comp;
    Record rec;
    while(inputPipe->Remove(&rec))
        if(comp.Compare(&rec,lit,cnf))
            outputPipe->Insert(&rec);
    outputPipe->ShutDown();
}

void Sum::Run(Pipe &inPipe, Pipe &outPipe, Function &computeMe)
{
    inputPipe = &inPipe;
    outputPipe = &outPipe;
    compute = &computeMe;
    if (pthread_create(&worker, NULL, BeginWorkerThread, (void *) this))
    {
        std::cout << "Thread creation unsuccessful\n";
    }
}

void Sum::DoWork()
{
    Record rec;
    double result=0.0;
    while(inputPipe->Remove(&rec))
    {
        int int_res=0;
        double dbl_res=0;
        compute->Apply(rec,int_res,dbl_res);
        result += (int_res + dbl_res);
    }
    Attribute DA = {"sum", Double};
    Schema sum_sch ("sum_sch", 1, &DA);
    stringstream ss;
    ss<<result<<"|";
    Record *rcd=new Record();
    rcd->ComposeRecord(&sum_sch, ss.str().c_str());
    outputPipe->Insert(rcd);
    outputPipe->ShutDown();
}

void DuplicateRemoval::Run(Pipe &inPipe, Pipe &outPipe, Schema &mySchema)
{
    inputPipe = &inPipe;
    outputPipe = &outPipe;
    schema = &mySchema;
    if (pthread_create(&worker, NULL, BeginWorkerThread, (void *) this))
    {
        std::cout << "Thread creation unsuccessful\n";
    }
}

void DuplicateRemoval::DoWork()
{
    OrderMaker sortOrder(schema);
    Pipe sorted(100);
    BigQ biq(*inputPipe, sorted, sortOrder, runLen);
    Record cur, next;
    ComparisonEngine comp;
    if(sorted.Remove(&cur))
    {
        while(sorted.Remove(&next))
        {
            if (comp.Compare(&cur, &next, &sortOrder))
            {
                outputPipe->Insert(&cur);
                cur.Consume(&next);
            }
        }
        outputPipe->Insert(&cur);
    }
    outputPipe->ShutDown();
}

void Project::Run(Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput)
{
    inputPipe = &inPipe;
    outputPipe = &outPipe;
    attsToKeep = keepMe;
    numAttsIn = numAttsInput;
    numAttsOut = numAttsOutput;
    if (pthread_create(&worker, NULL, BeginWorkerThread, (void *) this))
    {
        std::cout << "Thread creation unsuccessful\n";
    }
}

void Project::DoWork()
{
    Record tmpRcd;
    int i=0;
    while(inputPipe->Remove(&tmpRcd))
    {
        tmpRcd.Project(attsToKeep, numAttsOut, numAttsIn);
        outputPipe->Insert(&tmpRcd);
    }
    outputPipe->ShutDown();
}

void GroupBy::Run(Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe)
{
    inputPipe = &inPipe;
    outputPipe = &outPipe;
    order = &groupAtts;
    compute = &computeMe;
    if (pthread_create(&worker, NULL, BeginWorkerThread, (void *) this))
    {
        std::cout << "Thread creation unsuccessful\n";
    }
}

void GroupBy::DoWork()
{

}

void WriteOut::Run(Pipe &inPipe, FILE *outFile, Schema &mySchema)
{
    inputPipe = &inPipe;
    file = outFile;
    schema = &mySchema;
    if (pthread_create(&worker, NULL, BeginWorkerThread, (void *) this))
    {
        std::cout << "Thread creation unsuccessful\n";
    }
}

void WriteOut::DoWork()
{
    Attribute *atts = schema->GetAtts();
    int n = schema->GetNumAtts();
    Record rec;
    int cnt=1;
    while(inputPipe->Remove(&rec)){
        if(!(file))
        {
            std::cout<<"Write Out "<<cnt++<<"\n";
            rec.Print(schema);
        }
        else{
            //std::fstream fstr(file);
            fprintf(file, "%d: ", cnt++);
            char *bits = rec.bits;
            for (int i = 0; i < n; i++) {
                fprintf(file, "%s",atts[i].name);
                int pointer = ((int *) bits)[i + 1];
                fprintf(file, "[");
                if (atts[i].myType == Int) {
                    int *myInt = (int *) &(bits[pointer]);
                    fprintf(file, "%d",*myInt);
                } else if (atts[i].myType == Double) {
                    double *myDouble = (double *) &(bits[pointer]);
                    fprintf(file, "%f", *myDouble);
                } else if (atts[i].myType == String) {
                    char *myString = (char *) &(bits[pointer]);
                    fprintf(file, "%s", myString);
                }
                fprintf(file, "]");
                if (i != n - 1) {
                    fprintf(file, ", ");
                }
            }
            fprintf(file, "\n");
            //fflush(file);
        }
    }
    if(file)
        fclose(file);
}

void Join::Run(Pipe &inputPipeL, Pipe &inputPipeR, Pipe &outPipe, CNF &selOp, Record &literal)
{
    inputLeftPipe = &inputPipeL;
    inputRightPipe = &inputPipeR;
    outputPipe = &outPipe;
    cnf = &selOp;
    lit = &literal;
    if (pthread_create(&worker, NULL, BeginWorkerThread, (void *) this))
    {
        std::cout << "Thread creation unsuccessful\n";
    }
}

void Join::DoWork()
{
    int count = 0;

    int leftNumAtts, rightNumAtts;
    int totalNumAtts;
    int *attsToKeep;

    ComparisonEngine comp;

    Record *fromLeft = new Record();
    Record *fromRight = new Record();

    OrderMaker leftOrder, rightOrder;

    cnf->GetSortOrders(leftOrder, rightOrder);

    if (leftOrder.numAtts > 0 && rightOrder.numAtts > 0 && leftOrder.numAtts == rightOrder.numAtts)
    {
        Pipe left(buffSize);
        Pipe right(buffSize);

        //cout << "bigq right size" << std::endl;
        BigQ* bigqRight = new BigQ(*inputRightPipe, right, rightOrder, 100);
        //cout << "bigq left size" << std::endl;
        BigQ* bigqLeft = new BigQ(*inputLeftPipe, left, leftOrder, 100);


        //cout << "Inserted into pipes" << endl;

        left.Remove(fromLeft);
        right.Remove(fromRight);

        leftNumAtts = fromLeft->GetLength();
        rightNumAtts = fromRight->GetLength();
        totalNumAtts = leftNumAtts + rightNumAtts;
        attsToKeep = new int[totalNumAtts];

        for (int i = 0; i < leftNumAtts; i++)
            attsToKeep[i] = i;

        for (int i = 0; i < rightNumAtts; i++)
            attsToKeep[leftNumAtts + i] = i;

        vector < Record * > vecL;
        vector < Record * > vecR;

        Record *rcd1 = new Record();
        rcd1->Consume(fromLeft);
        Record *rcd2 = new Record();
        rcd2->Consume(fromRight);

        vecL.push_back(rcd1);
        vecR.push_back(rcd2);

        int cL = 1, cR = 1;
        while (right.Remove(fromRight))
        {
            Record *rcd2 = new Record();
            rcd2->Consume(fromRight);
            vecR.push_back(rcd2);
//            cR++;
//            if (cR % 800000 ==0)
//                break;
        }

        while (left.Remove(fromLeft))
        {
            Record *rcd1 = new Record();
            rcd1->Consume(fromLeft);
            vecL.push_back(rcd1);
//            cL++;
//            if (cL % 10000 ==0)
//                break;
        }

       // cout << "No. of recs inserted into vecL = " << vecL.size() << endl;
       // cout << "No. of recs inserted into vecR = " << vecR.size() << endl;

        Record *lr = new Record(), *rr = new Record(), *jr = new Record();

        int l = 0, r = 0, index = 0;
        for (auto itL :vecL)
        {
            lr->Consume(itL);
           /* l++;
            if (l % 1000 == 0)
                cout << l << " recs processed" << endl;*/
            //index = 0;
            for (auto itR: vecR)
            {
                //index ++;
                if (comp.Compare(lr, itR, lit, cnf))
                {
//                    if (r == 58560)
//                    {
//                        cout << "Index when broken is " << index << endl;
//                        continue;
//                    }
                    rr->Copy(itR);
                    jr->MergeRecords(lr, rr, leftNumAtts, rightNumAtts, attsToKeep, leftNumAtts + rightNumAtts,
                                     leftNumAtts);
                    outputPipe->Insert(jr);
//                    r++;
//                    if (r % 10000 == 0)
//                        cout << r << "recs merged" << endl;
                }
            }
            //cout << "Left pipe rec " << l << " is done." << endl;
        }

        for(auto it:vecL)
            if(!it)
                delete it;
        vecL.clear();
        for(auto it : vecR)
            if(!it)
                delete it;
        vecR.clear();

    }
    else
    {
       // cout << "Inside else in Join: " << endl;
        char fileName[100];
        sprintf(fileName, "tempfile.tmp");

        HeapDBFile dbFile;
        dbFile.Create(fileName);

        bool isDone = false;

        if (!inputLeftPipe->Remove(fromLeft))
            isDone = true;
        else
            leftNumAtts = fromLeft->GetLength();

        if (!inputRightPipe->Remove(fromRight))
            isDone = true;
        else
        {
            rightNumAtts = fromRight->GetLength();
            totalNumAtts = leftNumAtts + rightNumAtts;
            attsToKeep = new int[totalNumAtts];

            for (int i = 0; i < leftNumAtts; i++)
                attsToKeep[i] = i;

            for (int i = 0; i < rightNumAtts; i++)
                attsToKeep[leftNumAtts + i] = i;
        }

        if (!isDone)
        {
            do
            {
                dbFile.Add(*fromLeft);
            } while (inputLeftPipe->Remove(fromLeft));

            int r = 0, l = 0;
            do
            {
                dbFile.MoveFirst();
                Record *newRec = new Record();
                r++;
                while (dbFile.GetNext(*fromLeft))
                {
                    if (comp.Compare(fromLeft, fromRight, lit, cnf))
                    {
                        newRec->MergeRecords(fromLeft, fromRight, leftNumAtts, rightNumAtts, attsToKeep, totalNumAtts, leftNumAtts);
                        outputPipe->Insert(newRec);
                    }
                }
                delete newRec;
//                if (r % 100 == 0)
//                    cout << r << " recs processed" << endl;
            } while (inputRightPipe->Remove(fromRight));
        }
        dbFile.Close();
        //remove("tempfile.tmp");
    }
    outputPipe->ShutDown();
    delete fromLeft;
    delete fromRight;
    delete attsToKeep;
}