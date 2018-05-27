/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 1
*/


#ifndef _LEXER_DEF_
#define _LEXER_DEF_
#include <stdio.h>
#include <stdlib.h>

#define NUM_NON_TERMINALS 34 // This defines the number of non-terminals (used by parser)
#define NUM_TERMINALS 39 // This defines number of terminals + endofsymbol marker (used by parser)


#define EXITERROR(msg) ( { perror(msg),exit(EXIT_FAILURE);  } )
#define EXIT(msg) ( { printf("ERROR:%s\n",msg),exit(EXIT_FAILURE);  } )

typedef enum 
{
	S_START, S_FUNID1, S_FUNID2, 
	S_ID1, S_NUM1,  S_RNUM1, S_RNUM2, 
	S_STR1, S_LOGOP1, S_LT1, S_GT1, S_ASSIGN1, S_NE1
} STATE;
// Assign all tokens negative numbers (They have an equivalent terminal)
// Assign positive number for Non-terminals
typedef enum {
	// Definitions of TOKENs as used by lexer
	T_ASSIGN = -100, T_FUNID, T_ID, T_NUM, T_RNUM,
	T_STR, T_END, T_INT, T_REAL, T_STRING, T_MATRIX, 
	T_MAIN, T_SQO, T_SQC, T_OP, T_CL, T_SEMICOLON, T_COMMA,
	T_IF, T_ELSE, T_ENDIF, T_READ, T_PRINT, T_FUNCTION,
	T_PLUS, T_MINUS, T_MUL, T_DIV, T_SIZE, T_AND, T_OR, T_NOT,
 	T_LT, T_LE, T_GT, T_GE, T_EQ, T_NE, T_EOF, // Note: EOF used for Marking the end of input 
 	// These Are Control Symbols used by parser
 	RULE_DIVIDER, IMPLIES,
 	T_NULL, // this is used by null production
 	// Definitions of TOKENs (Non Terminals) used by Parser
 	NT_PROGRAM = 0, // Start symbol assigned value of zero rest incremented incrementally
 	NT_ARG, NT_ARGS, NT_ARITH_EXPR,  NT_ASSIGN_STMT,  NT_BOOL_EXPR,  NT_BOOL_OP,  
	NT_BOOL_OPERAND,  NT_CONDITION,  NT_COND_STMT,  NT_DECL_STMT,  NT_ELSE_STMT,  
	NT_EXPR1,  NT_EXPR2,  NT_FUNC_CALL,  NT_FUNC_DEF,  NT_IO_STMT,  NT_MATRIX,  
	NT_MATRIX_ELEMENT,  NT_MOREVARS,  NT_MORE_IN_ARITH_EXPR,  NT_MORE_IN_PROD,  
	NT_MORE_IN_ROW,  NT_MORE_PARAMS,  NT_MORE_ROWS,  NT_PARAMS,  NT_PROD_TERM,  
	NT_REL_OP,  NT_ROW,  NT_RVALUE,  NT_STMT,  NT_STMTS,  NT_TYPE,  NT_VARLIST 

} TOKEN ;


typedef unsigned int uint;
typedef struct { 
	TOKEN token ;
	uint linenumber ;
	union {
		int _int;
		char *_str;
		float _real;
		char *_id;
	} lexeme ; 
} Token; 


#endif
