/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 2
*/


#ifndef __SYMBOL__TABLE__
#define __SYMBOL__TABLE__

#include "lexerDef.h"
#include "parserDef.h"
#include "abstractnstableDef.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
// NOTE: THE MAXIMUM STRING SIZE IS DEFINED HERE
#define STRING_SIZE 24 // to allign well
struct _srecord {
	struct _srecord *next;
	// Now the record ...
	char * id; // this contains the pointer to id ...
	struct _stable *scope; // this contains the scope of variable (which table does it belong to)
	// there are two types of records ...
	// one for variables other for function
	union {
		struct {
			uint size; // this contains the size occupied on layout
			// this is used for matrices only ...
			uint numcols;
			uint numrows; 
			SYMBOL type; // this contains the type of variable ...
			// bool declared; // this contains info whether declared or not ...
			bool initialized;//?
			uint offset; // this contains the offset ...
			// Added Later ...
			bool isinputparam ;
			bool isoutputparam ;
		} variable;
		struct {
			// Later fill this ...
			ASTPStmt function; // to know the type and number of variables ... ??
		}function;
	};

};
struct _shead{
	struct _srecord *first;
	uint count;
};
struct _stable {
	struct _stable *parent; // this points to parent symbol table record ....
	char *funid; // this points to function ...
	// this contains the sizes of various hashtables ...
	uint funsize; 
	uint varsize;
	uint funcount;
	uint varcount;
	uint varoff;
 	struct _shead *vars;
 	struct _shead *funs;
 	uint level; // this contains the nesting level of the table 
};
/// NOTE: Before creating a function check if already name is taken >>>>> ???
/// Also check with the parent ... it can be parent name or its own name ? ( this will be handled except at root which itself is MAIN)
// During function call check if function is accesssible and number of arguments are right and intialized
// Also check for recirsion ....



// This function is used to create a symbol table ...
STable *createSTable(uint varsize, uint funsize, char *funid, STable *parent);
uint hash(char *str, int size);

// These are used to insert the variables (IDs) to symbol table ...
// this function if inserted returns that record else null...
SRecord *insertSTable(STable *st, char *id, SYMBOL type) ;
// this function checks if it exists else returns that record ...
SRecord *existsSRecord(STable *st, char *id);
// this checks only within a STable
SRecord *existsSRecordInSTable(STable *st, char *id);
void UpdateMatrixSTRecord(STable *st, SRecord *record, uint numrows, uint numcols);
void printSTable(STable *st);
void printAllSTable(STable *st);
// Till Now only handled variables now to handle functions... 
SRecord *insertSTableFunction(STable *st, char *funid, ASTPStmt function);
// SRecord *existsSRecordInSTableFunction(STable *st, char *funid);
SRecord *existsSRecordFunction(STable *st, char *funid, bool *recursive);
#endif