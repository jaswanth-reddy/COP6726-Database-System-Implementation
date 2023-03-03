#ifndef DBFILE_H
#define DBFILE_H

#include <fstream>
#include "BigQ.h"
#include "DBFPart2.h"
#include "Defs.h"
#include "Pipe.h"
#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "Util.h"
 
void MergeTwoPipes(Pipe* pipe, DBFPart2* sorted_file, DBFPart2* out, OrderMaker* order);

void MakeFileEmpty(string fpath);

struct FileMetaInfo {
	fType fileType;
	int FileLength;
	OrderMaker order; 
};

struct SortInfo { 
	OrderMaker *SortOrder; 
	int sortLengthInfo;
};

struct BigQInfo {
	BigQ* sorter;
	Pipe* InpPipe;
	Pipe* OutPipe;
};


class HolisticDBFile {
	protected:
		fType DBFileTyp;

		DBFPart2* DBFile;
		DBFPart2* DB_cache;
		string m_FilePth;
		string m_CachePth;

	public:
		virtual ~HolisticDBFile() {}
		virtual int Create (const char *fpath, fType file_type, void *startup) {}
		virtual int Close () {}
		virtual void Load (Schema &myschema, const char *loadpath) {}
		virtual void MoveFirst () {}
		virtual void Add (Record &addme) {}
		virtual int GetNext (Record &fetchme) {}
		virtual int GetNext (Record &fetchme, CNF &cnf, Record &literal) {} 
		int GetNextNoFlush (Record &fetchme);

		fType getFileType();
		
};

class Heap : public virtual HolisticDBFile {
	public:
		Heap(DBFPart2* file, DBFPart2* cache, string fpath, string cache_path);
		~Heap() {}

		int Create (const char *fpath, fType file_type, void *startup);
		int Close ();
		void Load (Schema &myschema, const char *loadpath);
		void MoveFirst ();
		void Add (Record &addme);
		int GetNext (Record &fetchme);
		int GetNext (Record &fetchme, CNF &cnf, Record &literal);
}; 

class Sorted : public virtual HolisticDBFile {
	private:
		OrderMaker m_SortOrd;
		int m_SortLeng;
		fMode m_FMode;
		BigQInfo m_BigQInfoSort;
	
		OrderMaker m_QryOrd;
		bool IsContNext;

		// Initializes the input pipe instance, the output pipe instance, and the BigQ instance.
		void makeSorter() {
    		m_BigQInfoSort.InpPipe = new Pipe(SizeOfBigQBuffer);
			m_BigQInfoSort.OutPipe = new Pipe(SizeOfBigQBuffer);
    		m_BigQInfoSort.sorter = new BigQ(*(m_BigQInfoSort.InpPipe), *(m_BigQInfoSort.OutPipe), m_SortOrd, m_SortLeng);
  		}

		// Release the memory by deleting the instances of the input pipe, the output pipe, and the BigQ.
  		void DelSorter() {
    		delete m_BigQInfoSort.InpPipe; delete m_BigQInfoSort.OutPipe; delete m_BigQInfoSort.sorter;
    		m_BigQInfoSort.sorter = NULL; m_BigQInfoSort.InpPipe = m_BigQInfoSort.OutPipe = NULL;
		}
		
		// First shuts down the input pipe and clean up the cache file, then calls the twoWayMerge() to read and merge the records from the output pipe and the sorted file into the cache file in ascending order.
		void BeginReadin() {
			if (m_FMode == writing) {
				m_FMode = reading;
				m_BigQInfoSort.InpPipe->ShutDown();
				DB_cache->Close();
				MakeFileEmpty(m_CachePth);
				DB_cache->Open(m_CachePth.c_str());
				DB_cache->MoveFirst();
				MergeTwoPipes(m_BigQInfoSort.OutPipe, DBFile, DB_cache, &m_SortOrd);
				DelSorter();
			}
		}

		// Sets the mode as writing and calls the createSorter().
		void BeginWritin() {
			if (m_FMode == reading) {
				m_FMode = writing;
				makeSorter();
			}
		}
		
		// Verwrites the content of the sorted file with the content in the cache file.
		void MigrateCacheToFile() {

			if (DB_cache->file->GetLength() <= 1) {
				return;
			}
			DBFile->Close();
			MakeFileEmpty(m_FilePth);
			DBFile->Open(m_FilePth.c_str());	
			DBFile->MoveFirst();
			DB_cache->MoveFirst();
			Record record;
			while (DB_cache->GetNext(record)) {
				DBFile->Add(record);
			}
			DBFile->MoveFirst();
			DB_cache->Close();
			MakeFileEmpty(m_CachePth);
			DB_cache->Open(m_CachePth.c_str());
			DB_cache->MoveFirst();
		}
		
	public:
		Sorted() {}
		Sorted(OrderMaker myOrder, int runLength, DBFPart2* file, DBFPart2* cache,  string fpath, string cache_path);
		~Sorted() {}

		int Create (const char *fpath, fType file_type, void *startup);
		int Close ();
		void Load (Schema &myschema, const char *loadpath);
		void MoveFirst ();
		void Add (Record &addme);
		int GetNext (Record &fetchme);
		
		int GetNextByBinarySearch (Record &fetchme, CNF &cnf, Record &literal);
		int GetNext (Record &fetchme, CNF &cnf, Record &literal);
		OrderMaker getOrder();
		
}; 

class DBFile {

public:
	DBFPart2* m_file; 
	DBFPart2* DBCache;
	string m_FileMetaPth;
	FileMetaInfo m_FileMetaInf;

	DBFile (); 
	
	string metaInfoToStr(FileMetaInfo meta_info);
	FileMetaInfo readMetaInfo(string& meta_file);
	void writeMetaInfo(string& meta_file, FileMetaInfo& meta_info);

	int Open (const char *fpath);
	
	int Create (const char *fpath, fType file_type, void *startup);
	
	int Close ();
	
	void Load (Schema &myschema, const char *loadpath);

	void MoveFirst ();
	void Add (Record &addme);
	
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);
	
private: 
	
	string m_sFilePath;
	string m_scachePath;
	HolisticDBFile* m_DBFile;
	
};
#endif
