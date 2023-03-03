#include <iostream>
#include <fstream>
#include <string>
#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "Defs.h"

using namespace std;

//Reads and merges the records from the output pipe and the sorted file into the cache file in ascending order.
void MergeTwoPipes(Pipe* pipe, DBFPart2* sorted_file, DBFPart2* out, OrderMaker* order) {
    int cnt1 = 0, cnt2 = 0, cntt = 0;
    sorted_file->MoveFirst();
    out->MoveFirst();
    Record rec1;
    Record rec2;
    bool is_rec1_remained = false, is_rec2_remained = false;
    int read_rec1, read_rec2;
    Record recOut;
    ComparisonEngine comp;
    while (true) {
        if ( !is_rec1_remained ) {
            read_rec1 = pipe->Remove(&rec1);
        }
        else {
            read_rec1 = 1;
        }
        if ( !is_rec2_remained ) {
            read_rec2 = sorted_file->GetNext(rec2);
        }
        else {
            read_rec2 = 1;
        }
        if (read_rec1 == 1 && read_rec2 == 1) {
            if (comp.Compare(&rec1, &rec2, order) > 0) {
                recOut.Consume(&rec2);
                is_rec1_remained = true;
                is_rec2_remained = false;
                cnt2++;
            }
            else {
                recOut.Consume(&rec1);
                is_rec1_remained = false;
                is_rec2_remained = true;
                cnt1++;
            }
        }
        else if (read_rec1) {
            recOut.Consume(&rec1);
            is_rec1_remained = false;
            cnt1++;
        }
        else if (read_rec2) {
            recOut.Consume(&rec2);
            is_rec2_remained = false;
            cnt2++;
        }
        else {
            break;
        }
        out->Add(recOut);
        cntt++;
    }
    out->MoveFirst();
}

// Erase the content in the file
void MakeFileEmpty(string fpath) {
    DBFPart2 db;
	db.file->Open(0, (char*)(fpath.c_str()));
	db.Close();
}

fType HolisticDBFile::getFileType() {
    return DBFileTyp;
}


Sorted::Sorted(OrderMaker myOrder, int runLength, DBFPart2* file, DBFPart2* cache, string fpath, string cache_path) {
    DBFileTyp = sorted;
    DBFile = file;
    m_SortOrd = myOrder;
    m_SortLeng = runLength;
    m_FMode = reading;
    DB_cache = cache;
    m_FilePth = fpath;
    m_CachePth = cache_path;
    IsContNext = false;
}

int Sorted::Create(const char *fpath, fType file_type, void *startup) {
    m_FilePth = string(fpath);
    m_CachePth = string(fpath) + ".cache";
    DBFile->Create(fpath, file_type, startup);
    DB_cache->Create((char*)(m_CachePth.c_str()), file_type, startup);
    m_SortOrd = *((*((SortInfo*)startup)).SortOrder);
    m_SortLeng = (*((SortInfo*)startup)).sortLengthInfo;
    return 1;
}

int Sorted::Close () {
    try{  
        IsContNext = false;
        BeginReadin();
        MigrateCacheToFile();
        int state = DBFile->Close();
        DB_cache->Close();
        MakeFileEmpty(m_CachePth);
        m_FilePth = "";
        m_CachePth = "";
        
        return state;
    }
    catch (exception e){
        cerr << "[Error] In function DBFile::Close (): " << e.what() << endl;
        return 0;
    }
}

void Sorted::Load (Schema &f_schema, const char *loadpath) {
    DBFile->Load(f_schema, loadpath);
}

void Sorted::Add (Record &addme) {
    IsContNext = false;
    BeginWritin();
    m_BigQInfoSort.InpPipe->Insert(&addme);
}

void Sorted::MoveFirst () {
    IsContNext = false;
    BeginReadin();
    MigrateCacheToFile();
    if (DBFile->file->GetLength() > 1) {
        
        DBFile->file->GetPage(&DBFile->currentPage, 0);
        DBFile->currentPage.MoveToFirst();
        DBFile->currentDataPageIdx = 0;
    }
    else{
        DBFile->currentDataPageIdx = -1;
        cerr << "[Error] In funciton DBFile::MoveFirst (): No data in file" << endl;
    }
}

int Sorted::GetNext (Record &fetchme) {
    return DBFile->GetNext(fetchme);
}

