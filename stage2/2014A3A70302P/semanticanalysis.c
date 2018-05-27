/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 2
	Fills symbol table ... 
	Does Type checking ...
	Does Semantic Analysis all in single traversal ...
*/



#include "parserDef.h"
#include "symbolhashs.h"
#include "lexer.h"
#include "lexerDef.h"
#include "parserStack.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "abstractDef.h"
#include "stable.h"


#define INDEX_TERMINAL(symbol) ( symbol - T_ASSIGN ) 
// THis computes for a given ifstmt ...
void getvarfuncountifstmt(ASTPStmt stmt, uint *funsizep, uint *varsizep){
	uint funsize =0;
	uint varsize =0;
	// check for if stmts ...
	for ( int i = 0; i < stmt->cond_stmt.numifstmts; i ++)
		if ( stmt->cond_stmt.ifstmts[i].symbol == NT_DECL_STMT )
			varsize += stmt->cond_stmt.ifstmts[i].decl_stmt.numvars ;
		else if ( stmt->cond_stmt.ifstmts[i].symbol == NT_FUNC_DEF )
			funsize ++; // increase the function count ...
		else if ( stmt->cond_stmt.ifstmts[i].symbol == NT_COND_STMT ) // if condition stmt call it again ..
			getvarfuncountifstmt( &(stmt->cond_stmt.ifstmts[i]), funsizep, varsizep );
	
	// then for else stmts ...
	for ( int i = 0; i < stmt->cond_stmt.numelsestmts; i ++)
		if ( stmt->cond_stmt.elsestmts[i].symbol == NT_DECL_STMT )
			varsize += stmt->cond_stmt.elsestmts[i].decl_stmt.numvars ;
		else if ( stmt->cond_stmt.elsestmts[i].symbol == NT_FUNC_DEF )
			funsize ++; // increase the function count ...
		else if ( stmt->cond_stmt.elsestmts[i].symbol == NT_COND_STMT ) // if condition stmt call it again ..
			getvarfuncountifstmt( &(stmt->cond_stmt.elsestmts[i]), funsizep, varsizep );

	*funsizep += funsize;
	*varsizep += varsize; // Note: we are adding the size ...

}
// this function gets the number of variables and function declarations in given function
// used to create a hash table ...
void getvarfuncountfunc(ASTPStmt function, uint *funsizep, uint *varsizep){
	uint funsize =0;
	uint varsize =0;
	// add the count of parameters ...
	varsize += function->func_def.numinparams + function->func_def.numoutparams; // first count the input and output parameters ..
	for ( int i = 0; i < function->func_def.numstmts; i ++ ){
		if ( function->func_def.stmts[i].symbol == NT_DECL_STMT )
			varsize += function->func_def.stmts[i].decl_stmt.numvars ; // increase the count of variables ...
		else if ( function->func_def.stmts[i].symbol == NT_FUNC_DEF )
			funsize ++; // increase the function count ...
		else if ( function->func_def.stmts[i].symbol == NT_COND_STMT )
			getvarfuncountifstmt( &(function->func_def.stmts[i]), &funsize, &varsize );
	}

	*funsizep = funsize;//??
	*varsizep = varsize;//??
}


