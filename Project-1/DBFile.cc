#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "Errors.h"
#include "Defs.h"


DBFile *dbfile;
int expr;


DBFile::DBFile () {
	pointerCurRec = new Record();
	readAllRec = false;

	curPageNo = 0;
	fileMode = WRITE;

	readPgNo = 0;
	readPgRecNo = 0;
}


//Function is used to create a file.
//It can take 3 different type of filetypes as Heap, B+ Tree, Sorted.
int DBFile::Create (const char *f_path, fType f_type, void *startup) {
	

	//cout<<"Calling the Create Function"<<endl;
	// check if path to load the binary data is valid or not.
    if(f_path == NULL){
        return 0;
    }

	//cout<<"File Path is not NULL"<<endl;
	// check if the file is opened already or not.
    expr = (dbfile!=NULL);
    FATALIF(expr, "DBFile is opened, Please check!")

	if(f_type == heap){ 
		// if f_type is heap
		// first parameter '0' will always create a new file if not present.
		//cout<<"Heap File Type is utilised"<<endl;
    	file.Open(0, (char *)f_path); 
    	return 1;
	} else if(f_type == sorted){
		cout<<"ImplementationError: SortedFile Type is passed"<<endl;
	} else if(f_type == tree){
		cout<<"ImplementationError: TreeFile Type is passed"<<endl;
	} else{
		cout<<"UndefinedError: No such implementation Type is passed"<<endl;
	}
	return 0;
}

// Loads the DBFile instance and appends new data using SuckNextRecord from Record.h
void DBFile::Load (Schema &f_schema, const char *loadpath) {

	//cout<<"Calling Load Function"<<endl;
	// change to write mode
	EnableWriteMode(); 


	// open the table file
	FILE *openTableFile = fopen (loadpath, "r"); 

	// holder for new record
	Record newRecordObject; 

	// Iterate over the record object to fetch
	while(newRecordObject.SuckNextRecord(&f_schema, openTableFile) == 1){
		
		// add the new record to the file
		Add(newRecordObject); 

	}
	EnableReadMode(); // Write out the last page to the file
 }


// Function to open the filepath.
int DBFile::Open (const char *file_path) {
	
	//cout<<"Calling the Open Function to open the file with the respective file location"<<endl;
	// first parameter '1' (any non zero value) will open an existing file
	file.Open(1, (char *)file_path); 
	return 1;
}

void DBFile::MoveFirst () {

	//cout<<"Calling MoveFirst Function"<<endl;
	readAllRec = false;
	readPgNo = 0;
	readPgRecNo = 0;

	// change to read mode
	EnableReadMode(); 

	// read from start
	curPageNo = 0; 

	// get the first page
	file.GetPage(&page, curPageNo); 
	// initialise the current pointer to the first record
	page.GetFirst(pointerCurRec); 
}

// Closing the file which is opened
int DBFile::Close () {
	
	//cout<<"Calling the Close function"<<endl;	
	// close the file using the close function.
	file.Close(); 
	return 1;
}

void DBFile::Add (Record &record) {
	
	// switch to write mode
	EnableWriteMode(); 

	// get the status of the page --> return 1: Success, 0: No more space 
	int recAddStatus = page.Append(&record);

	// when page is full
	if(recAddStatus == 0){ 
		
		// We add new page to the file.
		file.AddPage(&page, curPageNo); 

		// update the current page number by one.
		curPageNo+=1;
		
		// Empty the newly added page to remove the same content.
		page.EmptyItOut();
		// add the record to the page.
		page.Append(&record); 
	}
}

int DBFile::GetNext (Record &fetch_me) {
	
	//cout<<"Calling the GetNext function to retreive the next record"<<endl;
	if(readAllRec){ 
		// if the end of the file has been reached, intially it is set to 'False'.
		return 0;
	}
	
	// grab the content from the parameter passed.
	fetch_me.Consume(pointerCurRec);

	// Gets the First record by deleting it, also checking if there is any ? 
	// Returns 1: has record and feteched it, 0: Has no records.
	if(page.GetFirst(pointerCurRec) == 1){
		readPgRecNo++;
		return 1;
	}

	// page has been consumed. Increment page number
	curPageNo+=1; 
	if(curPageNo == file.GetLength()-1){ 
		
		// if there is no next page return 0
		readAllRec = true;
		return 1;
	}

	// get the next page
	file.GetPage(&page, curPageNo); 
	readPgNo+=1;

	// record found
	if(page.GetFirst(pointerCurRec) == 1){ 
		readPgRecNo++;
		return 1;
	}
}

// Function used to fetch the next record, by checking of selection predicate with literal record.
int DBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
	ComparisonEngine comparisonEngine;
	
	//cout<<"Calling the GetNext function to retreive the next record"<<endl;
	// while there are records
	while(GetNext(fetchme)){
		
		// when the fetched records matches the Conjugate Normal Form
		if(comparisonEngine.Compare(&fetchme, &literal, &cnf) == 1){ 
			return 1;
		}
	}
	
	// when no match found with the CNF
	return 0; 
}

// Enabling the WRITE mode in file for IO Operations.
void DBFile::EnableWriteMode(){


	// if file mode is write already
	if(fileMode == WRITE){ 
		return;
	}

	// change fileMode to WRITE
	fileMode = WRITE; 
	//cout<<"Filemode is changed to Write"<<endl;

	/*
	* if file.GetLength()-2<0, then there are no pages except the first blank page so curPageNo=0
	* else curPageNo=file.GetLength()-2 because the first page is blank
	*/
	curPageNo = file.GetLength()-2<0?0:file.GetLength()-2; 
	
	// get the required page with the page number
	file.GetPage(&page, curPageNo);
}

// Enabling the READ mode in file for IO Operations.
void DBFile::EnableReadMode(){
	
	// if file mode is already READ
	if(fileMode==READ){ 
		return;
	}

	// change fileMode to READ
	fileMode = READ; 
	//cout<<"Filemode is changed to Read"<<endl;

	// if the current page has records
	if(page.GetNumRecs() != 0){ 
		
		// add the dirty page to the file
		file.AddPage(&page, curPageNo); 
	}

	// get the page that was being read earlier(before the write started)
	file.GetPage(&page, readPgNo); 
	
	Record temp;
	// move to the record that was being pointed to earlier (before the write started)
	for(int i=0; i<readPgRecNo; i++){ 
		if(GetNext(temp)){
			pointerCurRec = &temp;
		}
	}
}
