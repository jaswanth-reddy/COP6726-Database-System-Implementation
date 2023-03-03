#include "BigQ.h"
#include <ctime>

BigQ::BigQ(Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen) : inputPipe(in), outputPipe(out), orderMaker(sortorder)
{
    this->runLength = runlen;
    //this->SetNoOfRecordsPerPage();
    this->OpenFile();
    int rs = pthread_create(&threadID, NULL, BigQ::SortAndDumpToPipe, (void *)this);
    if (rs)
    {
        printf("Error creating worker thread! Return %d\n", rs);
        exit(-1);
    }
}

/*
    Opens database file.
    Returns 1 on success; 0 on failure
*/
int BigQ::OpenFile()
{
    try
    {
        std::string savefile = (random_string(15) + "temp.txt");
        auto f_path = const_cast<char *>(savefile.c_str());
        this->file.Open (0, f_path);
       // cout << "File present at path: " << std::string(f_path) << " is now opened. \n";
//        this->tempFile="temp1234.tmp";
//        auto f_path = const_cast<char *>(this->tempFile.c_str());
//        if(!std::ifstream(strdup(f_path)))
//        {
//            cerr << "ERROR: File does not exist or could not be opened" << std::endl;
//            return 0;
//        }
//        char myFileName[]="tpmmsFile";
//        this->file.Open(0, myFileName);

        //char *fileNamePointer = new char[100];
        //this->tempFile = (random_string(15) + "temp.txt");
        //sprintf (fileNamePointer, this>tempFile.c_str());
        //auto f_path = const_cast<char *>(this->tempFile.c_str());
        //this->file.Open (0, f_path);
        //cout << "File present at path: " << std::string(f_path) << " is now opened. \n";
    }
    catch (exception ex)
    {
        cerr << ex.what();
        return 0;
    }
    return 1;
}

/*
    Closes database file.
    Returns 1 on success; 0 on failure
*/
int BigQ::CloseFile()
{
    try
    {
        //auto f_path = const_cast<char *>(this->tempFile.c_str());
        //ifstream file(f_path);
        //if(file.is_open())
        //    this->file.Close();
//        else
//        {
//            cerr << "In BigQ::Close(), ERROR: File does not exist" << std::endl;
//            return 0;
//        }

    }
    catch (exception e)
    {
        cerr << e.what();
        return 0;
    }
    return 1;
}

BigQ::~BigQ()
{
}

