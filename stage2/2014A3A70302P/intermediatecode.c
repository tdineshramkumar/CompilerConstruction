/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 2
	This takes a semantically correct ast .. and generates assembly code
*/



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "lexerDef.h"
#include "parserDef.h"
#include "parser.h"
#include "abstractDef.h"
#include "stable.h"

#define INDEX_TERMINAL(symbol) ( symbol - T_ASSIGN ) // Subtract from first terminal symbol ( To start indexing from begining)

struct _typeinfo{
	SYMBOL type;
	uint numrows; // this contains the number of rows
	uint numcols; // this contains the number of cols
};
typedef struct _typeinfo TypeInfo;
// First function generates intermediate code ...
// First Associate each operation with a temporary variable ...
// Assuming valid types ...
// That is Sematic check is done ...
int newvariablecount = 0;
int labelcount =0; // This is used for jmp label ...
TypeInfo addtempvariables(ASTPNode node, STable *stable){
	
	TypeInfo info, linfo, rinfo;
	char newid[21];
	char jmplabel[41];
	if ( node == NULL ){
		printf("ERROR: CALLED WITH NULL ...\n");
		exit(EXIT_FAILURE);	
	} 
	switch ( node->symbol ){
		case NT_MATRIX:
			info.type = T_MATRIX;
			info.numrows = node->matrix.numrows;
			info.numcols = node->matrix.numcols;
			return info;
		case T_STR:	
			info.type = T_STRING;
			info.numrows = 0;
			info.numcols = 0;
			return info;
		case T_NUM:	
			info.type = T_INT;
			info.numrows = 0;
			info.numcols = 0;
			return info;
		case T_RNUM: 
			info.type = T_REAL;
			info.numrows = 0;
			info.numcols = 0;
			return info;
		case T_ID:	
			// Assuming all the records exist and perfect...
			info.type = node->id.record->variable.type;
			info.numrows = node->id.record->variable.numrows;
			info.numcols = node->id.record->variable.numcols;
			return info;
		case NT_MATRIX_ELEMENT:
			// Assuming matrix declared and initialized
			info.type = T_INT;
			info.numrows = 0;
			info.numcols = 0;
			return info;
		// Note: FOR BOOL EXPRESSION ...
		// LABELS ARE REQUIRED ...
		// AND ARITHMATIC TEMP VARS ...
		case T_NOT:
			// In case of not operand now go to next level ...
			// Interchange the labels ...
			node->operator.left->operator.truelabel = node->operator.falselabel;
			node->operator.left->operator.falselabel = node->operator.truelabel;
			// node->operator.newchildlabel = NULL;
			linfo = addtempvariables(node->operator.left, stable);
			return linfo; // Not used ... in case of booleans
			break;
		case T_AND:
			snprintf(jmplabel,41,"__label%d", labelcount);
			labelcount++;
			node->operator.left->operator.truelabel = strdup(jmplabel); // create a new label ..
			node->operator.left->operator.falselabel = node->operator.falselabel;
			// node->operator.newchildlabel = node->operator.left->operator.truelabel; // just keep a copy of newly created label in parent...
			linfo = addtempvariables(node->operator.left, stable);

			node->operator.right->operator.truelabel = node->operator.truelabel;
			node->operator.right->operator.falselabel = node->operator.falselabel;
			rinfo = addtempvariables(node->operator.right, stable);

			return linfo; // Not used ...
			break; 
		case T_OR:
			snprintf(jmplabel,41,"__label%d", labelcount);
			labelcount++;
			node->operator.left->operator.truelabel = node->operator.truelabel; // create a new label ..
			node->operator.left->operator.falselabel = strdup(jmplabel);
			// node->operator.newchildlabel = node->operator.left->operator.falselabel; // just keep a copy of newly created label in parent...
			linfo = addtempvariables(node->operator.left, stable);

			node->operator.right->operator.truelabel = node->operator.truelabel;
			node->operator.right->operator.falselabel = node->operator.falselabel;
			rinfo = addtempvariables(node->operator.right, stable);
			return linfo; // Not used ...
			break;
		case T_LT:
		case T_LE:
		case T_GT:
		case T_GE:
		case T_EQ:
		case T_NE:
			// Print code Accordingly ...

			break;
		default:
			// printf("[%s]", TerminalSymbols[INDEX_TERMINAL(node->symbol)]);
			// In case of operator ...
			linfo = addtempvariables(node->operator.left, stable);
			rinfo = addtempvariables(node->operator.right, stable);
			// Do this for the root as well ...
			snprintf(newid, 21,"__%d", newvariablecount);
			newvariablecount ++; // increment the count ...
			// Now based on this info create a new variable and insert it to context ...
			// Since we know it is of valid type ...
			// Record will be created ...
			// printf("INSERT %s %d\n",newid, linfo.type);
			node->operator.record = insertSTable( stable, strdup(newid), linfo.type );
			
			if ( node->operator.record == NULL ){
				printf("ERROR inserting temporary variable.. %s\n", newid );
				exit(EXIT_FAILURE);
			}
			if ( linfo.type == T_MATRIX ){
				// Then we need to update index with dimensions.
				UpdateMatrixSTRecord(stable, node->operator.record, linfo.numrows, linfo.numcols);
			}
				// Note: NOT needed use the type of operands ...
				// if ( node->symbol == T_DIV )
				// 	linfo.type = T_REAL; // change it to real ...
			
			return linfo;
	}
}


