/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 1
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lexerDef.h"
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<< NOTE: LINE NUMBERS NOT WORING >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Global Variables 
char buffer1[BUFSIZ+1], buffer2[BUFSIZ+1];
char *lexemeBegin, *forward;
unsigned int linenumber = 1; // Stores the current line number (starting with 1)
FILE *fp ;
STATE state = S_START;
unsigned int columnnumber = 0 ; // Stores the current column number (starting from 0)
#define _NO_DEBUG_
#ifndef _NO_DEBUG_
#define DEBUG_MSG(msg, ...) ({ printf("[%s,%d] DEBUG: " msg "\n",__FILE__, __LINE__, ##__VA_ARGS__ ); })
#else
#define DEBUG_MSG(msg,...) 
#endif

#define LOAD(buffer,fp) ({int size=fread(buffer,sizeof(char),BUFSIZ,fp); DEBUG_MSG("LOADED %d of %d into %s\n",size,BUFSIZ,#buffer); buffer[size]=EOF;})
#define ENDOFBUFFER(buffer) ((buffer + BUFSIZ ))
// When ever you call retract .. call update pointers after it after fetching the lexeme...
// or rather go with this flag
bool retracted = false ;
#define RETRACT ({ if (forward == buffer1) forward = buffer2+BUFSIZ-1; else if (forward == buffer2) forward = buffer1+BUFSIZ-1; else forward--; retracted = true ;}) // set the flag to indicate retracting ... which is unset by nextChar
//#define UPDATE_POINTERS  ({ if (forward==(buffer1+BUFSIZ-1)) forward=buffer2; else if (forward==(buffer2+BUFSIZ-1)) forward=buffer1; else forward++; lexemeBegin=forward; })
// if not RETRACT call this 
//	NOTE: it assumes forward has moved ahead .. (by call to nextChar())  
#define RESET_BEGIN_POINTER ({ if (forward == buffer1) lexemeBegin=buffer2+BUFSIZ-1; else if (forward == buffer2) lexemeBegin=buffer1+BUFSIZ-1; else lexemeBegin=forward-1; }) 

#define INBUFFER(ptr,buffer) ( (ptr>=buffer) && (ptr<(buffer+BUFSIZ)) )

#define REPORT_LEX_ERROR1(msg1) ({ printf( "\033[31m ERROR: [LINE(%d):COLUMN(%d)] LEXICAL-ERROR: %s.\n\033[0m",linenumber,columnnumber, msg1); })
#define REPORT_LEX_ERROR2(msg1,lexchar) ({ printf("\033[31m ERROR: [LINE(%d):COLUMN(%d)] LEXICAL-ERROR: %s %c.\n\033[0m",linenumber,columnnumber, msg1,lexchar,lexchar); })
#define REPORT_LEX_ERROR3(msg1,lexchar,msg2) ({ printf("\033[31m ERROR: [LINE(%d):COLUMN(%d)] LEXICAL-ERROR: %s %c %s.\n\033[0m",linenumber,columnnumber,msg1,lexchar,msg2); })
#define REPORT_LEX_WARNING(msg, ...) ({ printf("\033[34m [%s,%d] WARNING: " msg "\033[0m\n",__FILE__, __LINE__, ##__VA_ARGS__ ); })
#define MAX_TOKEN_LEN 20
char nextChar() {
	while ( true ) {
		if ((*forward) == EOF) { // current value of forward pointer
			if ( forward == ENDOFBUFFER(buffer1) ){
				if (! retracted ) // if not retracted 
					LOAD(buffer2,fp); // load the buffer
				// else just move the pointer
				forward = buffer2 ; // make forward point to it
				retracted = false ; // unset the flag
			}
			else if (forward == ENDOFBUFFER(buffer2)) {
				if ( !retracted )
					LOAD(buffer1,fp);
				forward = buffer1 ;
				retracted = false ;
			}
			else return EOF; // No more characters
		}
		else {
			char foundchar = *forward; // then it is a character
			if (!retracted) {// It would have incremented previously
				columnnumber++;
				if (foundchar == '\n') // <---------------------- MODIFIED
					linenumber++, columnnumber= 0; // if new line character then update line number
				// Added column number as well
			}
			forward++; // Move to next character
			retracted = false ; // Just reset the flag
			return foundchar ;
		}
	}
}
#define IN_SYNC_SET(lexchar) ( (lexchar == ';' || lexchar == '\n' || lexchar == EOF || lexchar == '\r') ) // Set of synchronizing elements for panic more recovery
//#define PANIC_MODE ({ while (! IN_SYNC_SET(nextChar()) ) ; state = S_START; }) // Go till after element in synchronizing set <-- NOT WORKING
#define PANIC_MODE ({ char tmp ; do{ tmp = nextChar() ;} while(! IN_SYNC_SET(tmp)) ; RETRACT; state = S_START; }) // Go till after element in synchronizing set <-- MODIFIED WITH RETRACT