void *BigQ::SortAndDumpToPipe(void *Big)
{
    BigQ *bigQ = (BigQ *) Big;
    OrderMaker o1 = bigQ->orderMaker;
    OrderMaker o2 = bigQ->orderMaker;

    auto comp = [&](Record *p, Record *q)
    {
        ComparisonEngine comp;
        return comp.Compare(p, q, &o1) < 0;
    };

    auto comp1 = [&](pair<int, Record *> p, pair<int, Record *> q)
    {
        ComparisonEngine comp;
        return comp.Compare(p.second, q.second, &o2) >= 0;
    };

    Record temp;
    Page *writePage = new Page();
    size_t curLength = 0;
    int numPages = 0;
    vector<Record *> result; //this will be used to sort the values internally within reach run;
    vector<int> pageCounter;
    int count = 0;
    int recordCount = 0;
    while ((bigQ->inputPipe).Remove(&temp))
    {
        char *temp_bits=temp.GetBits();
//        if (recordCount <= (bigQ->noOfRecordsPerPage))// * bigQ->runLength))
        if(curLength+((int *)temp_bits)[0] < (PAGE_SIZE)*(bigQ->runLength))
        {
            //cout << "Processing " << recordCount << " record now" << endl;
            Record *rec = new Record();
            rec->Consume(&temp);
            result.push_back(rec);
            ++recordCount;
            ++count;
            curLength += ((int *)temp_bits)[0];
        }
        else
        {
            //cout << "Processed " << recordCount << " records." << endl;
            //cout << "Going to sort now." << endl;
            std::sort(result.begin(), result.end(), comp);
            pageCounter.push_back(numPages);
            for (auto i:result)
            {
                int res = writePage->Append(i);
                if (!res)
                {
                    int pos = bigQ->file.GetLength();
                    pos = (pos == 0 ? 0 : (pos - 1));
                    bigQ->file.AddPage(writePage, pos);
                    writePage->EmptyItOut();
                    writePage->Append(i);
                    numPages++;
                }
            }
            if (!writePage->empty())
            {
                int position = bigQ->file.GetLength();
                bigQ->file.AddPage(writePage, (!position) ? 0 : (position - 1));
                writePage->EmptyItOut();
                numPages++;
            }

            for (auto i:result)
                delete i;
            result.clear();
            Record *rec = new Record();
            rec->Consume(&temp);
            result.push_back(rec);
            recordCount = 1;
            count ++;
            curLength = ((int *)temp_bits)[0];
        }
    }
    if (!result.empty())
    {
        // If its not the last record in a page, have to append records to a new page.
        std::sort(result.begin(), result.end(), comp);
        pageCounter.push_back(numPages);
        for (auto i:result)
        {
            int res = writePage->Append(i);
            if (!res)
            {
                int pos = bigQ->file.GetLength();
                pos = (pos == 0 ? 0 : (pos - 1));
                bigQ->file.AddPage(writePage, pos);
                writePage->EmptyItOut();
                writePage->Append(i);
                numPages++;
            }
        }
        if (!writePage->empty())
        {
            int position = bigQ->file.GetLength();
            bigQ->file.AddPage(writePage, (!position) ? 0 : (position - 1));
            writePage->EmptyItOut();
            numPages++;
        }

        for (auto i:result)
            delete i;
        result.clear();
        pageCounter.push_back(numPages);
    }

   // cout << "Inside Bigq: No. of recs processed = " << count << endl;
    // Load to Priority Queue and dump to output pipe

    vector<Page *> pageKeeper;
    priority_queue<pair<int, Record *>, vector<pair<int, Record *>>, decltype(comp1)> pq(comp1);
    for (int i = 0; i < pageCounter.size() - 1; i++)
    {
        Page *temp_P = new Page();
        bigQ->file.GetPage(temp_P, pageCounter[i]);
        Record *temp_R = new Record();
        temp_P->GetFirst(temp_R);
        //cout << "Before pushing into Priority Queue, i = " << i << endl;
        pq.push(make_pair(i, temp_R));
        pageKeeper.push_back(temp_P);
    }
    //cout << "Size of Priority Queue = " << pq.size() << endl;
    //cout << "Size of pageKeeper = " << pageKeeper.size() << endl;

    //cout << "PageCounter = ";
//    for (std::vector<int>::const_iterator i = pageCounter.begin(); i != pageCounter.end(); ++i)
//        std::cout << *i << ' ';

    //cout << endl;

    vector<int> pageChecker(pageCounter);
    //cout << "Dumping records to output pipe" << endl;
    int c = 0;
    while (!pq.empty())
    {
        int temp_I = pq.top().first;
        Record *temp_R = pq.top().second;
        pq.pop();
        //cout << "Before pop(), top.First = " << top.first << endl;
        //cout << "Before pop(), top.second = " << top.second << endl;
        c++;
//        if (c % 1 == 0 || c % 1000 == 0 || c % 10000 == 0)
        bigQ->outputPipe.Insert(temp_R);
        //cout << "After pop(), top.First = " << top.first << endl;
        //cout << "After pop(), top.second = " << top.second << endl;
        //cout << "PageKeeper[top.first] = " << pageKeeper[top.first] << endl;
        if (!pageKeeper[temp_I]->GetFirst(temp_R))
        {
            //cout << "Hit 1" << endl;
            if (++pageChecker[temp_I] < pageCounter[temp_I + 1])
            {
                //cout << "Hit 2" << endl;
                pageKeeper[temp_I]->EmptyItOut();
                bigQ->file.GetPage(pageKeeper[temp_I], pageChecker[temp_I]);
                pageKeeper[temp_I]->GetFirst(temp_R);
                pq.push(make_pair(temp_I, temp_R));
            }
        }
        else
        {
            //cout << "In else:" << " top.first = " << top.first << endl;
            //cout << "In else:" << " top.second = " << top.second << endl;
            pq.push(make_pair(temp_I, temp_R));
        }
    }
   // cout << c << " inserted into pipe" << endl;
    for (auto i:pageKeeper)
        delete i;
    bigQ->outputPipe.ShutDown();
    bigQ->CloseFile();
    bigQ->file.Close();
    remove((bigQ->tempFile).c_str());
    //remove((bigQ->tempFile + ".meta").c_str());
    return NULL;
}