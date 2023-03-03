#ifndef RECORD_H
#define RECORD_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Defs.h"
#include "ParseTree.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"



// Basic record data structure. Data is actually stored in "bits" field. The layout of bits is as follows:
//	1) First sizeof(int) bytes: length of the record in bytes
//	2) Next sizeof(int) bytes: byte offset to the start of the first att
//	3) Byte offset to the start of the att in position numAtts
//	4) Bits encoding the record's data

class Record {

friend class ComparisonEngine;
friend class Page;

private:
	char *bits;
	char* GetBits ();
	void SetBits (char *bits);
	void CopyBits(char *bits, int b_len);

public:
	Record ();
	~Record();

	// deep copy constructor, added by Yifan Wang
	//Record(const Record& src);

	// Added by Yifan wang
	int length();

	// suck the contents of the record fromMe into this; note that after
	// this call, fromMe will no longer have anything inside of it
	void Consume (Record *fromMe);

	// make a copy of the record fromMe; note that this is far more 
	// expensive (requiring a bit-by-bit copy) than Consume, which is
	// only a pointer operation
	void Copy (Record *copyMe);

	// reads the next record from a pointer to a text file; also requires
	// that the schema be given; returns a 0 if there is no data left or
	// if there is an error and returns a 1 otherwise
	int SuckNextRecord (Schema *mySchema, FILE *textFile);

	// this projects away various attributes... 
	// the array attsToKeep should be sorted, and lists all of the attributes
	// that should still be in the record after Project is called.  numAttsNow
	// tells how many attributes are currently in the record
	void Project (int *attsToKeep, int numAttsToKeep, int numAttsNow);

	// takes two input records and creates a new record by concatenating them;
	// this is useful for a join operation
	void MergeRecords (Record *left, Record *right, int numAttsLeft, 
		int numAttsRight, int *attsToKeep, int numAttsToKeep, int startOfRight);

	// prints the contents of the record; this requires
	// that the schema also be given so that the record can be interpreted
	void Print (Schema *mySchema);

	// only makes *bits NULL, without deleting it
	// This is for implicitly copying a record
	// For example, we declare 'vector<Record> vec' globally, 
	// then in a block: { Record rec; page.GetFirstRecord(rec); vec.push_back(rec);}
	// vec.push_back will implicitly shallow copy rec and push the copy into vec
	// so after push_back(), vec.back().bits == rec.bits as it is just shallow copy
	// Then at the end of the block, rec will call its destructor which will delete its bits pointer
	// This means vec.back().bits will be also deleted. Then there is an error.
	// So we have to use this PointNULL() to make the old record's bits as NULL such that its destructor will not affect vec's element.
	void PointNULL () {bits = NULL;}
};

#endif