// Do we need to RESET BEGIN POINTER above ? check later ..
#define IS_FUNCTION_SYMBOL(lexchar) ( ( lexchar >= 'a' && lexchar <= 'z' ) || ( lexchar >= 'A' && lexchar <= 'Z' ) || ( lexchar >= '0' && lexchar <= '9') )
// This is [begin,forward) <- USING THIS CURRENTLY (Is it Right?)
char *getLexeme() {
	char *lexeme;
	if ( ( INBUFFER(forward,buffer1) && INBUFFER(lexemeBegin,buffer1) ) || 
		 ( INBUFFER(forward,buffer2) && INBUFFER(lexemeBegin,buffer2) ) ) {
			int length = forward - lexemeBegin ; // Note : Not including the last letter of forward
			lexeme = (char *) malloc((length+1)*sizeof(char)); lexeme[length]='\0'; // Note: Extra character is allocated to convert it to string
			for (int i = 0; i < length; i++) lexeme[i] = *(lexemeBegin+i);
	}
	else if ( INBUFFER(forward,buffer1)  &&  INBUFFER(lexemeBegin,buffer2) ) {
		int length = (forward-buffer1)+ ((buffer2+BUFSIZ)-lexemeBegin); // Note: Not including the last letter of forward
		lexeme = (char *) malloc((length+1)*sizeof(char)); lexeme[length]='\0'; // converted to string
		int currpos = 0;
		for (char *i = lexemeBegin; i < (buffer2+BUFSIZ); i++) lexeme[currpos] = *i, currpos ++;
		for (char *i = buffer1; i < forward; i++) lexeme[currpos] = *i, currpos++; // Notice the Less than sign
	}
	else if (INBUFFER(forward,buffer2)  &&  INBUFFER(lexemeBegin,buffer1)) {
		int length = (forward-buffer2)+ ((buffer1+BUFSIZ)-lexemeBegin); 
		lexeme = (char *) malloc((length+1)*sizeof(char));lexeme[length]='\0'; // Converted to string
		int currpos = 0;
		for (char *i = lexemeBegin; i < (buffer1+BUFSIZ); i++) lexeme[currpos] = *i, currpos ++;
		for (char *i = buffer2; i < forward; i++) lexeme[currpos] = *i, currpos++;
	}
	else EXIT("UNKNOWN LOCATIONS\n");
	return lexeme ;
}
// This is [begin-forward] inclusive one (NOT USED)
char *getLexemeinc() {
	char *lexeme;
	if ( ( INBUFFER(forward,buffer1) && INBUFFER(lexemeBegin,buffer1) ) || 
		 ( INBUFFER(forward,buffer2) && INBUFFER(lexemeBegin,buffer2) ) ) {
			int length = forward - lexemeBegin + 1;
			lexeme = (char *) malloc((length+1)*sizeof(char)); lexeme[length]='\0'; // Note: Extra character is allocated to convert it to string
			for (int i = 0; i < length; i++) lexeme[i] = *(lexemeBegin+i);
	}
	else if ( INBUFFER(forward,buffer1)  &&  INBUFFER(lexemeBegin,buffer2) ) {
		int length = (forward-buffer1+1)+ ((buffer2+BUFSIZ)-lexemeBegin);
		lexeme = (char *) malloc((length+1)*sizeof(char)); lexeme[length]='\0'; // converted to string
		int currpos = 0;
		for (char *i = lexemeBegin; i < (buffer2+BUFSIZ); i++) lexeme[currpos] = *i, currpos ++;
		for (char *i = buffer1; i <= forward; i++) lexeme[currpos] = *i, currpos++;
	}
	else if (INBUFFER(forward,buffer2)  &&  INBUFFER(lexemeBegin,buffer1)) {
		int length = (forward-buffer2+1)+ ((buffer1+BUFSIZ)-lexemeBegin); 
		lexeme = (char *) malloc((length+1)*sizeof(char));lexeme[length]='\0'; // Converted to string
		int currpos = 0;
		for (char *i = lexemeBegin; i < (buffer1+BUFSIZ); i++) lexeme[currpos] = *i, currpos ++;
		for (char *i = buffer2; i <= forward; i++) lexeme[currpos] = *i, currpos++;
	}
	else EXIT("UNKNOWN LOCATIONS\n");
	return lexeme ;
}

