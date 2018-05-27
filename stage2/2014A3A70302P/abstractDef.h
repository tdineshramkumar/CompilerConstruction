/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 2
*/




#ifndef __ABSTRACT_SYMTAX_TREE__
#define __ABSTRACT_SYMTAX_TREE__


#include "abstractnstableDef.h"
#include "lexerDef.h"
#include "parserDef.h"
/*

VALID SYMBOL in ASTNode are NT_MATRIX, T_STR, T_NUM, T_RNUM, T_ID, NT_MATRIX_ELEMENT
VALID SYMBOL in ASTNode.operator are T_LT, T_LE, T_GT, T_GE, T_EQ, T_NE (Relational Operator)
		T_PLUS, T_MINUS, T_MUL, T_DIV (Arithmatic Operator) 
		T_AND, T_OR, T_NOT (Logical Operator)
*/
#pragma pack(1)
struct _astnode {
	uint linenumber;
	SYMBOL symbol; // this tells the type...
	union {
		// Define the constants
		struct {
			uint numrows;
			uint numcols;
			uint ** matrix;
			// This is used in code generation to refer to this matrix 
			// char *label;
		} matrix; // first the matrix ...
		struct {
			char *_str;
			int _int;
			float _real;
			// This is used in code generation to refer to this constant ...
			char *label;
		} constant; // then the int, real and the string. 
		// Now variables 
		struct {
			SRecord *record; // this is a record pointer ( POints to hash table record ....)
			char *_id;
		} id; // this is the id ...
		struct {
			SRecord *record; // this points to hash table record ...
			char * _id;
			uint * indices; // this contains the matrix indices..
		} matrix_element; // funally the matrix element
		struct {
			union {
				SRecord *record; // this points to a record generated during intermediate code (used for expression evaluation)
				// This is added for boolean expressions...
				struct {
					char *truelabel;
					char *falselabel;
					// How to use this ...
					// char *newchildlabel; // This is the new label created in child ...
				};
			};
			// SYMBOL type; // this is used later during SEMANTIC ANALYSIS for STRICT TYPE CHECKING ..
			ASTNode *left; // this contains the left operand 
			ASTNode *right; // this contains the right operand
		} operator;
	};
};

// Later include hash table ...

/*
VALID SYMBOL in ASTSTMT are NT_DECL_STMT, NT_COND_STMT, T_READ, T_PRINT (Instead of IO_STMT),
	NT_FUNC_DEF, NT_PROGRAM, NT_FUNC_CALL, NT_EXPR1, NT_EXPR2 ( Instead of NT_ASSIGN_STMT)

VALID decl_stmt.type, args are T_IN, T_REAL, T_STRING, T_MATRIX (data types)
VALID assign_stmt.lhs.type are NT_VARLIST, T_ID
VALID assign_stmt.rhs.type are NT_FUNC_CALL, T_SIZE, NT_ARITH_EXPR, NT_RVALUE
*/
struct _aststmt {
	uint linenumber;
	// this is used to define the type of stmt
	// whether declaration, func_def, fun_call, assign1, assign2, cond ...
	SYMBOL symbol;
	union{
		struct {
			SYMBOL type;
			uint numvars; // this contains the number of vars
			ASTNode *vars; // this contains the array of vars
		} decl_stmt;
		struct {
			uint numifstmts;
			ASTStmt *ifstmts;
			uint numelsestmts;
			ASTStmt *elsestmts;
			ASTNode *condition; // this points to operator...
			// THese are added for branching ...
			char *iflabel;	
			char *elselabel;
			char *nextlabel;

		} cond_stmt;
		struct {
			// Note symbol itself defines the type of IO STMT
			// So only points to variable...
			ASTNode *var; // this points to variable to read or write..
		} io_stmt;
		struct {
			STable * stable; // this points to a hash table ...
			char *_funid;
			uint numinparams;
			SYMBOL *intypes;
			ASTNode *inparams;
			uint numoutparams;
			SYMBOL *outtypes;
			ASTNode *outparams;
			uint numstmts;
			ASTStmt *stmts;
			// These are added for FUNCALL in IR
			char * funlabel;
		} func_def; // this contains the function definition 
		// this is also used for main...
		struct {
			SRecord * record ; // this also points to a hash table record  ...
			char *_funid;
			uint numinargs;
			ASTNode *inargs;
			// Note these are not used... 
			// May  be later ...
			uint numoutargs; 
			ASTNode *outargs;
			// If no out params ...
			// Use these variables to store the values ...
			// They are not part of AST or SEMANTIC CHECK
			// but Intermediate code generator ..
			SRecord ** outrecords; // These contain the record pointers of out variables
			uint numoutrecords;
			// Which Function to CALL?
			char *funlabel; 
		} func_call;
		struct {
			// Type of assign_stmt is decided by symbol ...
		//	SYMBOL type; // this tells the type of assignment (whether function, size, expression with operator or just a variable or constant ...)
			struct {
				SYMBOL symbol;
				union {
					struct{ // if varlist 
						uint numvars;
						ASTNode *varlist;
					};
					struct { // if single variable on LHS
						ASTNode *var;
					};
				};
			} lhs;
			struct {
				SYMBOL symbol;
				union {
					// Use this in case of fun_call 
					ASTStmt *func_call;
					// Use this in case of size stmt...
					// Or in case of initialization or copying a variable ...
					// Use this in case of expression involvin an operator..
					// This is used always execpt function call ...
					ASTNode *var; 
				};
			} rhs;
		} assign_stmt;
	};
};
#pragma pack() // Restore previous packing

#endif