// Warning this does not check for NULL stmts
// this function scans across declaration stmts and populates the corresponding 
// AST node (function)...
// NOTE: Populate the parents later .....
// Depth is just simple there ... to indicate level in tree...
// stable is just used to initialize the parent ...
bool createandpopulatehashtables(ASTPStmt function, int depth, STable *stable);
// This function is used to check if valid expression and if valid returns the size ...
// This contains the current scope symbol table
// This does not account for initialization currently .. 
bool evaluateExprType( ASTPNode node, SYMBOL *dtype, STable *stable, int *dnumrows, int *dnumcols){
	// printf("EVALUATE EXPR TYPE CALLED ...\n");
	#define REPORT_SEMANTIC_ERROR(linenumber, msg, ...) printf("\033[031m ERROR: [Line(%d)] SEMANTIC-ERROR:" msg "\033[0m\n", linenumber, ##__VA_ARGS__ );
	switch(node->symbol){
		// In case of constants just return them back as type ...
		case T_STR: 
			*dtype = T_STRING;
			*dnumcols = 0;
			*dnumrows = 0;
			return true;
		case T_RNUM:
			*dtype = T_REAL;
			*dnumcols = 0;
			*dnumrows = 0;
			return true;
		case T_NUM:
			*dtype = T_INT;
			*dnumcols = 0;
			*dnumrows = 0;
			return true;
		case NT_MATRIX:
			*dnumrows = node->matrix.numrows;
			*dnumcols = node->matrix.numcols;
			*dtype = T_MATRIX;
			return true;
		case T_ID:
			// printf("[T_ID EXPR %s]", node->id._id);
			node->id.record = existsSRecord(stable,node->id._id);
			if ( node->id.record == NULL ){
				REPORT_SEMANTIC_ERROR(node->linenumber, "Expression Contains Undeclared variable %s", node->id._id);
				return false;
			}
			// printf("Record exist ... TYPE: %d \n", node->id.record->variable.type );
			*dtype = node->id.record->variable.type; // get the type of variable ..
			if ( node->id.record->variable.initialized ){
				// Set the dimension if matrix ...
				*dnumrows = node->id.record->variable.numrows;
				*dnumcols = node->id.record->variable.numcols;
				return true; // return true if initialized ...
			}
			else {
				REPORT_SEMANTIC_ERROR(node->linenumber, "Expression Contains Uninitialized variable %s", node->id._id);
				return false;
			}
		case NT_MATRIX_ELEMENT: {
			// In case of matrix element ...
			SRecord *record = existsSRecord(stable,node->matrix_element._id);
			node->id.record = record ; // Just keep it ?
			if ( record == NULL ){
				REPORT_SEMANTIC_ERROR(node->linenumber, "Expression Contains accessing Undeclared matrix %s", node->matrix_element._id);
				return false;
			}
			else if ( record->variable.type != T_MATRIX ){
				REPORT_SEMANTIC_ERROR(node->linenumber, "%s is NOT a MATRIX", node->matrix_element._id);
				return false;
			}
			else if ( !record->variable.initialized ){
				REPORT_SEMANTIC_ERROR(node->linenumber, "Expression Contains accessing Uninitialized matrix %s", node->matrix_element._id);
				return false;
			}
			else if ( (record->variable.numrows <= node->matrix_element.indices[0]) || (record->variable.numcols <= node->matrix_element.indices[1]) ){
				REPORT_SEMANTIC_ERROR(node->linenumber, "Expression Contains accessing Out of Bound in matrix %s", node->matrix_element._id);
				return false;
			}
			else {
				// Matrix element ... is of type 
				*dtype = T_INT; // Integer ..
				*dnumrows = 0;
				*dnumcols = 0;
				return true;
			}
			break;
		}
		case T_PLUS:{
			ASTPNode left = node->operator.left;
			ASTPNode right = node->operator.right;
			int numrows1, numrows2, numcols1, numcols2;
			SYMBOL type1, type2;
			if ( !evaluateExprType( left, &type1, stable, &numrows1, &numcols1) )
				return false; // the previous one will print the error...
			if ( !evaluateExprType( right, &type2, stable, &numrows2, &numcols2) )
				return false; // the previous one will print the error...
			// If both are correct ..
			if ( type1 != type2 ){
				REPORT_SEMANTIC_ERROR(node->linenumber, "Expression involves operator '%s' with two different types %s and %s.",
					TerminalSymbols[INDEX_TERMINAL(node->symbol)], TerminalSymbols[INDEX_TERMINAL(type1)], TerminalSymbols[INDEX_TERMINAL(type2)]);
				return false;
			}
			if ( type1 == T_MATRIX ){
				// Check if dimensions match ...
				// Then set the dimensions ...
				if ( numrows1 == numrows2 && numcols1 == numcols2 ) {
					*dtype = type1;
					*dnumcols = numcols1;
					*dnumrows = numrows1; // set the dimensions ...
					return true;
				}
				// If not matching ...
				// check if any one is input param ..
				// then match it ...
				else {
					if ( left->symbol == T_ID && left->id.record->variable.isinputparam && numrows1 == 0 && numcols1 == 0 ) {
						// If input variable the update the sizes ...
						// and size is zero update with other variable size ...
						UpdateMatrixSTRecord(stable, left->id.record, numrows2, numcols2);
						// printf("HANDLING INPUTS Updating ... LEFT Maaatrix...\n");
						*dtype = type1;
						*dnumcols = numcols2;
						*dnumrows = numrows2; // set the dimensions ...
						return true;
					}
					else if ( right->symbol == T_ID && right->id.record->variable.isinputparam && numrows2 == 0 && numcols2 == 0   ) {
						UpdateMatrixSTRecord(stable, right->id.record, numrows1, numcols1);
						// printf("HANDLING INPUTS Updating ... RIGHT Maaatrix...\n");
						*dtype = type1;
						*dnumcols = numcols1;
						*dnumrows = numrows1; // set the dimensions ...
						return true;
					}
					else {
						REPORT_SEMANTIC_ERROR(node->linenumber, "Expression involves operator '%s' with two matrices of different sizes [%d,%d] [%d,%d]",
							TerminalSymbols[INDEX_TERMINAL(node->symbol)], numrows1,numcols1,numrows2,numcols2);
						return false;
					}
				}				
				// REPORT_SEMANTIC_ERROR(node->linenumber, "Expression involves operator '%s' with two matrices of different sizes [%d,%d] [%d,%d]",
				// 	TerminalSymbols[INDEX_TERMINAL(node->symbol)], numrows1,numcols1,numrows2,numcols2);
				// return false;
			}
			*dtype = type1; // set the type and return true..
			// Just setting it to first value though not needed ...
			*dnumrows = numrows1;
			*dnumcols = numcols1;
			return true; 
		}
		case T_MINUS:{
			ASTPNode left = node->operator.left;
			ASTPNode right = node->operator.right;
			int numrows1, numrows2, numcols1, numcols2;
			SYMBOL type1, type2;
			if ( !evaluateExprType( left, &type1, stable, &numrows1, &numcols1) )
				return false; // the previous one will print the error...
			if ( !evaluateExprType( right, &type2, stable, &numrows2, &numcols2) )
				return false; // the previous one will print the error...
			// If both are correct ..
			if ( type1 != type2 ){
				REPORT_SEMANTIC_ERROR(node->linenumber, "Expression involves operator '%s' with two different types %s and %s.",
					TerminalSymbols[INDEX_TERMINAL(node->symbol)], TerminalSymbols[INDEX_TERMINAL(type1)], TerminalSymbols[INDEX_TERMINAL(type2)]);
				return false;
			}
			// Now we need to check the types ...
			if ( type1 == T_INT || type1 == T_REAL ){
				*dtype = type1;
				*dnumrows = numrows1;
				*dnumcols = numcols1;
				return true;
			}
			else if ( type1 == T_MATRIX ) {
				// Now check if dimensions are matching ...
				// Note: Above functions return true only if initialized ...
				// <----------------------------------------- MODIFY TO ACCOUNT FOR SIZES OF MATRIX ...
				if ( numrows1 == numrows2 && numcols1 == numcols2 ) {
					*dtype = type1;
					*dnumcols = numcols1;
					*dnumrows = numrows1; // set the dimensions ...
					return true;
				}
				REPORT_SEMANTIC_ERROR(node->linenumber, "Expression involves operator '%s' with two matrices of different sizes",
					TerminalSymbols[INDEX_TERMINAL(node->symbol)]);
				return false;
			}
			return false;
		}
		case T_MUL:{
			ASTPNode left = node->operator.left;
			ASTPNode right = node->operator.right;
			int numrows1, numrows2, numcols1, numcols2;
			SYMBOL type1, type2;
			if ( !evaluateExprType( left, &type1, stable, &numrows1, &numcols1) )
				return false; // the previous one will print the error...
			if ( !evaluateExprType( right, &type2, stable, &numrows2, &numcols2) )
				return false; // the previous one will print the error...
			// If both are correct ..
			if ( type1 != type2 ){
				REPORT_SEMANTIC_ERROR(node->linenumber, "Expression involves operator '%s' with two different types %s and %s.",
					TerminalSymbols[INDEX_TERMINAL(node->symbol)], TerminalSymbols[INDEX_TERMINAL(type1)], TerminalSymbols[INDEX_TERMINAL(type2)]);
				return false;
			}
			// Now we need to check the types ...
			if ( type1 == T_INT || type1 == T_REAL ){
				*dtype = type1;
				*dnumrows = numrows1;
				*dnumcols = numcols1;
				return true;
			}
			return false;
		}
		case T_DIV:{
			ASTPNode left = node->operator.left;
			ASTPNode right = node->operator.right;
			int numrows1, numrows2, numcols1, numcols2;
			SYMBOL type1, type2;
			if ( !evaluateExprType( left, &type1, stable, &numrows1, &numcols1) )
				return false; // the previous one will print the error...
			if ( !evaluateExprType( right, &type2, stable, &numrows2, &numcols2) )
				return false; // the previous one will print the error...
			// If both are correct ..
			if ( type1 != type2 ){
				REPORT_SEMANTIC_ERROR(node->linenumber, "Expression involves operator '%s' with two different types %s and %s.",
					TerminalSymbols[INDEX_TERMINAL(node->symbol)], TerminalSymbols[INDEX_TERMINAL(type1)], TerminalSymbols[INDEX_TERMINAL(type2)]);
				return false;
			}
			// Now we need to check the types ...
			if ( type1 == T_INT || type1 == T_REAL ){
				// <-------------------------------------- CHANGED 
				// To ease the code generation ...
				// We rather have it return the type 
				// INT or REAL
				// *dtype = T_REAL; // division always results in real values ...
				*dtype = type1; // Use type of operands
				*dnumrows = numrows1;
				*dnumcols = numcols1;
				return true;
			}
			REPORT_SEMANTIC_ERROR(node->linenumber, "Expression involves operator '%s' with type %s.",
					TerminalSymbols[INDEX_TERMINAL(node->symbol)], TerminalSymbols[INDEX_TERMINAL(type1)]);
			return false;
		}
		default:
			REPORT_SEMANTIC_ERROR(node->linenumber,"Unknown operator in expression.");
			return false;
	}
	return true;
}
bool checkifvalidboolexpr(ASTPNode node, STable *stable){
	#define REPORT_SEMANTIC_ERROR(linenumber, msg, ...) printf("\033[031m ERROR: [Line(%d)] SEMANTIC-ERROR:" msg "\033[0m\n", linenumber, ##__VA_ARGS__ );
	bool noerror = true;
	switch(node->symbol){
		// If not any of the above check for logical operators ...
		case T_NOT:
			// Not has single operand ...
			return checkifvalidboolexpr(node->operator.left, stable); 
			break;
		case T_AND:
		case T_OR:
			if ( !checkifvalidboolexpr(node->operator.left, stable) ) noerror = false;
			if ( !checkifvalidboolexpr(node->operator.right, stable) ) noerror = false;
			return noerror;
			break;
		// Check for relational operators
		case T_LE:
		case T_LT:
		case T_GE:
		case T_GT:
		case T_NE:
		case T_EQ:
		{
			// Node now are either constants or id or matrix_element ....
			// Take care of types <-------------------
			SYMBOL type1, type2 ;
			// check them ...
			switch( node->operator.left->symbol ){
				case T_ID:
					node->operator.left->id.record = existsSRecord(stable, node->operator.left->id._id);
					if ( node->operator.left->id.record == NULL ){
						REPORT_SEMANTIC_ERROR(node->linenumber, "Boolean Expression Contains Undeclared variable %s", node->operator.left->id._id);
						return false;
					}
					// Else check the type ...
					if ( node->operator.left->id.record->variable.type != T_REAL &&  node->operator.left->id.record->variable.type != T_INT ){
						REPORT_SEMANTIC_ERROR(node->linenumber, "Boolean Expression Contains variable %s of invalid type %s.", node->operator.left->id._id,
							TerminalSymbols[INDEX_TERMINAL(node->operator.left->id.record->variable.type)]);
						return false;
					}
					// If valid type check if initialized...
					if ( ! node->operator.left->id.record->variable.initialized ){
						REPORT_SEMANTIC_ERROR(node->linenumber, "Boolean Expression Contains Uninitialized variable %s", node->operator.left->id._id);
						return false;
					}
					type1 = node->operator.left->id.record->variable.type; // just gather the type ...
					break;
					// return true; // If intialized and valid type 
				case NT_MATRIX_ELEMENT:
					node->operator.left->matrix_element.record = existsSRecord(stable, node->operator.left->matrix_element._id);
					if ( node->operator.left->matrix_element.record == NULL ){
						REPORT_SEMANTIC_ERROR(node->linenumber, "Boolean Expression Contains Undeclared variable %s", node->operator.left->matrix_element._id);
						return false;
					}
					// If record obtained check type ..
					if ( node->operator.left->matrix_element.record->variable.type != T_MATRIX ){
						REPORT_SEMANTIC_ERROR(node->linenumber, "Boolean Expression involves indexing a non matrix variable %s.", node->operator.left->matrix_element._id);
						return false;
					}
					// THen if initialized 
					if ( ! node->operator.left->matrix_element.record->variable.initialized ){
						REPORT_SEMANTIC_ERROR(node->linenumber, "Boolean Expression Contains Uninitialized variable %s", node->operator.left->matrix_element._id);
						return false;
					}
					// Then if valid index ..
					if ( ( node->operator.left->matrix_element.record->variable.numrows <= node->operator.left->matrix_element.indices[0]) || 
						( node->operator.left->matrix_element.record->variable.numcols <= node->operator.left->matrix_element.indices[1]) ){
						REPORT_SEMANTIC_ERROR(node->linenumber, "Expression Contains accessing Out of Bound in matrix %s", node->operator.left->matrix_element._id);
						return false;
					}
					type1 = T_INT; // Matrix contains only integers ...
					break;
				case T_NUM:
					type1 = T_INT;
					break;
				case T_RNUM:
					type1 = T_REAL;
					break;
				default:
					REPORT_SEMANTIC_ERROR(node->linenumber,"Unknown operator in expression.");
					return false;

			}
			// Get right symbol ..
			switch( node->operator.right->symbol ){
				case T_ID:
					node->operator.right->id.record = existsSRecord(stable, node->operator.right->id._id);
					if ( node->operator.right->id.record == NULL ){
						REPORT_SEMANTIC_ERROR(node->linenumber, "Boolean Expression Contains Undeclared variable %s", node->operator.right->id._id);
						return false;
					}
					// Else check the type ...
					if ( node->operator.right->id.record->variable.type != T_REAL &&  node->operator.right->id.record->variable.type != T_INT ){
						REPORT_SEMANTIC_ERROR(node->linenumber, "Boolean Expression Contains variable %s of invalid type %s.", node->operator.right->id._id, 
							TerminalSymbols[INDEX_TERMINAL(node->operator.right->id.record->variable.type)]);
						return false;
					}
					// If valid type check if initialized...
					if ( ! node->operator.right->id.record->variable.initialized ){
						REPORT_SEMANTIC_ERROR(node->linenumber, "Boolean Expression Contains Uninitialized variable %s", node->operator.right->id._id);
						return false;
					}
					type2 = node->operator.right->id.record->variable.type; // just gather the type ...
					break;
					// return true; // If intialized and valid type 
				case NT_MATRIX_ELEMENT:
					node->operator.right->matrix_element.record = existsSRecord(stable, node->operator.right->matrix_element._id);
					if ( node->operator.right->matrix_element.record == NULL ){
						REPORT_SEMANTIC_ERROR(node->linenumber, "Boolean Expression Contains Undeclared variable %s", node->operator.right->matrix_element._id);
						return false;
					}
					// If record obtained check type ..
					if ( node->operator.right->matrix_element.record->variable.type != T_MATRIX ){
						REPORT_SEMANTIC_ERROR(node->linenumber, "Boolean Expression involves indexing a non matrix variable %s.", node->operator.right->matrix_element._id);
						return false;
					}
					// THen if initialized 
					if ( ! node->operator.right->matrix_element.record->variable.initialized ){
						REPORT_SEMANTIC_ERROR(node->linenumber, "Boolean Expression Contains Uninitialized variable %s", node->operator.right->matrix_element._id);
						return false;
					}
					// Then if valid index ..
					if ( ( node->operator.right->matrix_element.record->variable.numrows <= node->operator.right->matrix_element.indices[0]) || 
						( node->operator.right->matrix_element.record->variable.numcols <= node->operator.right->matrix_element.indices[1]) ){
						REPORT_SEMANTIC_ERROR(node->linenumber, "Expression Contains accessing Out of Bound in matrix %s", node->operator.right->matrix_element._id);
						return false;
					}
					type2 = T_INT; // Matrix contains only integers ...
					break;
				case T_NUM:
					type2 = T_INT;
					break;
				case T_RNUM:
					type2 = T_REAL;
					break;
				default:
					REPORT_SEMANTIC_ERROR(node->linenumber,"Unknown operator in expression.");
					return false;
			}
			if ( type1 == type2 )
				return true;
			else {
				REPORT_SEMANTIC_ERROR(node->linenumber, "Boolean Operator %s is applied between two different types %s and %s.", 
					TerminalSymbols[INDEX_TERMINAL(node->symbol)], TerminalSymbols[INDEX_TERMINAL(type1)], TerminalSymbols[INDEX_TERMINAL(type2)] );
				return false;
			}
		}
		default:
			REPORT_SEMANTIC_ERROR(node->linenumber,"Unknown operator in expression.");
			return false;
	}
	return false;
	
}
// This contains a funccall accessible in stable scope ...
// need to validate input and output parameters.. (types and count ...)
// Check if they are declared ..
// THey are defined ....
// Initialize out params if any ...
bool checkifvalidfuncall( ASTPStmt funcall, STable *stable ){
	#define REPORT_SEMANTIC_ERROR(linenumber, msg, ...) printf("\033[031m ERROR: [Line(%d)] SEMANTIC-ERROR:" msg "\033[0m\n", linenumber, ##__VA_ARGS__ );
	// printf("Check if valid function call ...\n");
	ASTPStmt function = funcall->func_call.record->function.function; // get the declaration stmt ..	
	

// Note: INPUTS CAN BE CONSTANTS ...
	if ( function->func_def.numinparams != funcall->func_call.numinargs ){
		// mismatch in number of input args ...
		REPORT_SEMANTIC_ERROR(funcall->linenumber, "Number of Input parameters dont match in function call to %s.", funcall->func_call._funid);
		return false;
	}
	// IF input argument match check the types ...
	// Inputs must be initialized ..
	for ( int i = 0; i < funcall->func_call.numinargs; i ++ ){
		// Note: Input can be constants as well..
		switch( funcall->func_call.inargs[i].symbol ){
			case T_NUM:
				// If integer...
				// correct value ...
				if ( function->func_def.intypes[i] != T_INT ){
					REPORT_SEMANTIC_ERROR(funcall->linenumber, "Expected a NUM type. Got a constant of type '%s'", 
						TerminalSymbols[INDEX_TERMINAL(function->func_def.intypes[i])]);
					return false;
				}
				break;
			case T_RNUM:
				if ( function->func_def.intypes[i] != T_REAL ){
					REPORT_SEMANTIC_ERROR(funcall->linenumber, "Expected a REAL type. Got a constant of type '%s'", 
						TerminalSymbols[INDEX_TERMINAL(function->func_def.intypes[i])]);
					return false;
				}
				break;
			case T_STR:
				if ( function->func_def.intypes[i] != T_STRING ){
					REPORT_SEMANTIC_ERROR(funcall->linenumber, "Expected a STRING type. Got a constant of type '%s'", 
						TerminalSymbols[INDEX_TERMINAL(function->func_def.intypes[i])]);
					return false;
				}
				break;
			case NT_MATRIX:{
				if ( function->func_def.intypes[i] != T_MATRIX ){
					REPORT_SEMANTIC_ERROR(funcall->linenumber, "Expected a MATRIX type. Got a constant of type '%s'", 
						TerminalSymbols[INDEX_TERMINAL(function->func_def.intypes[i])]);
					return false;
				}
				// Now check the dimensions ..
				SRecord *record = function->func_def.inparams[i].id.record;
				if (  record == NULL ){
						// <----------------------------- NOTE: ADDED LATER ...
						// ADD SIMILAR LATER ....
						REPORT_SEMANTIC_ERROR(funcall->linenumber,"Invalid Function Parameter '%s'.",  function->func_def.inparams[i].id._id);
						return false;
					}
				// Get the record of that parameter ....
				if ( record->variable.numrows != funcall->func_call.inargs[i].matrix.numrows || 
					record->variable.numrows != funcall->func_call.inargs[i].matrix.numcols ) {
					REPORT_SEMANTIC_ERROR(funcall->linenumber, "Matrix not of expected dimensions.");
					return false;
				}
				break;
			}
			case T_ID:{
				// If id get the record and type
				funcall->func_call.inargs[i].id.record = existsSRecord(stable, funcall->func_call.inargs[i].id._id);
				if ( funcall->func_call.inargs[i].id.record == NULL ){
					REPORT_SEMANTIC_ERROR(funcall->linenumber, "Function called with Undeclared variable %s", funcall->func_call.inargs[i].id._id);
					return false;
				}
				// If declared check if initialized and of valid type ..
				if ( funcall->func_call.inargs[i].id.record->variable.type != function->func_def.intypes[i] ){
					REPORT_SEMANTIC_ERROR(funcall->linenumber, "Function called with variable %s of unexpected type.", funcall->func_call.inargs[i].id._id);
					return false;
				}
				// If initialized 
				if ( !funcall->func_call.inargs[i].id.record->variable.initialized ){
					REPORT_SEMANTIC_ERROR(funcall->linenumber, "Function called with Uninitialized variable %s", funcall->func_call.inargs[i].id._id);
					return false;
				}
				// Check if matrix type then compare dimensions ..
				if ( funcall->func_call.inargs[i].id.record->variable.type == T_MATRIX ){
					SRecord *record = function->func_def.inparams[i].id.record;
					if (  record == NULL ){
						// <----------------------------- NOTE: ADDED LATER ...
						// ADD SIMILAR LATER ....
						REPORT_SEMANTIC_ERROR(funcall->linenumber,"Invalid Function Parameter '%s'.",  function->func_def.inparams[i].id._id);
						return false;
					}
					// Now check the dimensions ...
					if ( record->variable.numrows != funcall->func_call.inargs[i].id.record->variable.numrows || 
						record->variable.numrows != funcall->func_call.inargs[i].id.record->variable.numcols ) {
						REPORT_SEMANTIC_ERROR(funcall->linenumber, "Matrix '%s' not of expected dimensions.", funcall->func_call.inargs[i].id._id);
						return false;
					}
				}
				// Then done ...
				break;
			} 
			case NT_MATRIX_ELEMENT:{
				ASTPNode me = &(funcall->func_call.inargs[i]);
				me->matrix_element.record = existsSRecord(stable, me->matrix_element._id);
				if ( me->matrix_element.record == NULL ){
					REPORT_SEMANTIC_ERROR(funcall->linenumber, "Function called with Undeclared matrix variable %s", funcall->func_call.inargs[i].matrix_element._id);
					return false;
				}
				// CHeck if type required is INT 
				if ( function->func_def.intypes[i] != T_INT ){
					REPORT_SEMANTIC_ERROR(funcall->linenumber, "Function called with unexpected matrix %s element expecting a variable of type '%s",
					 funcall->func_call.inargs[i].matrix_element._id, TerminalSymbols[INDEX_TERMINAL(function->func_def.intypes[i])]);
					return false;
				} 
				// Check if matrix is initialized ..
				if ( !me->matrix_element.record->variable.initialized ){
					REPORT_SEMANTIC_ERROR(funcall->linenumber, "Function called with Uninitialized matrix variable %s", funcall->func_call.inargs[i].matrix_element._id);
					return false;
				}
				// Check if within bounds ...
				if ( me->matrix_element.record->variable.numrows <= me->matrix_element.indices[0] ||
					me->matrix_element.record->variable.numcols <= me->matrix_element.indices[1] ){
					REPORT_SEMANTIC_ERROR(funcall->linenumber, "Access out of Bound of matrix %s", funcall->func_call.inargs[i].matrix_element._id);
					return false;
				}
				// then done ..
				break;
			}// 

		}
	}




	// First Check Inputs Then Outputs ..
	if ( funcall->func_call.numoutargs != 0 ){
		// check the output args ...
		if ( function->func_def.numoutparams != funcall->func_call.numoutargs ){
			// mismatch in number of output args ...
			REPORT_SEMANTIC_ERROR(funcall->linenumber, "Number of Output parameters dont match in function call to %s. [EXPECTED:%d, OBTAINED:%d]", 
				funcall->func_call._funid, function->func_def.numoutparams, funcall->func_call.numoutargs);
			return false;
		}

		// If arguments match get each argument type and match if they are already declared ...
		for ( int i = 0 ; i < funcall->func_call.numoutargs; i ++ ){
			// Note: THAT ACCORDING TO GRAMMAR LHS OF AN ARRALIST
			// IT MUST ONLY BE IDs
			// THAT IS IT IS T_ID
			// <------------------------------- IF GRAMMER CHANGES CHANGE HERE...
			funcall->func_call.outargs[i].id.record = existsSRecord(stable, funcall->func_call.outargs[i].id._id);
			if ( funcall->func_call.outargs[i].id.record == NULL ) {
				REPORT_SEMANTIC_ERROR(funcall->linenumber,"LHS has Undeclared variable %s", funcall->func_call.outargs[i].id._id);
				return false;
			}
			// If is was declared check the type ...
			if ( funcall->func_call.outargs[i].id.record->variable.type != function->func_def.outtypes[i] ){
				REPORT_SEMANTIC_ERROR(funcall->linenumber,"LHS %s is of unexpected type '%s' expecting variable of type '%s'.", 
					funcall->func_call.outargs[i].id._id,
					TerminalSymbols[INDEX_TERMINAL( funcall->func_call.outargs[i].id.record->variable.type)], 
					TerminalSymbols[INDEX_TERMINAL(function->func_def.outtypes[i])]);
				return false;
			}
			// IF VALID TYPE SET IS AS INITIALIZED ..
			if ( funcall->func_call.outargs[i].id.record->variable.initialized ){
				// If already initialized ..
				// If matrix type check dimensions ...
				if ( funcall->func_call.outargs[i].id.record->variable.type == T_MATRIX ){
					// If matrix ...
					// Check dimension ...
					// Check if variable is it self valid ...
					if (  function->func_def.outparams[i].id.record == NULL ){
						// <----------------------------- NOTE: ADDED LATER ...
						// ADD SIMILAR LATER ....
						REPORT_SEMANTIC_ERROR(funcall->linenumber,"Invalid Function Parameter '%s'.",  function->func_def.outparams[i].id._id);
						return false;
					}
					if ( function->func_def.outparams[i].id.record->variable.numrows != funcall->func_call.outargs[i].id.record->variable.numrows ||
						function->func_def.outparams[i].id.record->variable.numcols != funcall->func_call.outargs[i].id.record->variable.numcols ) {
						REPORT_SEMANTIC_ERROR(funcall->linenumber,"matrix not of required dimension '%s'.", funcall->func_call.outargs[i].id._id);
						return false;
					}
				}
			}
			else {
				// If not initialized ....
				if ( funcall->func_call.outargs[i].id.record->variable.type == T_MATRIX ){
					UpdateMatrixSTRecord(stable, 
						funcall->func_call.outargs[i].id.record, function->func_def.outparams[i].id.record->variable.numrows, 
						function->func_def.outparams[i].id.record->variable.numcols );
						// printf("Updating ... Maaatrix...\n");
				}
				funcall->func_call.outargs[i].id.record->variable.initialized = true; // whether previous intialized or not ..
			}
			
			
		}
		
	}
	//node->operator.right->id.record = existsSRecord(stable, node->operator.right->id._id);
	
	return true;
}
// To avoid repetition of code to handle stmts apart from func_def, decl_stmt
// this also handles conditional stmt (only to check condition (rest done elsewhere)...)
// this function exists ...
// Depth: Not used ...
// stmt contains the current stmt.
// function contains the function declaration stmt
bool checkifdeclared(ASTPStmt stmt, int depth, ASTPStmt function){
	#define REPORT_SEMANTIC_ERROR(linenumber, msg, ...) printf("\033[031m ERROR: [Line(%d)] SEMANTIC-ERROR:" msg "\033[0m\n", linenumber, ##__VA_ARGS__ );
					
	// handle other types of stmt here...
	switch(stmt->symbol){
		case NT_COND_STMT:
			// printf("Called on condition ...\n");
		 	return checkifvalidboolexpr(stmt->cond_stmt.condition, function->func_def.stable);
			// Note: Assuming if else stmts already checked ...
		break;
		case T_READ:
			// SRecord *existsSRecordInSTable(STable *st, char *id)
			stmt->io_stmt.var->id.record = existsSRecord(function->func_def.stable,stmt->io_stmt.var->id._id);
			if ( stmt->io_stmt.var->id.record == NULL ){
				REPORT_SEMANTIC_ERROR(stmt->linenumber, "Read called with Undeclared variable %s", stmt->io_stmt.var->id._id);
				return false;
			}
			else {
				// If declared ... check the type of variable of read ...
				SRecord * record = stmt->io_stmt.var->id.record; // get the record ... to verify type ...
				if ( record->variable.type == T_INT || record->variable.type == T_REAL ) {
					// if valid type 
					// Initialize it ...
					record->variable.initialized = true; // Since the value is read ..
					return true;
				}
				else {
					REPORT_SEMANTIC_ERROR(stmt->linenumber, "Read called with variable %s of type %s", stmt->io_stmt.var->id._id, TerminalStrings[INDEX_TERMINAL(record->variable.type)]);
					return false;
				}
			}
			break;
			//stmt->cond_stmt.ifstmts[i].decl_stmt.vars[j].id.record = insertSTable( function->func_def.stable, 
					//stmt->cond_stmt.ifstmts[i].decl_stmt.vars[j].id._id, stmt->cond_stmt.ifstmts[i].decl_stmt.type);
		case T_PRINT:
			stmt->io_stmt.var->id.record = existsSRecord(function->func_def.stable,stmt->io_stmt.var->id._id);
			if ( stmt->io_stmt.var->id.record == NULL ){
				REPORT_SEMANTIC_ERROR(stmt->linenumber, "Print called with Undeclared variable %s", stmt->io_stmt.var->id._id);
				return false;
			}
			else {
				// If declared ... 
				// <-------------------------------------------------------------- NEED LATER ATTENTION
				// Later implement to check if variable is declared or not ...
				//Matrix size is know are not ...
				if ( stmt->io_stmt.var->id.record->variable.initialized )
					return true;
				else {
					REPORT_SEMANTIC_ERROR(stmt->linenumber,"Print called with Uninitialized variable %s", stmt->io_stmt.var->id._id);
					return false;
				}

			}
			break;
		case NT_FUNC_CALL:
		// Note: this is a function call stmt ...
		// So it specifies no output params... (No checking of outputs ...)
			{
				bool recursive;
				stmt->func_call.record = existsSRecordFunction( function->func_def.stable, stmt->func_call._funid, &recursive);
				if ( stmt->func_call.record == NULL ){
					if ( !recursive){
						REPORT_SEMANTIC_ERROR(stmt->linenumber, "Undeclared Function called %s", stmt->func_call._funid);
					}
					else {
						REPORT_SEMANTIC_ERROR(stmt->linenumber, "Recursive Function call %s", stmt->func_call._funid);
					}
					return false;
				}
				stmt->func_call.numoutargs = 0; // no output arguments specified
			}
			
			// First check if input parameters are valid ... ?
			// If declared function check types ...
			return checkifvalidfuncall( stmt, function->func_def.stable );
		// 	printf(" %s", stmt->func_call._funid);
		// 	// printf(" FUNCCALL:%s", stmt->func_call._funid);
		// 	// printf(" IN:%d", stmt->func_call.numinargs);
		// 	printf(" [");
		// 	for ( int i = 0 ; i <  stmt->func_call.numinargs ; i ++ ){
		// 		printASTNode(&(stmt->func_call.inargs[i]));
		// 	}
		// 	printf(" ]\n");
		// 	// printf("\n");
		 	break;
			case NT_EXPR1:
			case NT_EXPR2:
			switch(stmt->assign_stmt.rhs.symbol){
				case T_SIZE: {
					// Get the record of the variable ...
					stmt->assign_stmt.rhs.var->id.record = existsSRecord(function->func_def.stable, stmt->assign_stmt.rhs.var->id._id);
					if ( stmt->assign_stmt.rhs.var->id.record == NULL ) {
						REPORT_SEMANTIC_ERROR(stmt->linenumber,"Size called with Undeclared variable %s", stmt->assign_stmt.rhs.var->id._id);
						// Don't Do any thing with LSH ??? <----------------------
						return false;
					}
					// if ( !record->variable.initialized ){
					// 	REPORT_SEMANTIC_ERROR(stmt->linenumber,"Size called with Undeclared variable %s", stmt->assign_stmt.rhs.var->id._id);

					// }
					switch(stmt->assign_stmt.lhs.symbol ){
						case T_ID:
							stmt->assign_stmt.lhs.var->id.record = existsSRecord(function->func_def.stable, stmt->assign_stmt.lhs.var->id._id);
							if ( stmt->assign_stmt.lhs.var->id.record == NULL ) {
								REPORT_SEMANTIC_ERROR(stmt->linenumber,"LHS is Undeclared variable %s", stmt->assign_stmt.lhs.var->id._id);
								return false;
							}
							// Once record is obtained ... check the type of lhs ...
							// 
							// rhs must be a string .....
							if ( stmt->assign_stmt.rhs.var->id.record->variable.type != T_STRING ){
								REPORT_SEMANTIC_ERROR(stmt->linenumber,"Invalid operand '%s' of type '%s' with size.", 
									stmt->assign_stmt.rhs.var->id._id, 
									TerminalSymbols[INDEX_TERMINAL(stmt->assign_stmt.rhs.var->id.record->variable.type)]);
								return false;
							}
							// if type of lhs is not int ...
							if ( stmt->assign_stmt.lhs.var->id.record->variable.type != T_INT ){
								REPORT_SEMANTIC_ERROR(stmt->linenumber,"LHS %s is of unexpected type.", stmt->assign_stmt.lhs.var->id._id);
								return false;
							}
							// if type of lhs is int ...
							// initialize it and return true ...
							stmt->assign_stmt.lhs.var->id.record->variable.initialized = true;
							return true;
							break;	
		// 			printASTNode(stmt->assign_stmt.lhs.var);
		// 			break;
						case NT_VARLIST:
							if ( stmt->assign_stmt.rhs.var->id.record->variable.type != T_MATRIX ){
								REPORT_SEMANTIC_ERROR(stmt->linenumber,"RHS %s is not a matrix.", stmt->assign_stmt.rhs.var->id._id);
								return false;
							}
							// if rhs is matrix ..
							if (  stmt->assign_stmt.lhs.numvars != 2 ){
								REPORT_SEMANTIC_ERROR(stmt->linenumber,"LHS has invalid number of variables.");
								return false;
							}
							// if two variables only ..
							for ( int i = 0 ; i < stmt->assign_stmt.lhs.numvars; i ++ ){
								stmt->assign_stmt.lhs.varlist[i].id.record = existsSRecord(function->func_def.stable, stmt->assign_stmt.lhs.varlist[i].id._id);
								if ( stmt->assign_stmt.lhs.varlist[i].id.record == NULL ){
									REPORT_SEMANTIC_ERROR(stmt->linenumber,"LHS has Undeclared variable %s", stmt->assign_stmt.lhs.varlist[i].id._id);
									return false;
								}
								// Check the type of LHS ...
								if ( stmt->assign_stmt.lhs.varlist[i].id.record->variable.type != T_INT ){
									REPORT_SEMANTIC_ERROR(stmt->linenumber,"LHS %s is of unexpected type.", stmt->assign_stmt.lhs.varlist[i].id._id);
									return false;
								}
								stmt->assign_stmt.lhs.varlist[i].id.record->variable.initialized = true;
							}
							return true; // if all variables were sucessfully initialized ..
							break;
					}			
					break;
				}
				case T_ID:{
					// printf("Intiatialize ID...\n");
					// Both are ID ...
					stmt->assign_stmt.rhs.var->id.record = existsSRecord(function->func_def.stable, stmt->assign_stmt.rhs.var->id._id);
					if ( stmt->assign_stmt.rhs.var->id.record == NULL ) {
						REPORT_SEMANTIC_ERROR(stmt->linenumber,"RHS has Undeclared variable %s", stmt->assign_stmt.rhs.var->id._id);
						// Don't Do any thing with LSH ??? <----------------------
						return false;
					}
					if ( ! stmt->assign_stmt.rhs.var->id.record->variable.initialized ){
						REPORT_SEMANTIC_ERROR(stmt->linenumber,"RHS has Uninitialized variable %s", stmt->assign_stmt.rhs.var->id._id);
						return false;
					}
					// If RHS exists and declared ...
					// Check if LHS is declared and is of valid type ...
					// if matrix then if uninitialized initialize it and set size 
					// else check size ...
					stmt->assign_stmt.lhs.var->id.record = existsSRecord(function->func_def.stable, stmt->assign_stmt.lhs.var->id._id);
					if ( stmt->assign_stmt.lhs.var->id.record == NULL ) {
						REPORT_SEMANTIC_ERROR(stmt->linenumber,"LHS has Undeclared variable %s", stmt->assign_stmt.lhs.var->id._id);
						// Don't Do any thing with LSH ??? <----------------------
						return false;
					}
					// Check the types ...
					if ( stmt->assign_stmt.lhs.var->id.record->variable.type != stmt->assign_stmt.rhs.var->id.record->variable.type ){
						REPORT_SEMANTIC_ERROR(stmt->linenumber,"LHS and RHS not of same type..");
						return false;
					}
					if ( ! stmt->assign_stmt.rhs.var->id.record->variable.initialized ){
						// If valid types and LHS is not initialized ... initialize it ...
						if ( stmt->assign_stmt.lhs.var->id.record->variable.type == T_MATRIX ){
							//printf("Updating matrix ... %s \n", stmt->assign_stmt.lhs.var->id._id);
							UpdateMatrixSTRecord(function->func_def.stable, stmt->assign_stmt.lhs.var->id.record, stmt->assign_stmt.rhs.var->id.record->variable.numrows, stmt->assign_stmt.rhs.var->id.record->variable.numcols);
							stmt->assign_stmt.lhs.var->id.record->variable.initialized = true;
							return true; // Intialize it and done
						}
						// Other types just initialize and we are done ...
						stmt->assign_stmt.lhs.var->id.record->variable.initialized = true;
						return true;
					}
					else {
						// If initialized ...
						if ( stmt->assign_stmt.lhs.var->id.record->variable.type == T_MATRIX ){
							// If type is MATRIX check dimensions ...
							if (  stmt->assign_stmt.lhs.var->id.record->variable.numrows != stmt->assign_stmt.rhs.var->id.record->variable.numrows
							|| stmt->assign_stmt.lhs.var->id.record->variable.numcols != stmt->assign_stmt.rhs.var->id.record->variable.numcols ){
								REPORT_SEMANTIC_ERROR(stmt->linenumber,"LHS and RHS are not of same dimensions ...");
								return false;
							}
							return true; // if dimensions match ...
						}
						return true; // Already initialized ..
					}
					break;
				}
				case NT_ARITH_EXPR:{
					// RHS is single variable ...
					//printf("NT_ARITH_EXPR\n");
					SYMBOL type;
					int numrows, numcols;
					// Evaluate the type of RHS
					if ( ! evaluateExprType( stmt->assign_stmt.rhs.var, &type, function->func_def.stable, &numrows, &numcols) ){
						return false;
					}
					// RHS is valid check now LHS and its types ...
					stmt->assign_stmt.lhs.var->id.record = existsSRecord(function->func_def.stable, stmt->assign_stmt.lhs.var->id._id);
					if ( stmt->assign_stmt.lhs.var->id.record == NULL ) {
						REPORT_SEMANTIC_ERROR(stmt->linenumber,"LHS has Undeclared variable %s", stmt->assign_stmt.lhs.var->id._id);
						return false;
					}
					if ( stmt->assign_stmt.lhs.var->id.record->variable.type != type ){
						REPORT_SEMANTIC_ERROR(stmt->linenumber,"Type Mismatch between LHS '%s' and RHS '%s'.. ", 
							TerminalSymbols[INDEX_TERMINAL(stmt->assign_stmt.lhs.var->id.record->variable.type)],
							TerminalSymbols[INDEX_TERMINAL(type)]);
						return false;
					}
					// If types match ...
					if ( stmt->assign_stmt.lhs.var->id.record->variable.type == T_MATRIX ){
						//printf("NUM ROWS: %d NUM COLS:%d\n",numrows, numcols );
						// If matrix do some thing .
						if ( !stmt->assign_stmt.lhs.var->id.record->variable.initialized ){
							// printf("Update Matrix NT_ARITH_EXPR numrows:%d, numcols:%d\n", numrows, numcols);
							UpdateMatrixSTRecord(function->func_def.stable, stmt->assign_stmt.lhs.var->id.record,
								numrows, numcols);
						//	printf("MATRIX: ROWS: %dCOLS: %d\n", stmt->assign_stmt.lhs.var->id.record->variable.numrows, stmt->assign_stmt.lhs.var->id.record->variable.numcols);
							stmt->assign_stmt.lhs.var->id.record->variable.initialized = true;
							return true; // Intialize it and done
						}
						else // IF already initialized ...
						if (  stmt->assign_stmt.lhs.var->id.record->variable.numrows != numrows
							|| stmt->assign_stmt.lhs.var->id.record->variable.numcols != numcols ){
								REPORT_SEMANTIC_ERROR(stmt->linenumber,"LHS and RHS are not of same dimensions ...");
								return false;
						}
						
					}
					stmt->assign_stmt.lhs.var->id.record->variable.initialized = true;
					// If not matrix 
					return true;
					break;
				}
				case NT_FUNC_CALL:{
					// Handle it later ... (After functions are filled ... ?)
					// <----------------------------- NEED ATTENTION...
					// Check if valid types and number of arguments ...
					// Note: BOTH VARLIST AND SINGLE VAR POSSIBLE SO CHECK ...
					ASTPStmt funcall = stmt->assign_stmt.rhs.func_call; // get the function call stmt

					bool recursive;
				
					funcall->func_call.record = existsSRecordFunction( function->func_def.stable, funcall->func_call._funid, &recursive);
					if ( funcall->func_call.record == NULL ){
						if ( recursive){
							REPORT_SEMANTIC_ERROR( funcall->linenumber, "Recursive Function call %s", funcall->func_call._funid);
						}
						else{
							REPORT_SEMANTIC_ERROR( funcall->linenumber, "Undeclared Function called %s", funcall->func_call._funid);
						}
						return false;
					}
					
					if ( stmt->assign_stmt.lhs.symbol == T_ID ){
						// Single output param...
						funcall->func_call.numoutargs = 1;
						funcall->func_call.outargs = stmt->assign_stmt.lhs.var; // get that arg
					}
					else {
						// Multiple output params ....
						funcall->func_call.numoutargs = stmt->assign_stmt.lhs.numvars ;
						funcall->func_call.outargs = stmt->assign_stmt.lhs.varlist; // get that arg
					}
					return checkifvalidfuncall(funcall,function->func_def.stable);
					break;
				}
			}
	}
	return true;
}