#define RANDOM_LARGE_PRIME 793669
#define END_HASH 365562
#define INT_HASH 408418
#define REAL_HASH 410725
#define STRING_HASH 564271
#define MATRIX_HASH 773698
#define IF_HASH 37928
#define ELSE_HASH 472101
#define ENDIF_HASH 37771
#define READ_HASH 410717
#define PRINT_HASH 295617
#define FUNCTION_HASH 474970

TOKEN IDorKeyWord(char *lexeme) {
	char * tmp = lexeme ;
	unsigned int hash = 0 ;
	while (*tmp != '\0') {
		hash = ( ( hash*7919 ) + *tmp ) % RANDOM_LARGE_PRIME;
		tmp ++ ;
	}
	switch(hash) {
		case END_HASH: if ( strcmp(lexeme,"end") == 0 ) return T_END ; else return T_ID ;
		case INT_HASH: if ( strcmp(lexeme,"int") == 0 ) return T_INT ; else return T_ID ;
		case REAL_HASH:if ( strcmp(lexeme,"real") == 0 )  return T_REAL ; else return T_ID ;
		case STRING_HASH:if ( strcmp(lexeme,"string") == 0 )  return T_STRING ; else return T_ID ;
		case MATRIX_HASH: if ( strcmp(lexeme,"matrix") == 0 ) return T_MATRIX ; else return T_ID ;
		case IF_HASH: if ( strcmp(lexeme,"if") == 0 ) return T_IF ; else return T_ID ;
		case ELSE_HASH:if ( strcmp(lexeme,"else") == 0 )  return T_ELSE ; else return T_ID ;
		case ENDIF_HASH:if ( strcmp(lexeme,"endif") == 0 )  return T_ENDIF ; else return T_ID ;
		case READ_HASH: if ( strcmp(lexeme,"read") == 0 ) return T_READ; else return T_ID ;
		case PRINT_HASH:if ( strcmp(lexeme,"print") == 0 )  return T_PRINT ; else return T_ID ;
		case FUNCTION_HASH: if ( strcmp(lexeme,"function") == 0 ) return T_FUNCTION; else return T_ID ;
		default: return T_ID ;
	}
}
// Assuming lexeme when required
// And use the lexeme only if requred
// NOTE: THIS FUNCTION HAS TO IDENTIFY KEYWORDS
Token newToken( TOKEN t, char * lexeme ) {
	// Checking for main
	if (t == T_FUNID && strcmp(lexeme,"_main") == 0)  t = T_MAIN;
	if (t == T_ID) t = IDorKeyWord(lexeme);
	Token token ;
	token.token = t ;
	token.linenumber = linenumber ;
	switch(t) {
		case T_NUM:
			token.lexeme._int = atoi(lexeme);
			if ( strlen(lexeme) > 9 ) 
				REPORT_LEX_WARNING("Given integer %s may overflow to (%d).", lexeme, token.lexeme._int);
			free(lexeme); // Free Not Needed Memory ..
			break;
		case T_RNUM:
			token.lexeme._real = (float) atol(lexeme);
			free(lexeme);
			break;
		case T_ID:
			token.lexeme._id = lexeme;
			break; // Not not freed
		case T_FUNID:
			token.lexeme._id = lexeme;
			break;
		case T_STR:
			token.lexeme._str = lexeme;
			// Since we have modified it to accept capital letters
			// We need to convert them back
			for (int i=0; i < strlen(lexeme); i++ ) 
				switch(lexeme[i]){
					case 'A'...'Z':
						lexeme[i] = lexeme[i] + 32 ; // convert to lower 
				}
			break;
	}
	return token ; // Note : Its not a pointer (return by value)
}
// Note: Each time when this function is called 
// State is Start ?
Token getNextToken() {
	// JUST FOR ERROR DEBUGGING
	if (state != S_START ) EXIT("Not In Start State");
	char lookahead ;
	Token token ;
	int tokenlength = 0 ; // Used by string, ID, FUNID
	while( (lookahead =  nextChar()) != EOF ) {
		switch(state) {
			case S_START:{
				// When even start state occured .. Reset the lexemeBegin pointer to point before forward 
				RESET_BEGIN_POINTER ; // bcz of call to nextChar has updated the position of forward to ahead
				switch(lookahead) {
					case '#': // comment
						{	char tmp ;
							do {
								tmp = nextChar() ;
								//printf("CHAR %d %c\n", tmp,tmp);
							} while ( tmp != '\n' && tmp != '\r' && tmp != EOF ) ; // keep looping till end of comment
						} 
						break; // No token returned and No state is changed (start state..)
					case '_':
						state = S_FUNID1 ; // Expecting function
						tokenlength ++ ;
						break; 
					case 'a' ... 'z':
					case 'A' ... 'Z':
						state = S_ID1 ; // Expecting ID or KEYWORD (implement later)
						tokenlength ++ ;
						break;
					case '0' ... '9':
						state = S_NUM1 ;
						break;
					case '"':
						state = S_STR1 ;
						tokenlength ++ ;
						break;
					case '[':
						token = newToken(T_SQO,NULL);
						return token;// Not state not changed (in start state)
					case ']':
						token = newToken(T_SQC,NULL);
						return token;
					case '(':
						token = newToken(T_OP,NULL);
						return token;
					case ')':
						token = newToken(T_CL,NULL);
						return token;
					case ';':
						token = newToken(T_SEMICOLON,NULL);
						return token;
					case ',':
						token = newToken(T_COMMA,NULL);
						return token;
					case '+':
						token = newToken(T_PLUS,NULL);
						return token;
					case '-':
						token = newToken(T_MINUS,NULL);
						return token;
					case '*':
						token = newToken(T_MUL,NULL);
						return token;
					case '/':
						token = newToken(T_DIV, NULL);
						return token;
					case '@':
						token = newToken(T_SIZE,NULL);
						return token;
					case '.':
						state = S_LOGOP1 ; // Waiting for logical operator
						break;
					case '<':
						state = S_LT1 ; // Waiting for some other to decide
						break;
					case '>':
						state = S_GT1 ; // Waiting for some other to decide
						break;
					case '=':
						state = S_ASSIGN1 ; //Waiting for some other to decide
						break;
					//case ' ': // Ignore newline, tab and space
					//case '\t':
					//case '\n':
					case 0 ... 32:
						//printf("Space\n");
						break; // Dont do any thing look at next token (ignore it..)
					default:
						REPORT_LEX_ERROR2("Unknown Symbol",lookahead);
				}

				break;
			}
			case S_FUNID1: // If function was expected ...
			{
				switch(lookahead) {
					case 'a'...'z':
					case 'A'...'Z':
						state = S_FUNID2;
						tokenlength ++ ;
						break;
					default :
						REPORT_LEX_ERROR3("Invalid function. Unexpected Symbol",lookahead,". Expecting [a-zA-Z] ");
						PANIC_MODE ; // after panic mode state is set to start symbol ..
						// to look for another symbol after next synchronizing element
				}
				break;
			}
			case S_FUNID2: // If function was expected ...
			{
				switch(lookahead) {
					case 'a'...'z':
					case 'A'...'Z':
					case '0'...'9': // stay in same state ...
						tokenlength ++ ;
						if (tokenlength > MAX_TOKEN_LEN) {
							REPORT_LEX_ERROR3("Invalid function. TOO long. remove ",lookahead," and ahead..");
							RETRACT ;
							token = newToken(T_FUNID, getLexeme());
							char tmp ;
							do {
								tmp = nextChar();
							}while ( IS_FUNCTION_SYMBOL(tmp) );
							RETRACT; // go back to the non-function-symbol
							state = S_START ; // RESET THE STATE
							return token;
						}
						break;
					default :
						// If some other symbol
						RETRACT; // go back a character ..
						state = S_START ; // RESET THE STATE
						token = newToken(T_FUNID, getLexeme());
						return token;
				}
				break;
			}
			case S_ID1: // If identifyier was expected ... 
			{
				switch(lookahead) {
					case 'a'...'z':
					case 'A'...'Z':
						tokenlength++; // Just increment the token length and stay..
						// Check if length is exceeding
						if (tokenlength > MAX_TOKEN_LEN) {
							REPORT_LEX_ERROR3("Invalid IDENTIFIER. TOO long. remove ",lookahead," and ahead..");
							RETRACT ; // Go Back a character
							token = newToken(T_ID, getLexeme()); // create a token
							char tmp ;
							do {
								tmp = nextChar();
							}while ( IS_FUNCTION_SYMBOL(tmp) ); // <---- NOTE: Here we are Checking with function symbol set (change it later)
							RETRACT; // go back to the non-function-symbol
							state = S_START ; // RESET THE STATE
							return token;
						}
						break;
					case '0' ... '9':
						tokenlength++;
						if (tokenlength > MAX_TOKEN_LEN) { // if length exceeded
							REPORT_LEX_ERROR3("Invalid IDENTIFIER. TOO long. remove ",lookahead," and ahead..");
							RETRACT ; // Go Back a character
							token = newToken(T_ID, getLexeme()); // create a token
							char tmp ;
							do {
								tmp = nextChar();
							}while ( IS_FUNCTION_SYMBOL(tmp) ); // <---- NOTE: Here we are Checking with function symbol set (change it later)
							RETRACT; // go back to the non-function-symbol
							state = S_START ; // RESET THE STATE
							return token;
						}
						else {
							// If within valid length
							token = newToken(T_ID,getLexeme()); // create a token
							state = S_START; // reset the state
							return token;
						}
						break;
					default:
						state = S_START ; // reset the state
						RETRACT; // Go back (so that it can be received)
						token = newToken(T_ID,getLexeme());
						return token ;
				}
				break;
			}
			case S_STR1: // If string was expected .. 
			{
				switch(lookahead){
					case ' ':
					case 'a'...'z':
						tokenlength++;
						// if Length exceeded
						if (tokenlength > MAX_TOKEN_LEN) { 
							REPORT_LEX_ERROR3("Invalid String. Too Long. Remove ",lookahead,"and Ahead");
							RETRACT;
							token = newToken(T_STR, getLexeme()); // create a token
							token.lexeme._str[MAX_TOKEN_LEN-1] = '"'; // Is this NEEDED ?
							PANIC_MODE; // Enter Panic Mode
							return token;
						}
						break;
					case '"':
						tokenlength++;
						// if Length exceeded
						if (tokenlength > MAX_TOKEN_LEN) { 
							REPORT_LEX_ERROR1("Invalid String. Too Long. Remove few characters");
							RETRACT;
							token = newToken(T_STR, getLexeme()); // create a token
							token.lexeme._str[MAX_TOKEN_LEN-1] = '"'; // Is this NEEDED ?
							PANIC_MODE; // Enter Panic Mode ?
							return token;
						}
						else {
							token = newToken(T_STR, getLexeme()); // create a token
							state = S_START ; // RESET THE STATE
							return token; // return the token
						}
						
						break;
					case 'A'...'Z':
						REPORT_LEX_ERROR3("Invalid String. Unexpected Capital Latter ",lookahead," changing it to lower.");
						tokenlength++;
						if (tokenlength > MAX_TOKEN_LEN) { 
							REPORT_LEX_ERROR3("Invalid String. Too Long. Remove ",lookahead,"and Ahead");
							RETRACT;
							token = newToken(T_STR, getLexeme()); // create a token
							token.lexeme._str[MAX_TOKEN_LEN-1] = '"'; // Is this NEEDED ?
							PANIC_MODE; // Enter Panic Mode
							return token;
						}
						break;
					default:
						REPORT_LEX_ERROR2("Invalid String. Unexpected ",lookahead);
						PANIC_MODE; // just enter panic mode
				}
				break;
			}
			case S_NUM1: // If NUM or RNUM expected
			{
				switch(lookahead){
					case '0'...'9':
						break; // Just continue
					case '.': // Then it is a RNUM
						state = S_RNUM1;
						break;
					default: // Then return a integer
						RETRACT ; // Retract
						state = S_START; // Reset the state
						token = newToken(T_NUM,getLexeme());
						return token;
				}
				break;
			}
			case S_RNUM1:{ // Expecting a RNUM (here a NUM after decimal)
				switch(lookahead){
					case '0'...'9':
						state = S_RNUM2 ; // change the state ..
						break;
					default:
						REPORT_LEX_ERROR3("Invalid REAL number. Unexpected ",lookahead," Expecting a number");
						PANIC_MODE ; // after panic mode state is set to start symbol ..
						// to look for another symbol after next synchronizing element
				}
				break;
			}
			case S_RNUM2:{
				switch(lookahead){
					case '0'...'9':
						state = S_START ;
						token = newToken(T_RNUM,getLexeme());
						return token;
					default:
						REPORT_LEX_ERROR3("Invalid READ number. Unexpected ",lookahead," Expecting a number");
						PANIC_MODE ; // after panic mode state is set to start symbol ..
						// to look for another symbol after next synchronizing element
				}
				break;
			}

			case S_LOGOP1: // If logical operator was expected
			{	// Error How Reported ?
				switch(lookahead) {
					case 'a' ... 'z' :
						tokenlength ++;
						if (tokenlength > 3) {
							// Invalid Logical operator
							REPORT_LEX_ERROR1("Invalid Logical operator");
							PANIC_MODE ;
						}
						break;
					case '.':
						// <-------------- CHECK FOR ERRORS HERE LATER
						{
							char *lexeme = getLexeme();
							if ( strcmp (lexeme, "and") == 0 ) {
								// And operator
								free(lexeme);
								token = newToken(T_AND,NULL);
								state = S_START ; // Reset the state..
								return token;
							}
							else if ( strcmp (lexeme, "or") == 0 ) {
								// And operator
								free(lexeme);
								token = newToken(T_OR,NULL);
								state = S_START ; // Reset the state..
								return token;
							}
							else if ( strcmp (lexeme, "not") == 0 ) {
								// And operator
								free(lexeme);
								token = newToken(T_NOT,NULL);
								state = S_START ; // Reset the state..
								return token;
							}
							else {
								REPORT_LEX_ERROR3("Unknown Logical Operator",'.',lexeme);
								free(lexeme);
								PANIC_MODE; // Enter Panic mode
							}
						}
						break;
					default: 
						REPORT_LEX_ERROR2("Unknown Logical Operator, Unexpected symbol",lookahead);
						PANIC_MODE ;
				}
				break;
			}
			case S_LT1: // if less than or equal expected
			{
				switch(lookahead) {
					case '=':
						token = newToken(T_LE,NULL);
						state = S_START;
						return token ; // LE Token returned
					default:
						RETRACT ; // Retract 
						state = S_START ;
						token = newToken(T_LT, NULL);
						return token ; // LT Token returned
				}
				break;
			}
			case S_GT1: // if Greater than or equal expected 
			{
				switch(lookahead) {
					case '=':
						token = newToken(T_GE,NULL);
						state = S_START;
						return token ; // LE Token returned
					default:
						RETRACT ; // Retract 
						state = S_START ;
						token = newToken(T_GT, NULL);
						return token ; // LT Token returned
				}
				break;
			}
			case S_ASSIGN1 :
			{
				switch(lookahead){
					case '/':
						state = S_NE1 ; // Move to not equal state
						break;
					case '=':
						state = S_START ;
						token = newToken(T_EQ,NULL);
						return token;
					default:
						RETRACT ;
						state = S_START ;
						token = newToken(T_ASSIGN,NULL);
						return token ;
				}
				break;
			}
			case S_NE1 : {
				switch(lookahead) {
					case '=':
						state = S_START ;
						token = newToken(T_NE,NULL);
						return token ;
					default:
						REPORT_LEX_ERROR3("Invalid Relational Operator, Unexpected",lookahead,"Expecting =");
						PANIC_MODE ;
				}
				break;
			}
			default: {
				EXIT("Unhandled State");
			}
		}
	}
	token.token = T_EOF ;
	token.linenumber = linenumber; // <-- MODIFIED
	return token;
} 

