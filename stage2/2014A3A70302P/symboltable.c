/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 2
*/



#include "stable.h"
#include "abstractDef.h"
// Note: sizes are just suggestive .. (sizes are chosen accordingly to hashing)
STable *createSTable(uint varsize, uint funsize, char *funid, STable *parent){
	STable *st = malloc(sizeof(STable));
	st->parent = parent;
	st->funid = funid ;
	// st->varsize = varsize;
	// st->funsize = funsize;
	st->varsize = ( (5*varsize+1) % 33 == 0 )? 5 * varsize + 2: 5*varsize + 1;
	st->funsize = (funsize % 33 == 0 )? funsize + 1: funsize;
	st->varcount = 0;
	st->funcount = 0;
	st->varoff = 0;
	st->vars = malloc(sizeof(SHead)*st->varsize);
	st->funs = malloc(sizeof(SHead)*st->funsize);
	for ( int i =0; i < st->varsize; i++)
		st->vars[i].first = NULL, st->vars[i].count = 0;
	for ( int i =0; i < st->funsize; i++)
		st->funs[i].first = NULL, st->funs[i].count = 0;

	// <--- NOTE: LATER ADDED ...
	// Now set the nesting level ...
	// Use the parent ...
	int level = 1;
	STable *tmp = parent;
	// Keep looking for root to know the nesting level
	while ( tmp != NULL ) {
		level ++;
		tmp = tmp->parent; // go back one level
	}
	st->level = level; // set the nesting level
	return st;
}

uint hash(char *str, int size){
	uint hashval = 0;
	while ( *str != '\0' ) { 
		hashval = ( hashval * 33 + *str ) % size ;  
		str ++ ;
	}
	return hashval;
}