int Sorted::GetNextByBinarySearch (Record &fetchme, CNF &cnf, Record &literal) { // Notice: file may be empty.
    off_t lpage_idx = DBFile->currentDataPageIdx;
    off_t rpage_idx = DBFile->LastPageIndex();

    if (rpage_idx < 0) {
        return 0;
    }
    return DBFile->GetNext(fetchme, cnf, literal);
    if (!OrderMaker::QueryOrderMaker(m_QryOrd, m_SortOrd, cnf)) {
        return DBFile->GetNext(fetchme, cnf, literal);
    }
    ComparisonEngine comp_engin; 
    while (lpage_idx <= rpage_idx) {
        off_t mid = (lpage_idx + rpage_idx) / 2;
        DBFile->file->GetPage(&(DBFile->currentPage), mid);    
        DBFile->currentDataPageIdx = mid;
        DBFile->currentPage.MoveToFirst();
        cout << "Got m_file" << endl;
        if (!DBFile->GetNext(fetchme)){
            cerr << "[Error] Empty page found in middle of file" << endl;
            exit(1);
        }
        int res = comp_engin.Compare(&fetchme, &literal, &m_QryOrd);
        if (res < 0) {
            lpage_idx = mid + 1;
        }
        else if (res >= 0) {
            rpage_idx = mid - 1;
        }
    }
    lpage_idx = max(lpage_idx, (off_t)0);
    rpage_idx = max(rpage_idx, (off_t)0);
    off_t first_possible_page = min(lpage_idx, rpage_idx);
    DBFile->file->GetPage(&(DBFile->currentPage), first_possible_page);    
    DBFile->currentDataPageIdx = first_possible_page;
    DBFile->currentPage.MoveToFirst();
    return DBFile->GetNext(fetchme, cnf, literal);
}

int Sorted::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
    if (IsContNext) {
        return DBFile->GetNext(fetchme, cnf, literal);
    }
    else{
        IsContNext = true;
        return GetNextByBinarySearch(fetchme, cnf, literal);
    }
}

DBFile::DBFile () {
    m_file = new DBFPart2();
    DBCache = new DBFPart2();
    m_DBFile = NULL;
}

//Reads the meta-information from the structure MetaInfo {fType fileType; int runLength; OrderMaker order;} and outputs them as a string.
string DBFile::metaInfoToStr(FileMetaInfo meta_info) {
    string res = "";
    res += fTypeStr[meta_info.fileType];
    if (meta_info.fileType == sorted) {
        res += "\n" + Util::toString(meta_info.FileLength) + "\n";
        res += meta_info.order.toString();
    }
    return res;
}

//Reads the meta-information from the cache file and returns them in the structur of MetaInfo.
FileMetaInfo DBFile::readMetaInfo(string& meta_file) {
    string file_type_str;
    fType file_type;
    int runLength = -1;
    OrderMaker order;    
    
    ifstream meta;
    meta.open((char*)meta_file.c_str());

    meta >> file_type_str;

    if (file_type_str == fTypeStr[heap]) {
        file_type = heap;
    }
    else if (file_type_str == fTypeStr[sorted]) {
        string numAttsStr, whichAttsStr, whichTypesStr;
        file_type = sorted;
        meta >> runLength;
        meta.ignore();
        getline(meta, numAttsStr);
        getline(meta, whichAttsStr);
        getline(meta, whichTypesStr);
        order = OrderMaker(numAttsStr, whichAttsStr, whichTypesStr);
    }
    meta.close();
    return FileMetaInfo {
        file_type,
        runLength,
        order
    };

}

//Writes the string generated by the function DBFile::metaInfoToStr() into the cache file.
void DBFile::writeMetaInfo(string& meta_file, FileMetaInfo& meta_info) {
    
    ofstream meta;
    meta.open((char*)meta_file.c_str());
    
    string meta_str = metaInfoToStr(meta_info);
    meta << meta_str;
    meta.close();
}

int DBFile::Open(const char *fpath) {
    m_sFilePath = string(fpath);
    m_scachePath = string(fpath) + ".cache";
    m_file->Open(fpath);
    DBCache->Open((char*)(m_scachePath.c_str()));
    m_FileMetaPth = string(fpath) + ".meta";
    m_FileMetaInf = readMetaInfo(m_FileMetaPth);
    if (m_DBFile != NULL) {
        delete m_DBFile;
        m_DBFile = NULL;
    }
    if (m_FileMetaInf.fileType == heap) {
        m_DBFile = new Heap(m_file, DBCache, m_sFilePath, m_scachePath);
    }
    else if (m_FileMetaInf.fileType == sorted) {
        m_DBFile = new Sorted(m_FileMetaInf.order, m_FileMetaInf.FileLength, m_file, DBCache, m_sFilePath, m_scachePath);
    }
    else {
        cerr << "[Error] In DBFile::Open (const char *f_path): Invalid file type '" << fTypeStr[m_FileMetaInf.fileType] << "'" << endl;
        return 0;
    }
    return 1;
}


int DBFile::Create (const char *f_path, fType f_type, void *startup) {
    m_sFilePath = string(f_path);
    m_scachePath = string(f_path) + ".cache";
    if (m_DBFile != NULL) {
        delete m_DBFile;
        m_DBFile = NULL;
    }
    if (f_type == heap) {
        m_DBFile = new Heap(m_file, DBCache, m_sFilePath, m_scachePath);
        m_FileMetaInf = FileMetaInfo {
            heap, 
            -1,
            OrderMaker()
        };
    }
    else if (f_type == sorted) {
        m_DBFile = new Sorted(*(((SortInfo*)startup)->SortOrder), ((SortInfo*)startup)->sortLengthInfo, m_file, DBCache, m_sFilePath, m_scachePath);
    
        m_FileMetaInf = FileMetaInfo {
            sorted, 
            ((SortInfo*)startup)->sortLengthInfo,
            *(((SortInfo*)startup)->SortOrder)
        };
    }
    if (m_DBFile == NULL) {
        cerr << "[Error] In DBFile::Create (const char *f_path, fType f_type, void *startup): No proper file created" << endl;
        return 0;
    }
    m_DBFile->Create(f_path, f_type, startup);
    m_FileMetaPth = string(f_path) + ".meta";
    return 1;
}


