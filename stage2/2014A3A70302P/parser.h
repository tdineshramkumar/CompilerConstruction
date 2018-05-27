/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 1
*/


#ifndef _PARSER_
#define _PARSER_
#include <stdbool.h>
#include "parserDef.h"
#include "abstractnstableDef.h"
// Don't Use any of these functions ..
// Just put here for requirements

// PARSER ..
void printparsetree();
void printfirst();
void printfollow();
void fillParseTable();
void initializeParser(char* grammarfile);
void computefollow() ;
void computefirst() ;
void initializeGrammar(char *grammarfile);
void commentfreecode(char *program);

// This one calls everything ....
bool parseInputFile(char *programfile) ;
// this returns parse tree 
PTPNode getparsetree(); // this function returns the parse tree ....
// this function prints generated program from parse tree ..
void printprogramfromparsetree();
// this deletes parse tree
void deleteparsetree();
// this function print parse tree into a file ..
void printparsetreetofile(char *outfile);
// get number of nodes in parse tree
int countparsetreenode( PTPNode node );

/// ABSTRACT SYNTAX TREE ...
// This function is used to create abstract syntax tree from parse tree ...
void createASTnode( PTPNode node, void * astnode);
// this function prints the abstract syntax tree ...
void printAST(ASTPStmt stmt);
// this is used to get number of ast count ..
int countASTStmt(ASTPStmt stmt);

/// SEMANTIC ANALYSIS AND TYPE CHECKING ...
// This is used to populate hash table ...
// this also does semantic analysis and type checking 
// all in single parse 
bool createandpopulatehashtables(ASTPStmt function, int depth, STable *stable);

// These are later additions ...

#endif