// Call this function after type checking ...
// stable contains the current scope hash table (that of the function)
// function definitions can be called with NULL....
void addtempvariablesallstmts(ASTPStmt stmt, STable *stable){
	char jmplabel[41];
	if (stmt == NULL ) return;
	switch( stmt->symbol ){
		case NT_PROGRAM:
		case NT_FUNC_DEF:
			
			snprintf(jmplabel,41,"__label%d__%s", labelcount, stmt->func_def._funid );
			labelcount++;
			stmt->func_def.funlabel = strdup(jmplabel);
			
			
			for ( int i = 0 ; i < stmt->func_def.numstmts; i ++ ){
				addtempvariablesallstmts(&(stmt->func_def.stmts[i]), stmt->func_def.stable);
			}
			break;
		case NT_DECL_STMT:
			break;
		case NT_COND_STMT:
			// Add Labels for next and else ...
			snprintf(jmplabel,41,"__label%d", labelcount);
			labelcount++;
			stmt->cond_stmt.iflabel = strdup(jmplabel);
			snprintf(jmplabel,41,"__label%d", labelcount);
			labelcount++;
			stmt->cond_stmt.elselabel = strdup(jmplabel);
			snprintf(jmplabel,41,"__label%d", labelcount);
			labelcount++;
			stmt->cond_stmt.nextlabel = strdup(jmplabel);

			// Now Create the Labels to jump to incase of true or false
			// snprintf(jmplabel,41,"__label%d", labelcount);
			// labelcount++;
			// stmt->cond_stmt.condition->operator.truelabel = strdup(jmplabel);
			// snprintf(jmplabel,41,"__label%d", labelcount);
			// labelcount++;
			// stmt->cond_stmt.condition->operator.falselabel = strdup(jmplabel);
			stmt->cond_stmt.condition->operator.truelabel = stmt->cond_stmt.iflabel; // if condition is true go to if label
			stmt->cond_stmt.condition->operator.falselabel = stmt->cond_stmt.elselabel; // else go to else label (Note: Else stmts may not be there...)

			// Create a new variable for root of condition ,,,
			addtempvariables(stmt->cond_stmt.condition, stable);
			

			for ( int i = 0; i <  stmt->cond_stmt.numifstmts; i ++)
				addtempvariablesallstmts(&(stmt->cond_stmt.ifstmts[i]), stable);		
	
			for ( int i = 0; i <  stmt->cond_stmt.numelsestmts; i ++)
				addtempvariablesallstmts(&(stmt->cond_stmt.elsestmts[i]), stable);
			break;
		case T_READ:
		case T_PRINT:
			break;
		case NT_FUNC_CALL:{
			// If Simply a function call 
			// The also add some temporary variables
			// TO hold the return values ...
			char newid[21];
			ASTPStmt funcstmt = stmt->func_call.record->function.function ; // get the corresponding function stmt
			stmt->func_call.numoutrecords = funcstmt->func_def.numoutparams;
			stmt->func_call.outrecords = malloc(sizeof(SRecord *)* stmt->func_call.numoutrecords); // Allocate pointers of array of record pointers ...
			for ( int i = 0 ; i < stmt->func_call.numoutrecords ; i ++ ){
				snprintf(newid, 21,"__%d", newvariablecount);
				newvariablecount ++; // increment the count ...

				// Now based on this info create a new variable and insert it to context ...
				// Since we know it is of valid type ...
				// Record will be created ...
				// printf("INSERT %s %d\n",newid, linfo.type);
				stmt->func_call.outrecords[i] = insertSTable( stable, strdup(newid), funcstmt->func_def.outtypes[i] );
				if ( stmt->func_call.outrecords[i] == NULL ){
					printf("ERROR inserting temporary variable.. %s\n", newid );
					exit(EXIT_FAILURE);
				}
				if ( funcstmt->func_def.outtypes[i] == T_MATRIX ){
					// Then we need to update index with dimensions.
					// printf("OUTPARAMS: %d %d\n",funcstmt->func_def.outparams[i].id.record->variable.numrows, 
					// 	funcstmt->func_def.outparams[i].id.record->variable.numcols);
					UpdateMatrixSTRecord(stable, stmt->func_call.outrecords[i], 
						funcstmt->func_def.outparams[i].id.record->variable.numrows, 
						funcstmt->func_def.outparams[i].id.record->variable.numcols);
				}

			}
			// Now add the label for function call
			stmt->func_call.funlabel = funcstmt->func_def.funlabel;
			break;
		}
		case NT_EXPR1:
		case NT_EXPR2:
			switch ( stmt->assign_stmt.rhs.symbol){
				case T_SIZE:
				case T_ID:
					break;
				case NT_FUNC_CALL:
					// Add the label for function call ..
					stmt->assign_stmt.rhs.func_call->func_call.funlabel = stmt->assign_stmt.rhs.func_call->func_call.record->function.function->func_def.funlabel;
					break;
				case NT_ARITH_EXPR:
					// ROOT: FALSE Because no new node is needed ...
					// Use the existing variable ... 
					addtempvariables(stmt->assign_stmt.rhs.var, stable);
					break;
			}
			break;
	}
}

void addalltemps(ASTPStmt mainstmt){
	newvariablecount = 0;
	addtempvariablesallstmts(mainstmt, NULL);
}
// This function generates the data code ...
// Later Modify code ..
// Int is used with 64 bit so convert all int to long everywhere...
uint constnum = 0;
FILE * fd ;
void printDataSectionASTNode(ASTPNode node){
	char newlabel[51];
	
	if ( node == NULL ) return;
	switch ( node->symbol ){
		case NT_MATRIX:
		// 	// Column Major....
			// snprintf(newlabel,51, "__const%d", constnum);
			// constnum ++;
			// node->matrix.label = strdup(newlabel); // set this for later use for reference
			// printf("\t%s DQ", newlabel); // Long int ...
			// for ( int i = 0 ; i < node->matrix.numrows; i ++ ){
			// 	for ( int j = 0 ;j < node->matrix.numcols; j ++) {
			// 		printf(" %d", node->matrix.matrix[i][j]);
			// 		if ( i != node->matrix.numrows-1 || j != node->matrix.numcols-1) 
			// 			printf(",");
			// 	}
			// }
			// printf("\n");
			break;
		case T_STR:	
			// Only store strings ...
			// Rest use instruction..
			snprintf(newlabel, 51,"__const%d", constnum);
			constnum ++;
			node->constant.label = strdup(newlabel);
			fprintf(fd,"\t%s DB %s,0\n", newlabel, node->constant._str); // bytes , 0 -> end of string
			break;
		case T_NUM:	
			// snprintf(newlabel, 51, "__const%d", constnum);
			// constnum ++;
			// node->constant.label = strdup(newlabel);
			// printf("\t%s DQ %d\n", newlabel, node->constant._int);
		 	break;
		case T_RNUM:
			// snprintf(newlabel, 51, "__const%d", constnum);
			// constnum ++;
			// node->constant.label = strdup(newlabel);
			// printf("\t%s DQ %f\n", newlabel, node->constant._real);
		 	break;
		case T_ID:
		case NT_MATRIX_ELEMENT:
		// 	// Do nothing ...
		// 	// Compute offsets and compute in instructions ...
		// 	// Don't Need to save explicitly ...
		 	break; // Do nothing 
		case T_NOT:
		 	printDataSectionASTNode( node->operator.left); // Call on left ..
		 	break;
		// Inother operators 
		default:
			printDataSectionASTNode( node->operator.left ); // Call on left
			printDataSectionASTNode( node->operator.right ); // Call on right
	}
}

void printDataSectionASTStmt(ASTPStmt stmt){
	if (stmt == NULL ) return;
	switch( stmt->symbol ){
		case NT_PROGRAM:
			for ( int i = 0 ; i < stmt->func_def.numstmts; i ++ ){
				printDataSectionASTStmt(&(stmt->func_def.stmts[i]));
			}
			break;
		case NT_FUNC_DEF:
			// Check if input parameter is a constant ...
			// Function definition no need to look at params only body ..
			for ( int i = 0 ; i < stmt->func_def.numstmts; i ++ ){
				printDataSectionASTStmt(&(stmt->func_def.stmts[i]));
			}
			break;
		case NT_DECL_STMT:
			break;
		case NT_COND_STMT:
			printDataSectionASTNode(stmt->cond_stmt.condition);
			for ( int i = 0; i <  stmt->cond_stmt.numifstmts; i ++){
				printDataSectionASTStmt(&(stmt->cond_stmt.ifstmts[i]));
			}
			for ( int i = 0; i <  stmt->cond_stmt.numelsestmts; i ++){
				printDataSectionASTStmt(&(stmt->cond_stmt.elsestmts[i]));
			}
			break;
		case T_READ:
		case T_PRINT:
			break;
		case NT_FUNC_CALL:
			for ( int i = 0 ; i <  stmt->func_call.numinargs ; i ++ ){
				printDataSectionASTNode(&(stmt->func_call.inargs[i]));
			}
			break;
		case NT_EXPR1:
		case NT_EXPR2:
			// printf(" EXPR: LHS");
			switch( stmt->assign_stmt.lhs.symbol ) {
				case T_ID:
					printDataSectionASTNode(stmt->assign_stmt.lhs.var);
					break;
				case NT_VARLIST:
					for ( int i = 0 ; i < stmt->assign_stmt.lhs.numvars; i ++ ){
						printDataSectionASTNode(&(stmt->assign_stmt.lhs.varlist[i]));
					}
					break;
			}
			switch ( stmt->assign_stmt.rhs.symbol){
				case T_SIZE:
				case T_ID:
				case NT_ARITH_EXPR:
					printDataSectionASTNode(stmt->assign_stmt.rhs.var);
					break;
				case NT_FUNC_CALL:
					for ( int i = 0 ; i <  stmt->assign_stmt.rhs.func_call->func_call.numinargs ; i ++ ){
						printDataSectionASTNode(&(stmt->assign_stmt.rhs.func_call->func_call.inargs[i]));
					}
					break;
			}
			break;
	}
}

