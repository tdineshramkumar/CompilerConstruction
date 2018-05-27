/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 1
*/


#include "symbolhashs.h"
// Takes as input a string ...
SYMBOL toSymbol(char *c) {
	uint hash = 0;
	// Calculate the hash
	while ( *c != '\0' ) { 
		hash = ( hash * MULTIPLIER + *c ) % SOME_RANDOM_PRIME ;  
		c ++ ;
	}
	//#define MULTIPLIER 83
	//#define SOME_RANDOM_PRIME 997
	switch (hash) {
		// Number of Symbols: 75
		// Number of Hashes: 75
		// Map Size: 75
		case IMPLIES_HASH  :  return IMPLIES ;
		case NT_ARG_HASH  :  return NT_ARG;
		case NT_ARGS_HASH  :  return NT_ARGS;
		case NT_ARITH_EXPR_HASH  :  return NT_ARITH_EXPR;
		case NT_ASSIGN_STMT_HASH  :  return NT_ASSIGN_STMT;
		case NT_BOOL_EXPR_HASH  :  return NT_BOOL_EXPR;
		case NT_BOOL_OP_HASH  :  return NT_BOOL_OP;
		case NT_BOOL_OPERAND_HASH  :  return NT_BOOL_OPERAND;
		case NT_CONDITION_HASH  :  return NT_CONDITION;
		case NT_COND_STMT_HASH  :  return NT_COND_STMT;
		case NT_DECL_STMT_HASH  :  return NT_DECL_STMT;
		case NT_ELSE_STMT_HASH  :  return NT_ELSE_STMT;
		case NT_EXPR1_HASH  :  return NT_EXPR1;
		case NT_EXPR2_HASH  :  return NT_EXPR2;
		case NT_FUNC_CALL_HASH  :  return NT_FUNC_CALL;
		case NT_FUNC_DEF_HASH  :  return NT_FUNC_DEF;
		case NT_IO_STMT_HASH  :  return NT_IO_STMT;
		case NT_MATRIX_HASH  :  return NT_MATRIX;
		case NT_MATRIX_ELEMENT_HASH  :  return NT_MATRIX_ELEMENT;
		case NT_MOREVARS_HASH  :  return NT_MOREVARS;
		case NT_MORE_IN_ARITH_EXPR_HASH  :  return NT_MORE_IN_ARITH_EXPR;
		case NT_MORE_IN_PROD_HASH  :  return NT_MORE_IN_PROD;
		case NT_MORE_IN_ROW_HASH  :  return NT_MORE_IN_ROW;
		case NT_MORE_PARAMS_HASH  :  return NT_MORE_PARAMS;
		case NT_MORE_ROWS_HASH  :  return NT_MORE_ROWS;
		case NT_PARAMS_HASH  :  return NT_PARAMS;
		case NT_PROD_TERM_HASH  :  return NT_PROD_TERM;
		case NT_PROGRAM_HASH  :  return NT_PROGRAM;
		case NT_REL_OP_HASH  :  return NT_REL_OP;
		case NT_ROW_HASH  :  return NT_ROW;
		case NT_RVALUE_HASH  :  return NT_RVALUE;
		case NT_STMT_HASH  :  return NT_STMT;
		case NT_STMTS_HASH  :  return NT_STMTS;
		case NT_TYPE_HASH  :  return NT_TYPE;
		case NT_VARLIST_HASH  :  return NT_VARLIST;
		case T_AND_HASH  :  return T_AND ;
		case T_ASSIGN_HASH  :  return T_ASSIGN ;
		case T_CL_HASH  :  return T_CL ;
		case T_COMMA_HASH  :  return T_COMMA ;
		case T_DIV_HASH  :  return T_DIV ;
		case T_ELSE_HASH  :  return T_ELSE ;
		case T_END_HASH  :  return T_END ;
		case T_ENDIF_HASH  :  return T_ENDIF ;
		case T_EQ_HASH  :  return T_EQ ;
		case T_FUNCTION_HASH  :  return T_FUNCTION ;
		case T_FUNID_HASH  :  return T_FUNID ;
		case T_GE_HASH  :  return T_GE ;
		case T_GT_HASH  :  return T_GT ;
		case T_ID_HASH  :  return T_ID ;
		case T_IF_HASH  :  return T_IF ;
		case T_INT_HASH  :  return T_INT ;
		case T_LE_HASH  :  return T_LE ;
		case T_LT_HASH  :  return T_LT ;
		case T_MAIN_HASH  :  return T_MAIN ;
		case T_MATRIX_HASH  :  return T_MATRIX ;
		case T_MINUS_HASH  :  return T_MINUS ;
		case T_MUL_HASH  :  return T_MUL ;
		case T_NE_HASH  :  return T_NE ;
		case T_NOT_HASH  :  return T_NOT ;
		case T_NULL_HASH  :  return T_NULL ;
		case T_NUM_HASH  :  return T_NUM ;
		case T_OP_HASH  :  return T_OP ;
		case T_OR_HASH  :  return T_OR ;
		case T_PLUS_HASH  :  return T_PLUS ;
		case T_PRINT_HASH  :  return T_PRINT ;
		case T_READ_HASH  :  return T_READ ;
		case T_REAL_HASH  :  return T_REAL ;
		case T_RNUM_HASH  :  return T_RNUM ;
		case T_SEMICOLON_HASH  :  return T_SEMICOLON ;
		case T_SIZE_HASH  :  return T_SIZE ;
		case T_SQC_HASH  :  return T_SQC ;
		case T_SQO_HASH  :  return T_SQO ;
		case T_STR_HASH  :  return T_STR ;
		case T_STRING_HASH  :  return T_STRING ;
		case RULE_DIVIDER_HASH  :  return RULE_DIVIDER ;
		default: EXIT("Unknown Symbol");
	}

} 

// int main(int argc,char **argv) {
// 	FILE *fp = fopen(argv[1],"r");
// 	if ( fp == NULL) EXITERROR("Unable to open file");
// 	char s[100];
// 	while ( fscanf(fp,"%s",s) > 0 ) {
// 		printf("%s[%d] ",s, toSymbol(s));
// 	}
// 	printf("\n");
// 	fclose(fp);
// 	return 0;
// }