
#include <iostream>
#include "Record.h"
#include "DBFile.h"
#include <stdlib.h>
using namespace std;

extern "C" {
	int yyparse(void);   // defined in y.tab.c
}

extern struct AndList *final;

int main () {

	Schema mySchema ("catalog", "test1");

	DBFile dbfile;
	dbfile.Open((char *)"./test1.bin");

	Record temp1;
	
	// read dbfile from start
	dbfile.MoveFirst();
	int counter=0;
	while(dbfile.GetNext(temp1)){
		temp1.Print(&mySchema);
		counter++;
	}
	cout<<"nRead:"<<counter<<" PageNumber:"<<dbfile.curPageNo<<endl;
}