void printCodeSectionASTNode(ASTPNode node){
	char newlabel[51];
	
	if ( node == NULL ) return;
	switch ( node->symbol ){
		case NT_MATRIX:
		// 	// Column Major....
			// snprintf(newlabel,51, "__const%d", constnum);
			// constnum ++;
			// node->matrix.label = strdup(newlabel); // set this for later use for reference
			// printf("\t%s DQ", newlabel); // Long int ...
			// for ( int i = 0 ; i < node->matrix.numrows; i ++ ){
			// 	for ( int j = 0 ;j < node->matrix.numcols; j ++) {
			// 		printf(" %d", node->matrix.matrix[i][j]);
			// 		if ( i != node->matrix.numrows-1 || j != node->matrix.numcols-1) 
			// 			printf(",");
			// 	}
			// }
			// printf("\n");
			break;
		case T_STR:	
			// Only store strings ...
			// Rest use instruction..
			// snprintf(newlabel, 51,"__const%d", constnum);
			// constnum ++;
			// node->constant.label = strdup(newlabel);
			// printf("\t%s DB %s,0\n", newlabel, node->constant._str); // bytes , 0 -> end of string
			break;
		case T_NUM:
			// printf("%d\n", node->constant._int);	
			// snprintf(newlabel, 51, "__const%d", constnum);
			// constnum ++;
			// node->constant.label = strdup(newlabel);
			// printf("\t%s DQ %d\n", newlabel, node->constant._int);
		 	break;
		case T_RNUM:
			// printf("%f\n", node->constant._real);
			// snprintf(newlabel, 51, "__const%d", constnum);
			// constnum ++;
			// node->constant.label = strdup(newlabel);
			// printf("\t%s DQ %f\n", newlabel, node->constant._real);
		 	break;
		case T_ID:

			break;
		case NT_MATRIX_ELEMENT:
			// NOTE: Not used in comparison
		// 	// Do nothing ...
		// 	// Compute offsets and compute in instructions ...
		// 	// Don't Need to save explicitly ...
		 	break; // Do nothing 
		
		case T_NOT:
		 	printCodeSectionASTNode( node->operator.left); // Call on left ..
		 	break;
		// Inother operators 
		case T_AND:
			printCodeSectionASTNode( node->operator.left ); // Call on left
			fprintf(fd,"%s:\n",node->operator.left->operator.truelabel);
			printCodeSectionASTNode( node->operator.right ); // Call on right
			/*
snprintf(jmplabel,41,"__label%d", labelcount);
			labelcount++;
			node->operator.left->operator.truelabel = strdup(jmplabel); // create a new label ..
			node->operator.left->operator.falselabel = node->operator.falselabel;
			// node->operator.newchildlabel = node->operator.left->operator.truelabel; // just keep a copy of newly created label in parent...
			linfo = addtempvariables(node->operator.left, stable, false);

			node->operator.right->operator.truelabel = node->operator.truelabel;
			node->operator.right->operator.falselabel = node->operator.falselabel;
			rinfo = addtempvariables(node->operator.right, stable, false);
			*/
			break;
		case T_OR:
			printCodeSectionASTNode( node->operator.left ); // Call on left
			fprintf(fd,"%s:\n",node->operator.left->operator.falselabel);
			printCodeSectionASTNode( node->operator.right ); // Call on right
			break;
		case T_LE:
			// Incase of less than...
			// Move values to register and then compare
			// Check the type of operands ....
			if ( (node->operator.left->symbol == T_ID && node->operator.left->id.record->variable.type == T_INT) 
				|| (node->operator.left->symbol == T_NUM ) ) {
				// Incase of int operators ...
				if ( node->operator.left->symbol == T_ID ) {
					// In Memory then load it into register ..
					// Use the offsets ...
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->id.record->variable.offset );
				}
				else {
					// If constant ...
					fprintf(fd,"\tmov eax, %d\n", node->operator.left->constant._int);
				}

				if ( node->operator.right->symbol == T_ID ){
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->id.record->variable.offset );
				}
				else {
					fprintf(fd,"\tmov ebx, %d\n", node->operator.right->constant._int);
				}
				fprintf(fd,"\tcmp eax, ebx\n\tjle %s\n\tjmp %s\n", node->operator.truelabel, node->operator.falselabel);
			}
			else {
				// Floating point instruction ..
				// Handle them later ...
				// printf("FLOATING POINT INSTRUCTION\n");
			}
			break;
		case T_LT:
			if ( (node->operator.left->symbol == T_ID && node->operator.left->id.record->variable.type == T_INT) 
				|| (node->operator.left->symbol == T_NUM ) ) {
				// Incase of int operators ...
				if ( node->operator.left->symbol == T_ID ) {
					// In Memory then load it into register ..
					// Use the offsets ...
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->id.record->variable.offset );
				}
				else {
					// If constant ...
					fprintf(fd,"\tmov eax, %d\n", node->operator.left->constant._int);
				}

				if ( node->operator.right->symbol == T_ID ){
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->id.record->variable.offset );
				}
				else {
					fprintf(fd,"\tmov ebx, %d\n", node->operator.right->constant._int);
				}
				fprintf(fd,"\tcmp eax, ebx\n\tjl %s\n\tjmp %s\n", node->operator.truelabel, node->operator.falselabel);
			}
			else {
				// Floating point instruction ..
				// Handle them later ...
				// printf("FLOATING POINT INSTRUCTION\n");
			}
			break;
		case T_GE:
			if ( (node->operator.left->symbol == T_ID && node->operator.left->id.record->variable.type == T_INT) 
				|| (node->operator.left->symbol == T_NUM ) ) {
				// Incase of int operators ...
				if ( node->operator.left->symbol == T_ID ) {
					// In Memory then load it into register ..
					// Use the offsets ...
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->id.record->variable.offset );
				}
				else {
					// If constant ...
					fprintf(fd,"\tmov eax, %d\n", node->operator.left->constant._int);
				}

				if ( node->operator.right->symbol == T_ID ){
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->id.record->variable.offset );
				}
				else {
					fprintf(fd,"\tmov ebx, %d\n", node->operator.right->constant._int);
				}
				fprintf(fd,"\tcmp eax, ebx\n\tjge %s\n\tjmp %s\n", node->operator.truelabel, node->operator.falselabel);
			}
			else {
				// Floating point instruction ..
				// Handle them later ...
				// printf("FLOATING POINT INSTRUCTION\n");
			}
			break;
		case T_GT:
			if ( (node->operator.left->symbol == T_ID && node->operator.left->id.record->variable.type == T_INT) 
				|| (node->operator.left->symbol == T_NUM ) ) {
				// Incase of int operators ...
				if ( node->operator.left->symbol == T_ID ) {
					// In Memory then load it into register ..
					// Use the offsets ...
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->id.record->variable.offset );
				}
				else {
					// If constant ...
					fprintf(fd,"\tmov eax, %d\n", node->operator.left->constant._int);
				}

				if ( node->operator.right->symbol == T_ID ){
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->id.record->variable.offset );
				}
				else {
					fprintf(fd,"\tmov ebx, %d\n", node->operator.right->constant._int);
				}
				fprintf(fd,"\tcmp eax, ebx\n\tjg %s\n\tjmp %s\n", node->operator.truelabel, node->operator.falselabel);
			}
			else {
				// Floating point instruction ..
				// Handle them later ...
				// printf("FLOATING POINT INSTRUCTION\n");
			}
			break;
		case T_EQ:
			if ( (node->operator.left->symbol == T_ID && node->operator.left->id.record->variable.type == T_INT) 
				|| (node->operator.left->symbol == T_NUM ) ) {
				// Incase of int operators ...
				if ( node->operator.left->symbol == T_ID ) {
					// In Memory then load it into register ..
					// Use the offsets ...
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->id.record->variable.offset );
				}
				else {
					// If constant ...
					fprintf(fd,"\tmov eax, %d\n", node->operator.left->constant._int);
				}

				if ( node->operator.right->symbol == T_ID ){
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->id.record->variable.offset );
				}
				else {
					fprintf(fd,"\tmov ebx, %d\n", node->operator.right->constant._int);
				}
				fprintf(fd,"\tcmp eax, ebx\n\tje %s\n\tjmp %s\n", node->operator.truelabel, node->operator.falselabel);
			}
			else {
				// Floating point instruction ..
				// Handle them later ...
				// printf("FLOATING POINT INSTRUCTION\n");
			}
			break;
		case T_NE:
			if ( (node->operator.left->symbol == T_ID && node->operator.left->id.record->variable.type == T_INT) 
				|| (node->operator.left->symbol == T_NUM ) ) {
				// Incase of int operators ...
				if ( node->operator.left->symbol == T_ID ) {
					// In Memory then load it into register ..
					// Use the offsets ...
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->id.record->variable.offset );
				}
				else {
					// If constant ...
					fprintf(fd,"\tmov eax, %d\n", node->operator.left->constant._int);
				}

				if ( node->operator.right->symbol == T_ID ){
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->id.record->variable.offset );
				}
				else {
					fprintf(fd,"\tmov ebx, %d\n", node->operator.right->constant._int);
				}
				fprintf(fd,"\tcmp eax, ebx\n\tjne %s\n\tjmp %s\n", node->operator.truelabel, node->operator.falselabel);
			}
			else {
				// Floating point instruction ..
				// Handle them later ...
				// printf("FLOATING POINT INSTRUCTION\n");
			}
			break;
		case T_PLUS:
			// If an INT 
			if ( node->operator.record->variable.type == T_INT ) {
				// Check if more operands ..
				// First compute them if any ..
				// <--------------------_MODIFIED 
				if (  node->operator.left->symbol != T_ID &&  node->operator.left->symbol != T_NUM && node->operator.left->symbol != NT_MATRIX_ELEMENT ){
					// Then some operator
					// Compute it first ..
					printCodeSectionASTNode( node->operator.left );
				}
				if (  node->operator.right->symbol != T_ID &&  node->operator.right->symbol != T_NUM && node->operator.right->symbol != NT_MATRIX_ELEMENT){
					// Then some operator
					// Compute it first ..
					printCodeSectionASTNode( node->operator.right ); 
				}
				// Now once we have the values ...
				// Compute the sum ...
				if ( node->operator.left->symbol == T_ID ) {
					// In Memory then load it into register ..
					// Use the offsets ...
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->id.record->variable.offset );
				}
				else if ( node->operator.left->symbol == T_NUM ) {
					// If constant ...
					fprintf(fd,"\tmov eax, %d\n", node->operator.left->constant._int);
				}
				else if ( node->operator.left->symbol == NT_MATRIX_ELEMENT ){
					// Assuming indexing are right ..
					//  ( row * numcols + col ) * sizeof(int) 
					int row = node->operator.left->matrix_element.indices[0];
					int col = node->operator.left->matrix_element.indices[1];
					int numcols = node->operator.left->matrix_element.record->variable.numcols;
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->matrix_element.record->variable.offset + ( row * numcols + col ) * sizeof(int)  );

				} 
				else {
					// Load from memory ..
					// Else some operator was present ...
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->operator.record->variable.offset );
				}
				// Next Operator...
				if ( node->operator.right->symbol == T_ID ) {
					// In Memory then load it into register ..
					// Use the offsets ...
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->id.record->variable.offset );
				}
				else if ( node->operator.right->symbol == T_NUM ) {
					// If constant ...
					fprintf(fd,"\tmov ebx, %d\n", node->operator.right->constant._int);
				}	
				else if ( node->operator.right->symbol == NT_MATRIX_ELEMENT ){
					// Assuming indexing are right ..
					//  ( row * numcols + col ) * sizeof(int) 
					// printf("MATRIX ELEMENT\n");
					int row = node->operator.right->matrix_element.indices[0];
					int col = node->operator.right->matrix_element.indices[1];
					int numcols = node->operator.right->matrix_element.record->variable.numcols;
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->matrix_element.record->variable.offset + ( row * numcols + col ) * sizeof(int)  );

				} 
				else {
					// Load from memory ..
					// Else some operator was present ...
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->operator.record->variable.offset );
				}
				// Add the values and move back the result ..
				fprintf(fd,"\tadd eax, ebx\n\tmov [rsp+%d], eax\n", node->operator.record->variable.offset);
			}
			else if ( node->operator.record->variable.type == T_MATRIX ) {
				// Check if more operands ..
				if (  node->operator.left->symbol != T_ID &&  node->operator.left->symbol != T_MATRIX ){
					// Then some operator
					// Compute it first ..
					printCodeSectionASTNode( node->operator.left );
				}
				if (  node->operator.right->symbol != T_ID &&  node->operator.right->symbol != T_MATRIX ){
					// Then some operator
					// Compute it first ..
					printCodeSectionASTNode( node->operator.right ); 
				}
				// Now once we have the values ...
				// Compute the sum ...
				// Matrix ..
				// Many Instructions ..
				int numcols =  node->operator.record->variable.numcols ;
				for ( int row = 0; row < node->operator.record->variable.numrows ; row ++ ){
					for ( int col = 0; col < node->operator.record->variable.numcols ; col ++) {
						if ( node->operator.left->symbol == T_ID ) {
							fprintf(fd, "\tmov eax, [rsp+%d]\n" ,node->operator.left->id.record->variable.offset + ( row * numcols + col ) * sizeof(int));
						}
						else if ( node->operator.left->symbol == T_MATRIX ) {
							 fprintf(fd,"\tmov eax, %d\n", node->operator.left->matrix.matrix[row][col]);
						}
						else {
							fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->operator.record->variable.offset + ( row * numcols + col ) * sizeof(int));
						}

						if ( node->operator.right->symbol == T_ID ) {
							fprintf(fd, "\tmov ebx, [rsp+%d]\n" ,node->operator.right->id.record->variable.offset + ( row * numcols + col ) * sizeof(int));
						}
						else if ( node->operator.right->symbol == T_MATRIX ) {
							 fprintf(fd,"\tmov ebx, %d\n", node->operator.right->matrix.matrix[row][col]);
						}
						else {
							fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->operator.record->variable.offset + ( row * numcols + col ) * sizeof(int));
						}

						fprintf(fd,"\tadd eax, ebx\n\tmov [rsp+%d], eax\n", node->operator.record->variable.offset + ( row * numcols + col ) * sizeof(int));
					}
				}
			}
			else if ( node->operator.record->variable.type == T_STRING ) { 
				// Handle String .. Operations in assembly ??
				// printf("STRING ADDITION..\n");
				if (  node->operator.left->symbol != T_ID &&  node->operator.left->symbol != T_STR ){
					// Then some operator
					// Compute it first ..
					printCodeSectionASTNode( node->operator.left );
				}
				if (  node->operator.right->symbol != T_ID &&  node->operator.right->symbol != T_STR ){
					// Then some operator
					// Compute it first ..
					printCodeSectionASTNode( node->operator.right ); 
				}
				// Now once we have the values ...
				// Compute the sum ...
				if ( node->operator.left->symbol == T_ID ) {
					// Use the offsets ...
					fprintf(fd,"\tmov rsi, rsp\n\tadd rsi, %d\n", node->operator.left->id.record->variable.offset );
				}
				else if ( node->operator.left->symbol == T_STR ) {
					// If constant ...
					fprintf(fd,"\tmov rsi, %s\n", node->operator.left->constant.label);
				}
				else {
					// Else some operator was present ...
					fprintf(fd,"\tmov rsi, rsp\n\tadd rsi, %d\n", node->operator.left->operator.record->variable.offset );
				}
				if ( node->operator.right->symbol == T_ID ) {
					// Use the offsets ...
					fprintf(fd,"\tmov rdx, rsp\n\tadd rdx, %d\n", node->operator.right->id.record->variable.offset );\
				}
				else if ( node->operator.right->symbol == T_STR ) {
					// If constant ...
					fprintf(fd,"\tmov rdx, %s\n", node->operator.right->constant.label);
				}
				else {
					fprintf(fd,"\tmov rdx, rsp\n\tadd rdx, %d\n", node->operator.right->operator.record->variable.offset );
				}
				fprintf(fd,"\tmov rdi, rsp\n\tadd rdi, %d\n", node->operator.record->variable.offset);
				fprintf(fd,"\tcall __strcat\n");
			}
			else {
				// printf("REAL ADDITION\n");
			}
			break;
		case T_MINUS:
			if ( node->operator.record->variable.type == T_INT ) {
				// Check if more operands ..
				// First compute them if any ..
				if (  node->operator.left->symbol != T_ID &&  node->operator.left->symbol != T_NUM && node->operator.left->symbol != NT_MATRIX_ELEMENT ){
					// Then some operator
					// Compute it first ..
					printCodeSectionASTNode( node->operator.left );
				}
				if (  node->operator.right->symbol != T_ID &&  node->operator.right->symbol != T_NUM && node->operator.right->symbol != NT_MATRIX_ELEMENT ){
					// Then some operator
					// Compute it first ..
					printCodeSectionASTNode( node->operator.right ); 
				}
				// Now once we have the values ...
				// Compute the difference ...
				if ( node->operator.left->symbol == T_ID ) {
					// In Memory then load it into register ..
					// Use the offsets ...
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->id.record->variable.offset );
				}
				else if ( node->operator.left->symbol == T_NUM ) {
					// If constant ...
					fprintf(fd,"\tmov eax, %d\n", node->operator.left->constant._int);
				}
				else if ( node->operator.left->symbol == NT_MATRIX_ELEMENT ){
					// Assuming indexing are right ..
					//  ( row * numcols + col ) * sizeof(int) 
					int row = node->operator.left->matrix_element.indices[0];
					int col = node->operator.left->matrix_element.indices[1];
					int numcols = node->operator.left->matrix_element.record->variable.numcols;
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->matrix_element.record->variable.offset + ( row * numcols + col ) * sizeof(int)  );

				} 
				else {
					// Load from memory ..
					// Else some operator was present ...
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->operator.record->variable.offset );
				}
				// Next Operator...
				if ( node->operator.right->symbol == T_ID ) {
					// In Memory then load it into register ..
					// Use the offsets ...
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->id.record->variable.offset );
				}
				else if ( node->operator.right->symbol == T_NUM ) {
					// If constant ...
					fprintf(fd,"\tmov ebx, %d\n", node->operator.right->constant._int);
				}
				else if ( node->operator.right->symbol == NT_MATRIX_ELEMENT ){
					// Assuming indexing are right ..
					//  ( row * numcols + col ) * sizeof(int) 
					int row = node->operator.right->matrix_element.indices[0];
					int col = node->operator.right->matrix_element.indices[1];
					int numcols = node->operator.right->matrix_element.record->variable.numcols;
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->matrix_element.record->variable.offset + ( row * numcols + col ) * sizeof(int)  );

				} 
				else {
					// Load from memory ..
					// Else some operator was present ...
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->operator.record->variable.offset );
				}
				// Add the values and move back the result ..
				fprintf(fd,"\tsub eax, ebx\n\tmov [rsp+%d], eax\n", node->operator.record->variable.offset);
			}
			else if ( node->operator.record->variable.type == T_MATRIX ) {
				// Check if more operands ..
				if (  node->operator.left->symbol != T_ID &&  node->operator.left->symbol != T_MATRIX ){
					// Then some operator
					// Compute it first ..
					printCodeSectionASTNode( node->operator.left );
				}
				if (  node->operator.right->symbol != T_ID &&  node->operator.right->symbol != T_MATRIX ){
					// Then some operator
					// Compute it first ..
					printCodeSectionASTNode( node->operator.right ); 
				}
				// Now once we have the values ...
				// Compute the difference ...
				// Matrix ..
				// Many Instructions ..
				int numcols =  node->operator.record->variable.numcols ;
				for ( int row = 0; row < node->operator.record->variable.numrows ; row ++ ){
					for ( int col = 0; col < node->operator.record->variable.numcols ; col ++) {
						if ( node->operator.left->symbol == T_ID ) {
							fprintf(fd, "\tmov eax, [rsp+%d]\n" ,node->operator.left->id.record->variable.offset + ( row * numcols + col ) * sizeof(int));
						}
						else if ( node->operator.left->symbol == T_MATRIX ) {
							 fprintf(fd,"\tmov eax, %d\n", node->operator.left->matrix.matrix[row][col]);
						}
						else {
							fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->operator.record->variable.offset + ( row * numcols + col ) * sizeof(int));
						}

						if ( node->operator.right->symbol == T_ID ) {
							fprintf(fd, "\tmov ebx, [rsp+%d]\n" ,node->operator.right->id.record->variable.offset + ( row * numcols + col ) * sizeof(int));
						}
						else if ( node->operator.right->symbol == T_MATRIX ) {
							 fprintf(fd,"\tmov ebx, %d\n", node->operator.right->matrix.matrix[row][col]);
						}
						else {
							fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->operator.record->variable.offset + ( row * numcols + col ) * sizeof(int));
						}

						fprintf(fd,"\tsub eax, ebx\n\tmov [rsp+%d], eax\n", node->operator.record->variable.offset + ( row * numcols + col ) * sizeof(int));
					}
				}
			}
			else {
				// printf("REAL ADDITION\n");
			}
			//  No string subtraction ...
			break;
		case T_MUL:
			if ( node->operator.record->variable.type == T_INT ) {
				// Check if more operands ..
				// First compute them if any ..
				if (  node->operator.left->symbol != T_ID &&  node->operator.left->symbol != T_NUM && node->operator.left->symbol != NT_MATRIX_ELEMENT ){
					// Then some operator
					// Compute it first ..
					printCodeSectionASTNode( node->operator.left );
				}
				if (  node->operator.right->symbol != T_ID &&  node->operator.right->symbol != T_NUM && node->operator.right->symbol != NT_MATRIX_ELEMENT){
					// Then some operator
					// Compute it first ..
					printCodeSectionASTNode( node->operator.right ); 
				}
				// Now once we have the values ...
				// Compute the multiplication ...
				if ( node->operator.left->symbol == T_ID ) {
					// In Memory then load it into register ..
					// Use the offsets ...
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->id.record->variable.offset );
				}
				else if ( node->operator.left->symbol == T_NUM ) {
					// If constant ...
					fprintf(fd,"\tmov eax, %d\n", node->operator.left->constant._int);
				}
				else if ( node->operator.left->symbol == NT_MATRIX_ELEMENT ){
					// Assuming indexing are right ..
					//  ( row * numcols + col ) * sizeof(int) 
					int row = node->operator.left->matrix_element.indices[0];
					int col = node->operator.left->matrix_element.indices[1];
					int numcols = node->operator.left->matrix_element.record->variable.numcols;
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->matrix_element.record->variable.offset + ( row * numcols + col ) * sizeof(int)  );

				} 
				else {
					// Load from memory ..
					// Else some operator was present ...
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->operator.record->variable.offset );
				}
				// Next Operator...
				if ( node->operator.right->symbol == T_ID ) {
					// In Memory then load it into register ..
					// Use the offsets ...
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->id.record->variable.offset );
				}
				else if ( node->operator.right->symbol == T_NUM ) {
					// If constant ...
					fprintf(fd,"\tmov ebx, %d\n", node->operator.right->constant._int);
				}
				else if ( node->operator.right->symbol == NT_MATRIX_ELEMENT ){
					// Assuming indexing are right ..
					//  ( row * numcols + col ) * sizeof(int) 
					int row = node->operator.right->matrix_element.indices[0];
					int col = node->operator.right->matrix_element.indices[1];
					int numcols = node->operator.right->matrix_element.record->variable.numcols;
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->matrix_element.record->variable.offset + ( row * numcols + col ) * sizeof(int)  );

				} 
				else {
					// Load from memory ..
					// Else some operator was present ...
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->operator.record->variable.offset );
				}
				// Multiply the values and move back the result ..
				fprintf(fd,"\txor rdx, rdx\n\tmul ebx\n\tmov [rsp+%d], eax\n", node->operator.record->variable.offset);
			}
			else {
				// printf("REAL MULTIPLICATION\n");
			}
			break;
		case T_DIV:
			if ( node->operator.record->variable.type == T_INT ) {
				// Check if more operands ..
				// First compute them if any ..
				if (  node->operator.left->symbol != T_ID &&  node->operator.left->symbol != T_NUM & node->operator.left->symbol != NT_MATRIX_ELEMENT){
					// Then some operator
					// Compute it first ..
					printCodeSectionASTNode( node->operator.left );
				}
				if (  node->operator.right->symbol != T_ID &&  node->operator.right->symbol != T_NUM & node->operator.right->symbol != NT_MATRIX_ELEMENT){
					// Then some operator
					// Compute it first ..
					printCodeSectionASTNode( node->operator.right ); 
				}
				// Now once we have the values ...
				// Compute the division ...
				if ( node->operator.left->symbol == T_ID ) {
					// In Memory then load it into register ..
					// Use the offsets ...
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->id.record->variable.offset );
				}
				else if ( node->operator.left->symbol == T_NUM ) {
					// If constant ...
					fprintf(fd,"\tmov eax, %d\n", node->operator.left->constant._int);
				}
				else if ( node->operator.left->symbol == NT_MATRIX_ELEMENT ){
					// Assuming indexing are right ..
					//  ( row * numcols + col ) * sizeof(int) 
					int row = node->operator.left->matrix_element.indices[0];
					int col = node->operator.left->matrix_element.indices[1];
					int numcols = node->operator.left->matrix_element.record->variable.numcols;
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->matrix_element.record->variable.offset + ( row * numcols + col ) * sizeof(int)  );

				} 
				else {
					// Load from memory ..
					// Else some operator was present ...
					fprintf(fd,"\tmov eax, [rsp+%d]\n", node->operator.left->operator.record->variable.offset );
				}
				// Next Operator...
				if ( node->operator.right->symbol == T_ID ) {
					// In Memory then load it into register ..
					// Use the offsets ...
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->id.record->variable.offset );
				}
				else if ( node->operator.right->symbol == T_NUM ) {
					// If constant ...
					fprintf(fd,"\tmov ebx, %d\n", node->operator.right->constant._int);
				}
				else if ( node->operator.right->symbol == NT_MATRIX_ELEMENT ){
					// Assuming indexing are right ..
					//  ( row * numcols + col ) * sizeof(int) 
					int row = node->operator.right->matrix_element.indices[0];
					int col = node->operator.right->matrix_element.indices[1];
					int numcols = node->operator.right->matrix_element.record->variable.numcols;
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->matrix_element.record->variable.offset + ( row * numcols + col ) * sizeof(int)  );

				} 
				else {
					// Load from memory ..
					// Else some operator was present ...
					fprintf(fd,"\tmov ebx, [rsp+%d]\n", node->operator.right->operator.record->variable.offset );
				}
				// Divide the values and move back the result ..
				fprintf(fd,"\txor rdx, rdx\n\tdiv ebx\n\tmov [rsp+%d], eax\n", node->operator.record->variable.offset);
			}
			else {
				// printf("REAL DIVISION\n");
			}
			break;
		default:
			printf("Unknown SYmbol...%s\n", TerminalSymbols[INDEX_TERMINAL(node->symbol)]);
			exit(EXIT_FAILURE);
	}
}

