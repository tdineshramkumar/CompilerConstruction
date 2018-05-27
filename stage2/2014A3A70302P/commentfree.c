/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 1
*/

// NOTE: DEFINED SEPARATELY (DUPLICATE OF LEXER) THOUGH NOT REQUIRED
// JUST FOR TESTING ...
// NOW LEXER CAN ITSELF BE USED
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lexerDef.h"

static char buffer1[BUFSIZ+1], buffer2[BUFSIZ+1];
static char *lexemeBegin, *forward;
static unsigned int linenumber = 1; // Stores the current line number (starting with 1)
static FILE *fp ;

#define LOAD(buffer,fp) ({int size=fread(buffer,sizeof(char),BUFSIZ,fp); buffer[size]=EOF;})
#define ENDOFBUFFER(buffer) ((buffer + BUFSIZ ))
static bool newline = true ;
static char nextChar() {
	while ( true ) {
		if ((*forward) == EOF) { // current value of forward pointer
			if ( forward == ENDOFBUFFER(buffer1) ){
				newline = false ;
				LOAD(buffer2,fp); // load the buffer
				// else just move the pointer
				forward = buffer2 ; // make forward point to it
			}
			else if (forward == ENDOFBUFFER(buffer2)) {
				newline = false ;
				LOAD(buffer1,fp);
				forward = buffer1 ;
			}
			else { newline = false ; return EOF;} // No more characters
		}
		else {
			newline = false ;
			char foundchar = *forward; // then it is a character
			if (foundchar == '\n') // <---------------------- MODIFIED
				linenumber++, newline = true ; // if new line character then update line number
			forward++; // Move to next character
			return foundchar ;
		}
	}
}


void commentfreecode(char *program) {
	fp = fopen(program,"r");
	if (fp == NULL) 
		EXITERROR("Unable to open file");
	printf("\t\nCOMMENT FREE PROGRAM\n");
	printf("=====================================================\n");
	LOAD(buffer1,fp); // load the first buffer
	forward = buffer1; // intialize the forward
	lexemeBegin = buffer1; // and lexemebegin pointer
	char lookahead ;
	if (newline == true)
		printf("[%d]\t", linenumber);
	while( (lookahead =  nextChar()) != EOF ) {
		//printf("%c",lookahead);
		switch(lookahead) {
			case '#': // comment
			{	char tmp ;
				do {
					tmp = nextChar() ;
					//printf("{%c}", tmp,tmp);
				} while ( tmp != '\n' && tmp != '\r' && tmp != EOF ) ; // keep looping till end of comment
				if ( tmp == '\n') printf("\n");
				if (newline == true) printf("[%d]\t", linenumber);
				break;
			}
			default:
				if (lookahead != '\r')
					printf("%c",lookahead); 
				if (newline == true)
					printf("[%d]\t", linenumber);
		}
	}
	printf("\n");
	fclose(fp);
}

// int main(int argc,char **argv){
// 	commentfreecode(argv[1]);
// 	return 0;
// }