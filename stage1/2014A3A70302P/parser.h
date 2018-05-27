/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 1
*/


#ifndef _PARSER_
#define _PARSER_
#include <stdbool.h>
// Don't Use any of these functions ..
// Just put here for requirements

void printparsetree();
void printfirst();
void printfollow();
void fillParseTable();
void initializeParser(char* grammarfile);
void computefollow() ;
void computefirst() ;
void initializeGrammar(char *grammarfile);
// This one calls everything ....
bool parseInputFile(char *programfile) ;
void printparsetreetofile(char *outfile);
#endif