void printCodeSectionASTStmt(ASTPStmt stmt){
	if (stmt == NULL ) return;
	switch( stmt->symbol ){
		case NT_PROGRAM:
			fprintf(fd,"%s:\n", stmt->func_def.funlabel );
			fprintf(fd,"\tpush rbp\n");
			fprintf(fd,"\tmov rbp, rsp\n");
			fprintf(fd,"\tsub rsp, %d\n", stmt->func_def.stable->varoff); // Allocate the local variables
			for ( int i = 0 ; i < stmt->func_def.numstmts; i ++ ){
				if ( stmt->func_def.stmts[i].symbol != NT_FUNC_DEF ) // skip function declaration stmt
					printCodeSectionASTStmt(&(stmt->func_def.stmts[i]));
			}
			// fprintf(fd,"\tmov rax, 60\n");
			// fprintf(fd,"\tmov rdi, 0\n");
			// fprintf(fd,"\tsyscall\n");
			fprintf(fd,"\tmov rsp, rbp\n");
			fprintf(fd,"\tpop rbp\n");
			fprintf(fd,"\tret\n");
			
			for ( int i = 0 ; i < stmt->func_def.numstmts; i ++ ){
				if ( stmt->func_def.stmts[i].symbol == NT_FUNC_DEF ) // now run on function declaration stmt
					printCodeSectionASTStmt(&(stmt->func_def.stmts[i]));
			}
			break;
		case NT_FUNC_DEF:
			fprintf(fd,"%s:\n", stmt->func_def.funlabel );
			fprintf(fd,"\tpush rbp\n");
			fprintf(fd,"\tmov rbp, rsp\n");
			fprintf(fd,"\tsub rsp, %d\n", stmt->func_def.stable->varoff); // Allocate the local variables
			for ( int i = 0 ; i < stmt->func_def.numstmts; i ++ ){
				if ( stmt->func_def.stmts[i].symbol != NT_FUNC_DEF ) // skip function declaration stmt
					printCodeSectionASTStmt(&(stmt->func_def.stmts[i]));
			}
			fprintf(fd,"\tmov rsp, rbp\n");
			fprintf(fd,"\tpop rbp\n");
			fprintf(fd,"\tret\n");

			for ( int i = 0 ; i < stmt->func_def.numstmts; i ++ ){
				if ( stmt->func_def.stmts[i].symbol == NT_FUNC_DEF ) // now run on function declaration stmt
					printCodeSectionASTStmt(&(stmt->func_def.stmts[i]));
			}

			break;
		case NT_DECL_STMT:
			// Nothing is needed here ...
			break;
		case NT_COND_STMT:
			// Handle the conditions ...
			// Note: Conditional stmts uses the rax and rbx registers for comparison...
			printCodeSectionASTNode(stmt->cond_stmt.condition);
			// Print the if label ...
			fprintf(fd,"%s:\n",stmt->cond_stmt.iflabel);
			// Print the if stmts ..
			for ( int i = 0; i <  stmt->cond_stmt.numifstmts; i ++){
				printCodeSectionASTStmt(&(stmt->cond_stmt.ifstmts[i]));
			}
			// After if jmp the else stmts
			fprintf(fd,"\tjmp %s\n", stmt->cond_stmt.nextlabel); // Finish if go to next stmt ...
			// The else stmts...
			fprintf(fd,"%s:\n", stmt->cond_stmt.elselabel);
			for ( int i = 0; i <  stmt->cond_stmt.numelsestmts; i ++){
				printCodeSectionASTStmt(&(stmt->cond_stmt.elsestmts[i]));
			}
			// The following stmts label...
			fprintf(fd,"%s:\n", stmt->cond_stmt.nextlabel);
			break;
		case T_READ:
			// Use scanf call ..
			// Accordingly ...
			if ( stmt->io_stmt.var->id.record->variable.type == T_INT ){
				// Incase of int
				fprintf(fd,"\tmov rdi, scannumberformat\n");
				fprintf(fd,"\tmov rsi, rsp\n\tadd rsi, %d\n", stmt->io_stmt.var->id.record->variable.offset);
				fprintf(fd,"\txor rax, rax\n");
				fprintf(fd,"\tcall scanf\n");
			}
			else {
				// Scanf float
				//printf("COMPLETE IT..\n");
				// printf("\tcall scanf\n");
			}
			break;
		case T_PRINT:
			if ( stmt->io_stmt.var->id.record->variable.type == T_INT ){
				// Incase of int
				fprintf(fd,"\tmov rdi, numberformat\n");
				fprintf(fd,"\txor rsi, rsi\n");
				fprintf(fd,"\tmov esi, [rsp+%d]\n", stmt->io_stmt.var->id.record->variable.offset );
				fprintf(fd,"\txor rax, rax\n");
				fprintf(fd,"\tcall printf\n");
			}
			else if ( stmt->io_stmt.var->id.record->variable.type == T_REAL ){
				// Incase of int
				//printf("COMPLETE IT..\n");
				//printf("\tcall printf\n");
			}
			else if ( stmt->io_stmt.var->id.record->variable.type == T_STRING ){
				// Incase of int
				fprintf(fd,"\tmov rdi, stringformat\n");
				fprintf(fd,"\tmov rsi, rsp\n\tadd rsi, %d\n", stmt->io_stmt.var->id.record->variable.offset );
				fprintf(fd,"\txor rax, rax\n");
				fprintf(fd,"\tcall printf\n");
			}
			else {
				int numcols = stmt->io_stmt.var->id.record->variable.numcols;
				// Incase of Matrix ..
				for ( int row = 0; row < stmt->io_stmt.var->id.record->variable.numrows; row ++ ){
					for ( int col = 0; col < stmt->io_stmt.var->id.record->variable.numcols ; col ++ ){
						if ( col == stmt->io_stmt.var->id.record->variable.numcols - 1) 
							fprintf(fd,"\tmov rdi, numberformat\n");
						else 
							fprintf(fd,"\tmov rdi, matrixformat\n");
						fprintf(fd,"\txor rsi, rsi\n");
						fprintf(fd,"\tmov esi, [rsp+%d]\n", stmt->io_stmt.var->id.record->variable.offset + ( row * numcols + col ) * sizeof(int) );
						fprintf(fd,"\txor rax, rax\n");
						fprintf(fd,"\tcall printf\n");
					}
				}
			}
			
			break;
		case NT_FUNC_CALL:
			// return variables are not used ...
			// printf("IMPLEMENT FUNCTION CALL\n");
			// for ( int i = 0 ; i <  stmt->func_call.numinargs ; i ++ ){
			// 	printCodeSectionASTNode(&(stmt->func_call.inargs[i]));
			// }
			break;
		case NT_EXPR1:
		case NT_EXPR2:
			switch ( stmt->assign_stmt.rhs.symbol){
				case T_SIZE:
					switch( stmt->assign_stmt.lhs.symbol ) {
						case T_ID:
							// Incase of string ...
							fprintf(fd,"\tmov rsi, rsp\n\tadd rsi, %d\n", stmt->assign_stmt.rhs.var->id.record->variable.offset);
							fprintf(fd,"\tcall __strlen\n");
							fprintf(fd,"\tmov [rsp+%d], eax\n", stmt->assign_stmt.lhs.var->id.record->variable.offset);
							break;
						case NT_VARLIST:
							// Incase of matrix ...
							// sizes are known during compile time..
							fprintf(fd,"\tmov eax, %d\n", stmt->assign_stmt.rhs.var->id.record->variable.numrows);
							fprintf(fd,"\tmov [rsp+%d], eax\n", stmt->assign_stmt.lhs.varlist[0].id.record->variable.offset);
							fprintf(fd,"\tmov eax, %d\n", stmt->assign_stmt.rhs.var->id.record->variable.numcols);
							fprintf(fd,"\tmov [rsp+%d], eax\n", stmt->assign_stmt.lhs.varlist[1].id.record->variable.offset);
							break;
						default:
							printf("%d %s ERROR: Unexpected in size.\n", __LINE__,__FILE__);
							exit(EXIT_FAILURE);
					}
					break;
				
				case NT_FUNC_CALL:
					// Add the label for function call ..
					//stmt->assign_stmt.rhs.func_call->func_call.funlabel = stmt->assign_stmt.rhs.func_call->func_call.record->function.function->func_def.funlabel;
					// return variables are used ..
					// printf("IMPLEMENT FUNCTION CALL..\n");
					break;
				case T_ID: // Is this required ...
				case NT_ARITH_EXPR:
					// ROOT: FALSE Because no new node is needed ...
					// Use the existing variable ... 
					switch ( stmt->assign_stmt.lhs.var->id.record->variable.type ){
						case T_INT:
							// printf("INT %d %s\n",__LINE__, TerminalSymbols[INDEX_TERMINAL( stmt->assign_stmt.rhs.var->symbol)]);
							// IF expression is of type int ...
							// Handle the assignment stmt ...
							printCodeSectionASTNode(stmt->assign_stmt.rhs.var);
							// Now for final assignment ...
							// Once every this is computer ...
							// Either an operand, constant or id...
							switch ( stmt->assign_stmt.rhs.var->symbol ){
								case T_NUM:
									fprintf(fd,"\tmov eax, %d\n",stmt->assign_stmt.rhs.var->constant._int);
									fprintf(fd,"\tmov [rsp+%d], eax\n", stmt->assign_stmt.lhs.var->id.record->variable.offset);
									break;
								case NT_MATRIX_ELEMENT:
								{
									int row = stmt->assign_stmt.rhs.var->matrix_element.indices[0];
									int col = stmt->assign_stmt.rhs.var->matrix_element.indices[1];
									int numcols = stmt->assign_stmt.rhs.var->matrix_element.record->variable.numcols;
									fprintf(fd,"\tmov eax, [rsp+%d]\n", stmt->assign_stmt.rhs.var->matrix_element.record->variable.offset + ( row * numcols + col ) * sizeof(int)  );
									fprintf(fd,"\tmov [rsp+%d], eax\n", stmt->assign_stmt.lhs.var->id.record->variable.offset);
								}								
									break;
								case T_ID:
									fprintf(fd,"\tmov eax, [rsp+%d]\n",stmt->assign_stmt.rhs.var->id.record->variable.offset);
									fprintf(fd,"\tmov [rsp+%d], eax\n", stmt->assign_stmt.lhs.var->id.record->variable.offset);
									break;
								default:
									// fprintf(fd,"%s\n",TerminalSymbols[INDEX_TERMINAL(stmt->assign_stmt.rhs.var->symbol)] );
									fprintf(fd,"\tmov eax, [rsp+%d]\n",stmt->assign_stmt.rhs.var->operator.record->variable.offset);
									fprintf(fd,"\tmov [rsp+%d], eax\n", stmt->assign_stmt.lhs.var->id.record->variable.offset);
									break;
							}
							break;
						case T_MATRIX:
							printCodeSectionASTNode(stmt->assign_stmt.rhs.var);
							for ( int row = 0 ; row < stmt->assign_stmt.lhs.var->id.record->variable.numrows ; row ++ ){
								for ( int col = 0 ; col < stmt->assign_stmt.lhs.var->id.record->variable.numcols ; col ++ ){
									int numcols = stmt->assign_stmt.lhs.var->id.record->variable.numcols;
									switch ( stmt->assign_stmt.rhs.var->symbol ){
										case NT_MATRIX:
											fprintf(fd,"\tmov eax, %d\n",stmt->assign_stmt.rhs.var->matrix.matrix[row][col]);
											fprintf(fd,"\tmov [rsp+%d], eax\n", stmt->assign_stmt.lhs.var->id.record->variable.offset + ( row * numcols + col ) * sizeof(int) );
											break;
										case T_ID:
											fprintf(fd,"\tmov eax, [rsp+%d]\n",stmt->assign_stmt.rhs.var->id.record->variable.offset + ( row * numcols + col ) * sizeof(int) );
											fprintf(fd,"\tmov [rsp+%d], eax\n", stmt->assign_stmt.lhs.var->id.record->variable.offset + ( row * numcols + col ) * sizeof(int) );
											break;
										default:
											fprintf(fd,"\tmov eax, [rsp+%d]\n",stmt->assign_stmt.rhs.var->operator.record->variable.offset + ( row * numcols + col ) * sizeof(int));
											fprintf(fd,"\tmov [rsp+%d], eax\n", stmt->assign_stmt.lhs.var->id.record->variable.offset + ( row * numcols + col ) * sizeof(int));
											break;
									}
								}
							}
							break;
						case T_REAL:
							break;
						case T_STRING:
							printCodeSectionASTNode(stmt->assign_stmt.rhs.var);
							if ( stmt->assign_stmt.rhs.var->symbol == T_ID ) {
								// Use the offsets ...

								fprintf(fd,"\tmov rsi, rsp\n\tadd rsi, %d\n", stmt->assign_stmt.rhs.var->id.record->variable.offset );
							}
							else if ( stmt->assign_stmt.rhs.var->symbol == T_STR ) {
								// If constant ...
								fprintf(fd,"\tmov rsi, %s\n", stmt->assign_stmt.rhs.var->constant.label );
							}
							else {
								fprintf(fd,"\tmov rsi, rsp\n\tadd rsi, %d\n", stmt->assign_stmt.rhs.var->operator.record->variable.offset );
							}
							fprintf(fd,"\tmov rdi, %d\n\tadd rdi, rsp\n", stmt->assign_stmt.lhs.var->id.record->variable.offset );
							fprintf(fd,"\tcall __strcpy\n");
							break;
					}
					break;
	
			}
	}
}
void generateNASMDataSectionCode(ASTPStmt mainstmt){
	constnum = 0;
	fprintf(fd,"section .data\n");
	fprintf(fd,"stringformat DB \"%%s\",10,0\n");
	fprintf(fd,"numberformat DB \"%%d\",10,0\n");
	fprintf(fd,"matrixformat DB \"%%d \",0\n");
	fprintf(fd,"scannumberformat DB \"%%d\",0\n");
	
	printDataSectionASTStmt(mainstmt);
}
void generateNASMTextSectionCode(ASTPStmt mainstmt){
	fprintf(fd,"section .text\n");
	fprintf(fd,"\tglobal main\n");
	fprintf(fd,"\textern printf\n");
	fprintf(fd,"\textern scanf\n");
	fprintf(fd,"main:\n");
	// printf("\tglobal _start\n");
	// printf("_start:\n");
	printCodeSectionASTStmt(mainstmt);
	// Print String functions ...
	FILE *fp = fopen("strfunctions.asm","r");
	if ( fp == NULL ){
		perror("Unable to read 'strfunctions.asm'");
		exit(EXIT_FAILURE);
	}
	char buffer[BUFSIZ+1];
	int numread ;
	while ( (numread = fread(buffer, sizeof(char), BUFSIZ, fp) )> 0 )
		buffer[numread] = '\0', fprintf(fd,"%s",buffer);
	fclose(fp);
}
// This function generates Assembly Code after 
// Intermediate variables and labels added...
void generateNASMCode( ASTPStmt mainstmt, FILE *outfile){
	// printf("; nasm -felf64 hello.asm && ld hello.o && ./a.out\n");
	//fd = fopen("test.asm","w");
	fd = outfile;
	//fd = stdout ;
	generateNASMDataSectionCode(mainstmt);
	generateNASMTextSectionCode(mainstmt);
}