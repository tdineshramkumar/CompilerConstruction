/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 1
*/


#ifndef _LEXER_
#define _LEXER_
#include "lexerDef.h"

void initializeLexer(char *program);
Token getNextToken();
void printTokenInfo(Token token);

void finalizeLexer();

// This one is a separate file so as to not to interfer with the lexer
// Modelled Similar to lexer
void commentfreecode(char *program) ;
void printtokenlist(char *program);
#endif