// This is an helper function to populate on ifstmts ..
// NOTE: Here cstable is the current symbol table
bool populatehashtables(ASTPStmt stmt, int depth, ASTPStmt function){
	// check for if stmts ...
	// this is used to identify most errors...
	bool noerror = true;
	// <-------------------------- HANDLE FUNCTIONS LATER ....
	for ( int i = 0; i < stmt->cond_stmt.numifstmts; i ++){
	//	printf("HANDLING A %s\n",NonTerminalStrings[stmt->cond_stmt.ifstmts[i].symbol] );
		if ( stmt->cond_stmt.ifstmts[i].symbol == NT_DECL_STMT ){
			for ( int j = 0; j < stmt->cond_stmt.ifstmts[i].decl_stmt.numvars; j ++ ){
				stmt->cond_stmt.ifstmts[i].decl_stmt.vars[j].id.record = insertSTable( function->func_def.stable, 
					stmt->cond_stmt.ifstmts[i].decl_stmt.vars[j].id._id, stmt->cond_stmt.ifstmts[i].decl_stmt.type);
				// if fails return and print error ...
				if ( stmt->cond_stmt.ifstmts[i].decl_stmt.vars[j].id.record == NULL ){
					printf("\033[031m ERROR: [Line(%d)] SEMANTIC-ERROR: Declaration Stmt with same ID [%s] already exists in function [%s]..\033[0m\n", stmt->cond_stmt.ifstmts[i].linenumber, stmt->cond_stmt.ifstmts[i].decl_stmt.vars[j].id._id, function->func_def._funid);
					//return false;
					noerror = false;
				}
			}
		}
		else if ( stmt->cond_stmt.ifstmts[i].symbol == NT_COND_STMT ) {// if condition stmt call it again ..
			if( !populatehashtables( &(stmt->cond_stmt.ifstmts[i]), depth, function) )
				// return false;
				noerror = false;
			// Later added <----------------------
			if ( !checkifdeclared(&(stmt->cond_stmt.ifstmts[i]), depth,function) )
				noerror = false;
		}
		else if ( stmt->cond_stmt.ifstmts[i].symbol == NT_FUNC_DEF ) {
			// Now handle function declaration as well...
			if ( !createandpopulatehashtables( &(stmt->cond_stmt.ifstmts[i]) , depth + 1, function->func_def.stable) )
				// return false;
				noerror = false;
			//printf("CREATING POPULATING INSIZE A IF STMT ...\n");
		}
		else {
			if ( !checkifdeclared(&(stmt->cond_stmt.ifstmts[i]), depth, function) ) // <-------------- LATER ADDED
				noerror = false;
		}
			
	}
		
	for ( int i = 0; i < stmt->cond_stmt.numelsestmts; i ++){
		if ( stmt->cond_stmt.elsestmts[i].symbol == NT_DECL_STMT ){
			for ( int j = 0; j < stmt->cond_stmt.elsestmts[i].decl_stmt.numvars; j ++ ){
				stmt->cond_stmt.elsestmts[i].decl_stmt.vars[j].id.record = insertSTable( function->func_def.stable, 
					stmt->cond_stmt.elsestmts[i].decl_stmt.vars[j].id._id, stmt->cond_stmt.elsestmts[i].decl_stmt.type);
				// if fails return and print error ...
				if ( stmt->cond_stmt.elsestmts[i].decl_stmt.vars[j].id.record == NULL ){
					printf("\033[031m ERROR: [Line(%d)] SEMANTIC-ERROR: Declaration Stmt with same ID [%s] already exists in function [%s]..\033[0m\n", stmt->cond_stmt.elsestmts[i].linenumber, stmt->cond_stmt.elsestmts[i].decl_stmt.vars[j].id._id, function->func_def._funid);
					// return false;
					noerror = false;
				}
			}
		}
		else if ( stmt->cond_stmt.elsestmts[i].symbol == NT_COND_STMT ){ // if condition stmt call it again ..
			if( !populatehashtables( &(stmt->cond_stmt.elsestmts[i]), depth, function) )
				// return false;
				noerror = false;
			// Later added <----------------------
			if ( !checkifdeclared(&(stmt->cond_stmt.elsestmts[i]), depth,function) )
				noerror = false;
		}
		else if ( stmt->cond_stmt.elsestmts[i].symbol == NT_FUNC_DEF ) {
			// Now handle function declaration as well...
			if ( !createandpopulatehashtables( &(stmt->cond_stmt.elsestmts[i]) , depth + 1, function->func_def.stable) )
				// return false;
				noerror = false;
		}
		else {
			if ( !checkifdeclared(&(stmt->cond_stmt.elsestmts[i]), depth, function) ) // <-------------- LATER ADDED
				noerror = false;
		}
	}
	// return true;
	return noerror;
}
// Here stable is PARENT Symbol table
// Function paramaters are set as initialized ...
bool createandpopulatehashtables(ASTPStmt function, int depth, STable *stable){
	// First traverse to count declaration stmts ...
	char *funid = function->func_def._funid ;
	uint funsize =0;
	uint varsize =0;
	getvarfuncountfunc(function, &funsize, &varsize);
	// this is used for reporting errors ...
	bool noerror = true;
	/// <<< ------------------------------------------------------
	// Note: Here are we are setting parent to NULL modify it later
	// First check before creating ....
	// IF function name already exists or not ...
	if ( stable != NULL ){
		bool recursive;
		// check if function name is defined somewhere in parent scope ...
		if ( existsSRecordFunction( stable, funid, &recursive) != NULL || recursive ){
			// If already function name exists in scope ...
			// Then error ...
			printf("\033[031m ERROR: [Line(%d)] SEMANTIC-ERROR: Function [%s] already exists in given scope ..\033[0m\n", function->linenumber,
				function->func_def._funid);
			return false;
		}
		// else create a function ...
		// First push that to parent stable (scope) ...
		if ( insertSTableFunction( stable, funid, function) == NULL ){	
			printf("\033[031m ERROR: [Line(%d)] SEMANTIC-ERROR: <<<<<<<< INSERT FAILED >>>>> Function [%s] already exists in given scope ..\033[0m\n", function->linenumber,
				function->func_def._funid);
			return false;
		}
	}
	
	function->func_def.stable = createSTable(varsize, funsize, funid, stable);
	// printf("[%d] FUNCTION %s VAR SIZE: %d FUN SIZE: %d \n",depth, funid, varsize, funsize );
	// First insert the input variables then output variables and finally the declared variables 
	// Note: Declaration variables ...
	for ( int i = 0; i < function->func_def.numinparams; i ++) {
		// Try to insert it ...
		function->func_def.inparams[i].id.record = insertSTable(function->func_def.stable, 
			function->func_def.inparams[i].id._id, function->func_def.intypes[i]);
		// if fails return and print error ...
		if ( function->func_def.inparams[i].id.record == NULL ){
			printf("\033[031m ERROR: [Line(%d)] SEMANTIC-ERROR: Input Parameter with same ID [%s] already exists in function [%s]..\033[0m\n", function->linenumber, function->func_def.inparams[i].id._id, function->func_def._funid);
			// return false;
			noerror = false;
		}
		else {
			// If not already existing ... 
			// If no error
			// <----------------------------------------- MODIFIED ...
			// Note: Only elements whose sizes are known are considered initialized ...
			function->func_def.inparams[i].id.record->variable.isinputparam = true ;
	//		if ( function->func_def.inparams[i].id.record->variable.type != T_MATRIX ) 
			function->func_def.inparams[i].id.record->variable.initialized = true; // Set it to initialized ...
	//		else 
	//			function->func_def.inparams[i].id.record->variable.initialized = false; // Set it to uninitialized
		}
	}
	// Now for output params ...
	for ( int i = 0; i < function->func_def.numoutparams; i ++) {
		// Try to insert it ...
		function->func_def.outparams[i].id.record = insertSTable(function->func_def.stable, 
			function->func_def.outparams[i].id._id, function->func_def.outtypes[i]);
		// if fails return and print error ...
		if ( function->func_def.outparams[i].id.record == NULL ){
			printf("\033[031m ERROR: [Line(%d)] SEMANTIC-ERROR: Output Parameter with same ID [%s] already exists in function [%s]..\033[0m\n", function->linenumber, function->func_def.outparams[i].id._id, function->func_def._funid);
			// return false;
			noerror = false;
		}
		else {
			function->func_def.outparams[i].id.record->variable.isoutputparam = true ;
		}
		// Don;t Initialize Output variables ....
		// else {
		// 	function->func_def.outparams[i].id.record->variable.initialized = true; // Set it to initialized ..
		// }
	}
	// Now for declaration stmts ...
	for ( int i = 0; i < function->func_def.numstmts; i ++ ){
		// Also there may be declaration stmt within other stmts ..
		// like if else 
		// <---------------------------- HANDLE IT 
		if ( function->func_def.stmts[i].symbol == NT_DECL_STMT ){
			for ( int j = 0; j < function->func_def.stmts[i].decl_stmt.numvars; j ++ ){
				function->func_def.stmts[i].decl_stmt.vars[j].id.record = insertSTable(function->func_def.stable, 
					function->func_def.stmts[i].decl_stmt.vars[j].id._id, function->func_def.stmts[i].decl_stmt.type);
				// if fails return and print error ...
				if ( function->func_def.stmts[i].decl_stmt.vars[j].id.record == NULL ){
					printf("\033[031m ERROR: [Line(%d)] SEMANTIC-ERROR: Declaration Stmt with same ID [%s] already exists in function [%s]..\033[0m\n", 
						function->func_def.stmts[i].linenumber, function->func_def.stmts[i].decl_stmt.vars[j].id._id, function->func_def._funid);
					// return false;
					noerror = false;
				}
			}
		}
		else if ( function->func_def.stmts[i].symbol == NT_COND_STMT ){
			if ( populatehashtables( &(function->func_def.stmts[i]), depth, function) == false){
				// return false;
					noerror = false;
			} 
			if ( !checkifdeclared( &(function->func_def.stmts[i]), depth, function) ) // <-------------- LATER ADDED
				noerror = false;
		}
		else if ( function->func_def.stmts[i].symbol == NT_FUNC_DEF ) {
			// Now handle function declaration as well...
			if ( !createandpopulatehashtables( &(function->func_def.stmts[i]) , depth + 1, function->func_def.stable) )
				// return false;
				noerror = false;
		}
		else {
			if ( !checkifdeclared( &(function->func_def.stmts[i]), depth, function) ) // <-------------- LATER ADDED
				noerror = false;
		}
		// else if ( function->func_def.stmts[i].symbol == NT_FUNC_DEF )
		// 	funsize ++, createandpopulatehashtables( &(function->func_def.stmts[i]) , depth + 1);
	}

	// Now check if all output variables have been initialized ...
	for ( int i = 0; i < function->func_def.numoutparams; i ++) {
		if ( function->func_def.outparams[i].id.record != NULL ) {
			// If no error in the output params ...
			// check if initialized ...
			if ( ! function->func_def.outparams[i].id.record->variable.initialized  ) {
				printf("\033[031m ERROR: [Line(%d)] SEMANTIC-ERROR: Output Parameter [%s] not initialized in function [%s]..\033[0m\n",
				 function->linenumber, function->func_def.outparams[i].id._id, function->func_def._funid);
				noerror = false ;
			}
		}	
		
	}
	return noerror;
}