void UpdateMatrixSTRecord(STable *st, SRecord *record, uint numrows, uint numcols){
	// printf("UPDATE MATRIX STRECORD: %s %d %d\n", record->id, numrows, numcols);
	record->variable.numcols = numcols;
	record->variable.numrows = numrows;
	record->variable.size = sizeof(int)*numrows*numcols;
	record->variable.offset = st->varoff;
	st->varoff += record->variable.size;
}
// Use this with declaration stmt...
// Note: Insert only computes offset for primitive types ..
// Offset for matrix (MATRIX) must be computed later on ...
SRecord *insertSTable(STable *st, char *id, SYMBOL type) {
	uint index = hash(id, st->varsize);
	SHead *head = &(st->vars[index]);
	for ( SRecord *record = head->first; record!= NULL; record = record->next) 
		if ( strcmp(record->id, id) == 0 ) // If such a name record already exist return NULL (Don;t create a NEW record..)
			return NULL;
	// If not already existing create a record ...
	SRecord *record = malloc(sizeof(SRecord));
	record->id = id;
	record->scope = st ; // set the scope to current symbol table 
	record->variable.initialized = false; // initially set everything to uninitialized
	// Take care of sizes and offsets later on ...
	record->variable.type = type;
	#define CASE(_case_,_size_) case _case_:\
	 	record->variable.size=_size_;\
	 	record->variable.numrows=0;\
	 	record->variable.numcols =0;\
	 	break;
	switch( type ) {
		CASE(T_STRING, STRING_SIZE*sizeof(char))
		CASE(T_MATRIX, 0) // Note size is zero ... (So later add it...)
		CASE(T_INT, sizeof(int))
		CASE(T_REAL, sizeof(float)) 
		default: record->variable.size =0; record->variable.numrows=0; record->variable.numcols=0;
	}
	record->variable.offset = st->varoff;
	record->variable.isinputparam = false ; // default is false
	record->variable.isoutputparam = false ;
	st->varoff += record->variable.size;
	st->varcount ++; // increase variable count ...
	head->count ++;
	// Just insert at beginning ...
	record->next = head->first;
	head->first = record;
	return record;
}
// This function checks if variable exists in current scope ...
SRecord *existsSRecordInSTable(STable *st, char *id){
	uint index = hash(id, st->varsize);
	SHead *head = &(st->vars[index]);
	for ( SRecord *record = head->first; record!= NULL; record = record->next) 
		if ( strcmp(record->id, id) == 0 ) 
			return record;
	return NULL;
}
// This function checks if variable is visible from any scope ...
SRecord *existsSRecord(STable *st, char *id){
	do {
		SRecord *tmp = existsSRecordInSTable(st,id);
		if ( st->parent == NULL )// if no parent then ..
			return tmp; // return whether found or not ...
		else if ( tmp != NULL ) // if found in current record also return it ..
			return tmp;
		else // else look in parent
			st = st->parent;
	} while ( 1 );
	return NULL;// this is not needed ...
}
void printSTable(STable *st) {
	if ( st == NULL ) {
		printf("CALLED printSTable with NULL ..\n");
		return;
	}
	// printf("\33[36m%s\033[0m LEVEL:%d FUNSIZE:%d VARSIZE:%d FUNCOUNT:%d VARCOUNT:%d VAROFF:%d\n",
	//  st->funid,st->level, st->funsize, st->varsize, st->funcount, st->varcount, st->varoff);
	// // Now print the variables ...
	// for ( int i =0; i < st->varsize; i ++){
	// 	if ( st->vars[i].count > 0 ){
	// 		//printf("INDEX:%d COUNT:%d\n",i, st->vars[i].count);
	// 		for ( SRecord *record = st->vars[i].first; record != NULL; record = record->next) {
	// 			switch( record->variable.type ){
	// 				case T_MATRIX: printf("\t %s [SIZE:%d, ROWS:%d, COLS:%d, MATRIX, OFF:%d]\n", record->id,
	// 			 record->variable.size, record->variable.numrows, record->variable.numcols, record->variable.offset );break;
	// 				case T_STRING: printf("\t %s [SIZE:%d, ROWS:%d, COLS:%d, STRING, OFF:%d]\n", record->id,
	// 			 record->variable.size, record->variable.numrows, record->variable.numcols, record->variable.offset );break;
	// 				case T_INT: printf("\t %s [SIZE:%d, ROWS:%d, COLS:%d, INT, OFF:%d]\n", record->id,
	// 			 record->variable.size, record->variable.numrows, record->variable.numcols, record->variable.offset );break;
	// 				case T_REAL: printf("\t %s [SIZE:%d, ROWS:%d, COLS:%d, REAL, OFF:%d]\n", record->id,
	// 			 record->variable.size, record->variable.numrows, record->variable.numcols, record->variable.offset );break;
	// 				default:
	// 					printf("--------INVALID TYPE-----\n");
	// 			}
	// 		}
	// 	}
	// }
	printf("\33[36m%s\033[0m LEVEL:%d FUNSIZE:%d VARSIZE:%d FUNCOUNT:%d VARCOUNT:%d VAROFF:%d\n",
	 st->funid,st->level, st->funsize, st->varsize, st->funcount, st->varcount, st->varoff);
	// Now print the variables ...
	#define PRINTPARENT(st) if ( st->parent == NULL ) printf(" \033[031mROOT\033[0m"); else printf(" PARENT:\033[33m%s\033[0m", st->parent->funid);
	for ( int i =0; i < st->varsize; i ++){
		if ( st->vars[i].count > 0 ){
			//printf("INDEX:%d COUNT:%d\n",i, st->vars[i].count);
			for ( SRecord *record = st->vars[i].first; record != NULL; record = record->next) {
				switch( record->variable.type ){
					case T_MATRIX: 
					printf("\t \33[36m%s\033[0m MATRIX SIZE:\033[33m%d\033[0m[\033[33m%d\033[0m \033[33m%d\033[0m] OFF:\033[33m%d\033[0m [SCOPE:\033[33m%s\033[0m NESTING:\033[33m%d\033[0m", record->id,
						record->variable.size, record->variable.numrows, record->variable.numcols,	record->variable.offset,
						st->funid, st->level); PRINTPARENT(st); printf("]\n");break;
					case T_STRING: printf("\t \33[36m%s\033[0m STRING SIZE:\033[33m%d\033[0m OFF:\033[33m%d\033[0m [SCOPE:\033[33m%s\033[0m NESTING:\033[33m%d\033[0m", record->id,
						record->variable.size, record->variable.offset,
						st->funid, st->level); PRINTPARENT(st); printf("]\n");break;
					case T_INT: printf("\t \33[36m%s\033[0m INT SIZE:\033[33m%d\033[0m OFF:\033[33m%d\033[0m [SCOPE:\033[33m%s\033[0m NESTING:\033[33m%d\033[0m", record->id,
						record->variable.size, record->variable.offset,
						st->funid, st->level); PRINTPARENT(st); printf("]\n");break;
					case T_REAL: printf("\t \33[36m%s\033[0m REAL SIZE:\033[33m%d\033[0m OFF:\033[33m%d\033[0m [SCOPE:\033[33m%s\033[0m NESTING:\033[33m%d\033[0m", record->id,
						record->variable.size, record->variable.offset,
						st->funid, st->level); PRINTPARENT(st); printf("]\n");break;
					default:
						printf("--------INVALID TYPE-----\n");
				}
			}
		}
	}
	#define TYPESTR(type) if (type == T_REAL) printf("REAL"); else if (type == T_INT) printf("INT"); else if ( type == T_MATRIX ) printf("MATRIX"); else printf("STRING");
	// Now also print function names it has in its arena ....
	for ( int i = 0 ; i < st->funsize; i ++){
		if ( st->funs[i].count > 0 ){
			for ( SRecord *record = st->funs[i].first; record != NULL; record = record->next) {
				printf("\t\033[032m%s ", record->id);
				ASTPStmt function = record->function.function;
				printf("[LINE:%d] %s IN:[", function->linenumber,function->func_def._funid);
				for ( int i = 0; i < function->func_def.numinparams; i ++){
					TYPESTR(function->func_def.intypes[i]);
					printf(":%s ", function->func_def.inparams[i].id._id);
				}
				printf("\033[1D] OUT:[");
				for ( int i = 0; i < function->func_def.numoutparams; i ++){
					TYPESTR(function->func_def.outtypes[i]);
					printf(":%s ", function->func_def.outparams[i].id._id);
				}
				printf("\033[1D]\033[0m\n");
			}
		}
	}
}





