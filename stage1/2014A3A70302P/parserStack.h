/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 1
*/



#ifndef _PARSER_STACK_
#define _PARSER_STACK_
#include "lexerDef.h"
#include "parserDef.h"
#include <stdbool.h>
void initializeParserStack(); // Pushes EOF (T_EOF) and YOU NEED to PUSH Start Symbol (NT_PROGRAM)
PTPNode topParserStack(); // Returns top of Stack
bool popParserStack(); // Pops the top of Stack
void pushParserStack(PTPNode treenode ); // Pushs ONE node at a time..
bool emptyParserStack();// Checks if Stack Empty

#endif