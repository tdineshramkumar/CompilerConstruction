/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 2
*/


#ifndef __INTERMEDIATE_CODE__
#define __INTERMEDIATE_CODE__


#include "lexerDef.h"
#include "parserDef.h"
#include "parser.h"
#include "abstractDef.h"
#include "stable.h"

// this function creates all intermediate variables 
// generates all intermediate labels 
// any many more ..
// this fills many attributes and helps to generateNASMCode ..
void addalltemps(ASTPStmt mainstmt);
void generateNASMCode( ASTPStmt mainstmt, FILE *outfile);
#endif