void printTokenInfo(Token token) {
	switch(token.token) {

		case T_ASSIGN: printf("%5d %15s (%5d)\n",token.linenumber,"T_ASSIGN",token.token); break;
		case T_FUNID:printf("%5d %15s (%5d) [%s]\n",token.linenumber,"T_FUNID",token.token,token.lexeme._id); break;
		case T_ID:printf("%5d %15s (%5d) [%s]\n",token.linenumber,"T_ID",token.token,token.lexeme._id); break;
		case T_NUM:printf("%5d %15s (%5d) [%5d]\n",token.linenumber,"T_NUM",token.token,token.lexeme._int); break;
		case T_RNUM:printf("%5d %15s (%5d) [%.2f]\n",token.linenumber,"T_RNUM",token.token,token.lexeme._real); break;
		case T_STR: printf("%5d %15s (%5d) [%s]\n",token.linenumber,"T_STR",token.token,token.lexeme._str); break;
		case T_END: printf("%5d %15s (%5d)\n",token.linenumber,"T_END",token.token); break;
		case T_INT:printf("%5d %15s (%5d)\n",token.linenumber,"T_INT",token.token); break;
		case T_REAL:printf("%5d %15s (%5d)\n",token.linenumber,"T_REAL",token.token); break;
		case T_STRING:printf("%5d %15s (%5d)\n",token.linenumber,"T_STRING",token.token); break;
		case T_MATRIX:printf("%5d %15s (%5d)\n",token.linenumber,"T_MATRIX",token.token); break;
		case T_MAIN:printf("%5d %15s (%5d)\n",token.linenumber,"T_MAIN",token.token); break;
		case T_SQO:printf("%5d %15s (%5d)\n",token.linenumber,"T_SQO",token.token); break;
		case T_SQC:printf("%5d %15s (%5d)\n",token.linenumber,"T_SQC",token.token); break;
		case T_OP:printf("%5d %15s (%5d)\n",token.linenumber,"T_OP",token.token);break;
		case T_CL:printf("%5d %15s (%5d)\n",token.linenumber,"T_CL",token.token);break;
		case T_SEMICOLON:printf("%5d %15s (%5d)\n",token.linenumber,"T_SEMICOLON",token.token);break;
		case T_COMMA:printf("%5d %15s (%5d)\n",token.linenumber,"T_COMMA",token.token);break;
		case T_IF:printf("%5d %15s (%5d)\n",token.linenumber,"T_IF",token.token);break;
		case T_ELSE:printf("%5d %15s (%5d)\n",token.linenumber,"T_ELSE",token.token);break;
		case T_ENDIF:printf("%5d %15s (%5d)\n",token.linenumber,"T_ENDIF",token.token);break;
		case T_READ:printf("%5d %15s (%5d)\n",token.linenumber,"T_READ",token.token);break;
		case T_PRINT:printf("%5d %15s (%5d)\n",token.linenumber,"T_PRINT",token.token);break;
		case T_FUNCTION:printf("%5d %15s (%5d)\n",token.linenumber,"T_FUNCTION",token.token);break;
		case T_PLUS:printf("%5d %15s (%5d)\n",token.linenumber,"T_PLUS",token.token);break;
		case T_MINUS:printf("%5d %15s (%5d)\n",token.linenumber,"T_MINUS",token.token);break;
		case T_MUL:printf("%5d %15s (%5d)\n",token.linenumber,"T_MUL",token.token);break;
		case T_DIV:printf("%5d %15s (%5d)\n",token.linenumber,"T_DIV",token.token);break;
		case T_SIZE:printf("%5d %15s (%5d)\n",token.linenumber,"T_SIZE",token.token);break;
		case T_AND:printf("%5d %15s (%5d)\n",token.linenumber,"T_AND",token.token);break;
		case T_OR:printf("%5d %15s (%5d)\n",token.linenumber,"T_OR",token.token);break;
		case T_NOT:printf("%5d %15s (%5d)\n",token.linenumber,"T_NOT",token.token);break;
		case T_LT:printf("%5d %15s (%5d)\n",token.linenumber,"T_LT",token.token);break;
		case T_LE:printf("%5d %15s (%5d)\n",token.linenumber,"T_LE",token.token);break;
		case T_GT:printf("%5d %15s (%5d)\n",token.linenumber,"T_GT",token.token);break;
		case T_GE:printf("%5d %15s (%5d)\n",token.linenumber,"T_GE",token.token);break;
		case T_EQ:printf("%5d %15s (%5d)\n",token.linenumber,"T_EQ",token.token);break;
		case T_NE: printf("%5d %15s (%5d)\n",token.linenumber,"T_NE",token.token);break;
		case T_EOF:printf("%5d %15s (%5d)\n",token.linenumber,"T_EOF",token.token);break;
		default: printf("%5d %15s\n",token.linenumber,"Unknown");



		// case T_ASSIGN: printf("%d T_ASSIGN(%d)\n",token.linenumber,token.token); break;
		// case T_FUNID:printf("%d T_FUNID(%d) [%s]\n",token.linenumber,token.token,token.lexeme._id); break;
		// case T_ID:printf("%d T_ID(%d) [%s]\n",token.linenumber,token.token,token.lexeme._id); break;
		// case T_NUM:printf("%d T_NUM(%d) [%d]\n",token.linenumber,token.token,token.lexeme._int); break;
		// case T_RNUM:printf("%d T_RNUM(%d) [%.2f]\n",token.linenumber,token.token,token.lexeme._real); break;
		// case T_STR: printf("%d T_STR(%d) [%s]\n",token.linenumber,token.token,token.lexeme._str); break;
		// case T_END: printf("%d T_END(%d)\n",token.linenumber,token.token); break;
		// case T_INT:printf("%d T_INT(%d)\n",token.linenumber,token.token); break;
		// case T_REAL:printf("%d T_REAL(%d)\n",token.linenumber,token.token); break;
		// case T_STRING:printf("%d T_STRING(%d)\n",token.linenumber,token.token); break;
		// case T_MATRIX:printf("%d T_MATRIX(%d)\n",token.linenumber,token.token); break;
		// case T_MAIN:printf("%d T_MAIN(%d)\n",token.linenumber,token.token); break;
		// case T_SQO:printf("%d T_SQO(%d)\n",token.linenumber,token.token); break;
		// case T_SQC:printf("%d T_SQC(%d)\n",token.linenumber,token.token); break;
		// case T_OP:printf("%d T_OP(%d)\n",token.linenumber,token.token);break;
		// case T_CL:printf("%d T_CL(%d)\n",token.linenumber,token.token);break;
		// case T_SEMICOLON:printf("%d T_SEMICOLON(%d)\n",token.linenumber,token.token);break;
		// case T_COMMA:printf("%d T_COMMA(%d)\n",token.linenumber,token.token);break;
		// case T_IF:printf("%d T_IF(%d)\n",token.linenumber,token.token);break;
		// case T_ELSE:printf("%d T_ELSE(%d)\n",token.linenumber,token.token);break;
		// case T_ENDIF:printf("%d T_ENDIF(%d)\n",token.linenumber,token.token);break;
		// case T_READ:printf("%d T_READ(%d)\n",token.linenumber,token.token);break;
		// case T_PRINT:printf("%d T_PRINT(%d)\n",token.linenumber,token.token);break;
		// case T_FUNCTION:printf("%d T_FUNCTION(%d)\n",token.linenumber,token.token);break;
		// case T_PLUS:printf("%d T_PLUS(%d)\n",token.linenumber,token.token);break;
		// case T_MINUS:printf("%d T_MINUS(%d)\n",token.linenumber,token.token);break;
		// case T_MUL:printf("%d T_MUL(%d)\n",token.linenumber,token.token);break;
		// case T_DIV:printf("%d T_DIV(%d)\n",token.linenumber,token.token);break;
		// case T_SIZE:printf("%d T_SIZE(%d)\n",token.linenumber,token.token);break;
		// case T_AND:printf("%d T_AND(%d)\n",token.linenumber,token.token);break;
		// case T_OR:printf("%d T_OR(%d)\n",token.linenumber,token.token);break;
		// case T_NOT:printf("%d T_NOT(%d)\n",token.linenumber,token.token);break;
		// case T_LT:printf("%d T_LT(%d)\n",token.linenumber,token.token);break;
		// case T_LE:printf("%d T_LE(%d)\n",token.linenumber,token.token);break;
		// case T_GT:printf("%d T_GT(%d)\n",token.linenumber,token.token);break;
		// case T_GE:printf("%d T_GE(%d)\n",token.linenumber,token.token);break;
		// case T_EQ:printf("%d T_EQ(%d)\n",token.linenumber,token.token);break;
		// case T_NE: printf("%d T_NE(%d)\n",token.linenumber,token.token);break;
		// case T_EOF:printf("T_EOF(%d)\n",token.token);break;
		// default: printf("%d Unknown TOKEN\n",token.linenumber);

	}
}
// This sets all the global variables of the lexer
void initializeLexer(char *program) {
	fp = fopen(program,"r");
	if (fp == NULL) EXITERROR("Unable to open file");
	LOAD(buffer1,fp); // load the first buffer
	forward = buffer1; // intialize the forward
	lexemeBegin = buffer1; // and lexemebegin pointer
	// Added Later on... ( If lexer called again reinitialize all global variables)
	linenumber = 1; // Stores the current line number (starting with 1) initialize it if repeated calling ...	
	state = S_START;
	columnnumber = 0 ;
	retracted = false ;
}
// This function closes the file
void finalizeLexer()  {
	fclose(fp);
}

// Added Later to print all tokens
// all initializelexer and finalizelexer

void printtokenlist(char *program) {
	initializeLexer(program);
	printf("\n\tTOKEN LIST\n");
	printf("======================================\n");
	printf("%5s %15s  %5s  %s\n","LINENO","TOKEN","ENUM","LEXEME");
	printf("======================================\n");
	Token token ;
	do {
		token = getNextToken() ;
		printTokenInfo(token);
	} while(token.token != T_EOF) ;
	finalizeLexer() ;
}

// int main(int argc, char **argv) {
// 	fp = fopen(argv[1],"r"); // read a file specified in the argument
// 	if (fp == NULL) EXITERROR("Unable to open file");
// 	LOAD(buffer1,fp); // load the first buffer
// 	forward = buffer1; // intialize the forward
// 	lexemeBegin = buffer1; // and lexemebegin pointer
// 	// char c ; 
// 	// while ( (c = nextChar()) != EOF) printf("%c", c);
// 	Token token  ;
// 	do {
// 		token = getNextToken();
// 		printTokenInfo(token);
// 		//printf("linenumber: %d token: %d\n",linenumber, token.token);
// 	}while (token.token != T_EOF) ;
// 	fclose(fp);
// 	return 0;
// }