void printAllSTable(STable *st){
	printSTable(st);
	// Now also print function names it has in its arena ....
	for ( int i = 0 ; i < st->funsize; i ++){
		if ( st->funs[i].count > 0 ){
			for ( SRecord *record = st->funs[i].first; record != NULL; record = record->next) {
				ASTPStmt function = record->function.function;
				if ( function->func_def.stable!= NULL )
					printSTable(function->func_def.stable);
			}
		}
	}
}






/// These are similar functions for function

// This function is used to insert a function into STable
SRecord *insertSTableFunction(STable *st, char *funid, ASTPStmt function) {
	uint index = hash(funid, st->funsize);
	SHead *head = &(st->funs[index]);
	for ( SRecord *record = head->first; record!= NULL; record = record->next) 
		if ( strcmp(record->id, funid) == 0 ) // If such a name record already exist return NULL (Don;t create a NEW record..)
			return NULL;
	// If not already existing create a record ...
	SRecord *record = malloc(sizeof(SRecord));
	record->id = funid;
	st->funcount ++;
	head->count ++;
	record->function.function = function;
	record->scope = st ; // set the scope to current symbol table 
	// Just insert at beginning ...
	record->next = head->first;
	head->first = record;
	return record;
}
// Note: If NULL is returned 2 reasons function name is not available or it is recursive ...
// First is used while before creating while other is used while calling ...
SRecord *existsSRecordInSTableFunction(STable *st, char *funid, bool *recursive){
	*recursive = false;
	if ( strcmp(st->funid,funid) == 0 ){

		// printf("ERROR: Recursion Not Allowed..\n");
		*recursive = true;
		return NULL;
	}
	uint index = hash(funid, st->funsize);
	SHead *head = &(st->funs[index]);
	for ( SRecord *record = head->first; record!= NULL; record = record->next) 
		if ( strcmp(record->id, funid) == 0 )
			return record;
	return NULL;
}
SRecord *existsSRecordFunction(STable *st, char *funid, bool *recursive){
	do {
		SRecord *tmp = existsSRecordInSTableFunction(st,funid, recursive);
		// This is just to prevent recursion ..
		if ( (*recursive) )
			return tmp; // <-------- ADDED LATER ,,,
		if ( st->parent == NULL )// if no parent then ..
			return tmp; // return whether found or not ...
		else if ( tmp != NULL ) // if found in current record also return it ..
			return tmp;
		else // else look in parent
			st = st->parent;
	} while ( 1 );
	return NULL;// this is not needed ...
}