int DBFile::Close () {
    try{  
        writeMetaInfo(m_FileMetaPth, m_FileMetaInf);
        int state = m_DBFile->Close();
        delete m_DBFile;
        m_DBFile = NULL;
        return state;
    }
    catch (exception e){
        cerr << "[Error] In function DBFile::Close (): " << e.what() << endl;
        return 0;
    }
}

void DBFile::Load (Schema &f_schema, const char *loadpath) {
    m_DBFile->Load(f_schema, loadpath);
}

void DBFile::Add (Record &rec) {
    m_DBFile->Add(rec);
}

void DBFile::MoveFirst () {
    m_DBFile->MoveFirst();
}

int DBFile::GetNext (Record &fetchme) { 
    return m_DBFile->GetNext(fetchme);
}

int DBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
    return m_DBFile->GetNext(fetchme, cnf, literal);    
}

Heap::Heap(DBFPart2* file, DBFPart2* cache,  string fpath, string cache_path) {
    DBFileTyp = heap;
    DBFile = file;
    DB_cache = cache;
    m_FilePth = fpath;
    m_CachePth = cache_path;
}

int Heap::Create(const char *fpath, fType file_type, void *startup) {
    m_FilePth = string(fpath);
    m_CachePth = string(fpath) + ".cache";
    DBFile->Create(fpath, file_type, startup);
    DB_cache->Create((char*)(m_CachePth.c_str()), file_type, startup);
    return 1;
}

int Heap::Close () {
    try{  
        DBFile->Close();
        DB_cache->Close();
        MakeFileEmpty(m_CachePth);
        m_CachePth = "";
        m_FilePth = "";
    }
    catch (exception e){
        cerr << "[Error] In function DBFile::Close (): " << e.what() << endl;
        return 0;
    }
    return 1;
}

void Heap::Load (Schema &f_schema, const char *loadpath) {
    FILE* data_file = fopen(loadpath, "r");

    Page new_page;

    Record* new_record = new Record();    
    
    while (new_record->SuckNextRecord(&f_schema, data_file)){ 
        if (!new_page.Append(new_record)) 
        {
            DBFile->file->AddPage(&new_page, DBFile->currentDataPageIdx + 1); 
            DBFile->currentDataPageIdx++;
            new_page.EmptyItOut();
        }
    }
    if (!new_page.IsEmpty()) {
        DBFile->file->AddPage(&new_page, DBFile->currentDataPageIdx + 1); 
        DBFile->currentDataPageIdx++;
        new_page.EmptyItOut();
    }

    DBFile->file->GetPage(&DBFile->currentPage, DBFile->currentDataPageIdx);
    this->MoveFirst();
    delete new_record;
    new_record = NULL;

}

void Heap::Add (Record &addme) {
    off_t cur_len = DBFile->file->GetLength();
    off_t last_data_page_idx = cur_len - 2;
    Page last_data_page;
    if (last_data_page_idx >= 0){ 
        DBFile->file->GetPage(&last_data_page, last_data_page_idx);
    }
    if (!last_data_page.Append(&addme)) { 
        Page new_page;
        new_page.Append(&addme);
        DBFile->file->AddPage(&new_page, last_data_page_idx + 1);
    }
    else { 
        if (last_data_page_idx >= 0) {
            DBFile->file->AddPage(&last_data_page, last_data_page_idx);
        }
        else {
            DBFile->file->AddPage(&last_data_page, 0);
        }
    } 
}

int Heap::GetNext (Record &fetchme) {
    off_t cur_len = DBFile->file->GetLength();
    off_t last_data_page_idx = cur_len - 2;
    if (last_data_page_idx < 0){
        cerr << "[Error] In funciton DBFile::GetNext (Record &fetchme): No data in file" << endl;
        return 0;
    }
    int status = DBFile->currentPage.GetNextRecord(fetchme);
    if (status == 2){
        if (DBFile->currentDataPageIdx + 1 > last_data_page_idx){
            cout << "[Info] In function DBFile::GetNext (Record &fetchme): Has arrived at the end of current file." << endl;
            return 0;
        }
        DBFile->currentDataPageIdx++;
        DBFile->file->GetPage(&DBFile->currentPage, DBFile->currentDataPageIdx);
        DBFile->currentPage.GetFirstNoConsume(fetchme);
        return 1;
    }
    return status;
}

int Heap::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
    return DBFile->GetNext(fetchme, cnf, literal);
}

void Heap::MoveFirst () {
    if (DBFile->file->GetLength() > 1) {
        DBFile->file->GetPage(&DBFile->currentPage, 0);
        DBFile->currentPage.MoveToFirst();
        DBFile->currentDataPageIdx = 0;
    }
    else{
        cerr << "[Error] In funciton DBFile::MoveFirst (): No data in file" << endl;
    }
}
