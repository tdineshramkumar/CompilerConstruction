/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 2
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
// This does a preorder traversal ...
void printASTNode(ASTPNode node);
void printASTStmt(ASTPStmt stmt, int indent){
	if (stmt == NULL ) return;
	//printf("%*s", 4*indent, "");
	printf("[%d]\t%*s", stmt->linenumber, 4*indent, "");
	// printf("[line: %d]", stmt->linenumber);
	switch( stmt->symbol ){
		case NT_PROGRAM:
		//	printf(" MAIN:[%d]\n",stmt->func_def.numstmts);
			printf("\033[034m MAIN\033[0m\n");
			for ( int i = 0 ; i < stmt->func_def.numstmts; i ++ ){
				printASTStmt(&(stmt->func_def.stmts[i]), indent+1);
			}
			break;
		case NT_FUNC_DEF:
			// printf(" FUNCTION:%s", stmt->func_def._funid);
			printf(" %s", stmt->func_def._funid);
			// printf(" IN:%d", stmt->func_def.numinparams);
			printf(" \033[032m[\033[0m");
			for ( int i = 0 ; i <  stmt->func_def.numinparams ; i ++ ){
				//printf(" TYPE: %s", TerminalStrings[INDEX_TERMINAL(stmt->func_def.intypes[i])] );
				printf(" %s", TerminalStrings[INDEX_TERMINAL(stmt->func_def.intypes[i])] );
				printASTNode(&(stmt->func_def.inparams[i]));
			}
			printf(" \033[032m] ==> [ \033[0m");
			// printf(" OUT:%d", stmt->func_def.numoutparams);
			for ( int i = 0 ; i <  stmt->func_def.numoutparams ; i ++ ){
			//	printf(" TYPE: %s", TerminalStrings[INDEX_TERMINAL(stmt->func_def.outtypes[i])] );
				printf(" \033[33m%s\033[0m", TerminalStrings[INDEX_TERMINAL(stmt->func_def.outtypes[i])] );
				printASTNode(&(stmt->func_def.outparams[i]));
			}
			printf(" \033[032m]\033[0m\n");
			// printf("[%d]\n",stmt->func_def.numstmts);
			for ( int i = 0 ; i < stmt->func_def.numstmts; i ++ ){
				printASTStmt(&(stmt->func_def.stmts[i]), indent+1);
			}
			break;
		case NT_DECL_STMT:
			// printf(" DECLSTMT TYPE:%s [%d]", TerminalStrings[INDEX_TERMINAL(stmt->decl_stmt.type)], stmt->decl_stmt.numvars);
			printf(" \033[33m%s\033[0m", TerminalStrings[INDEX_TERMINAL(stmt->decl_stmt.type)]);
			for ( int i = 0 ; i < stmt->decl_stmt.numvars; i ++ )
				printASTNode( &(stmt->decl_stmt.vars[i]) );
			printf("\n");
			break;
		case NT_COND_STMT:
			printf("\033[034m IF\033[0m");
			printASTNode(stmt->cond_stmt.condition);
			// printf(" [%d]\n", stmt->cond_stmt.numifstmts);
			printf("\n");
			for ( int i = 0; i <  stmt->cond_stmt.numifstmts; i ++){
				printASTStmt(&(stmt->cond_stmt.ifstmts[i]), indent+1);
			}
			if (  stmt->cond_stmt.numelsestmts > 0 ){
				// To account for line number ..
				// printf("%*s", 4*indent, "");
				printf("[*]\t%*s", 4*indent, "");
				printf("\033[034m ELSE\033[0m\n");
				for ( int i = 0; i <  stmt->cond_stmt.numelsestmts; i ++){
					printASTStmt(&(stmt->cond_stmt.elsestmts[i]),indent+1);
				}
			}
			break;
		case T_READ:
		case T_PRINT:
			printf("\033[034m %s\033[0m", TerminalStrings[INDEX_TERMINAL(stmt->symbol)] );
			printASTNode(stmt->io_stmt.var);
			printf("\n");
			break;
		case NT_FUNC_CALL:
			printf(" %s", stmt->func_call._funid);
			// printf(" FUNCCALL:%s", stmt->func_call._funid);
			// printf(" IN:%d", stmt->func_call.numinargs);
			printf("\033[032m [\033[0m");
			for ( int i = 0 ; i <  stmt->func_call.numinargs ; i ++ ){
				printASTNode(&(stmt->func_call.inargs[i]));
			}
			printf("\033[032m ]\033[0m\n");
			// printf("\n");
			break;
		case NT_EXPR1:
		case NT_EXPR2:
			// printf(" EXPR: LHS");
			switch( stmt->assign_stmt.lhs.symbol ) {
				case T_ID:
					printASTNode(stmt->assign_stmt.lhs.var);
					break;
				case NT_VARLIST:
				//	printf("[%d]", stmt->assign_stmt.lhs.numvars);
					for ( int i = 0 ; i < stmt->assign_stmt.lhs.numvars; i ++ ){
						printASTNode(&(stmt->assign_stmt.lhs.varlist[i]));
					}
					break;
			}
			// printf(" RHS:");
			printf("\033[032m <==\033[0m");
			switch ( stmt->assign_stmt.rhs.symbol){
				case T_SIZE:
					printf("\033[034m SIZE\033[0m");
				case T_ID:
				case NT_ARITH_EXPR:
					printASTNode(stmt->assign_stmt.rhs.var);
					printf("\n");
					break;
				case NT_FUNC_CALL:

					printf(" %s", stmt->assign_stmt.rhs.func_call->func_call._funid);
					// printf(" FUNCCALL:%s", stmt->func_call._funid);
					// printf(" IN:%d", stmt->func_call.numinargs);
					printf("\033[032m [\033[0m");
					for ( int i = 0 ; i <  stmt->assign_stmt.rhs.func_call->func_call.numinargs ; i ++ ){
						printASTNode(&(stmt->assign_stmt.rhs.func_call->func_call.inargs[i]));
					}
					printf("\033[032m ]\033[0m\n");
					// printASTStmt(stmt->assign_stmt.rhs.func_call, 0);
					break;
			}
			break;
	}
}

void printASTNode(ASTPNode node){
	if ( node == NULL ) return;
	switch ( node->symbol ){
		case NT_MATRIX:
			//printf(" MATRIX:[");
			printf("\033[032m [\033[0m");
			for ( int i = 0 ; i < node->matrix.numrows; i ++ ){
				for ( int j = 0 ;j < node->matrix.numcols; j ++) {
					printf(" %d", node->matrix.matrix[i][j]);
				}
				if ( i != node->matrix.numrows - 1) printf("\033[032m ;\033[0m");
			}
			printf("\033[032m ]\033[0m");
			break;
		case T_STR:	printf(" \033[037m%s\033[0m", node->constant._str); break;
		case T_NUM:	printf(" \033[037m%d\033[0m", node->constant._int); break;
		case T_RNUM: printf(" \033[037m%f\033[0m", node->constant._real); break;
		case T_ID:	printf(" \033[035m%s\033[0m", node->id._id);	break;
		case NT_MATRIX_ELEMENT:	printf(" %s\033[032m[\033[0m%d,%d\033[032m]\033[0m", node->matrix_element._id,node->matrix_element.indices[0],node->matrix_element.indices[1]); break;
		// case T_STR:	printf(" STR:%s", node->constant._str); break;
		// case T_NUM:	printf(" NUM:%d", node->constant._int); break;
		// case T_RNUM: printf(" RNUM:%f", node->constant._real); break;
		// case T_ID:	printf(" ID:%s", node->id._id);	break;
		// case NT_MATRIX_ELEMENT:	printf(" ME:%s[%d,%d]\n", node->matrix_element._id,node->matrix_element.indices[0],node->matrix_element.indices[1]); break;
		default:
		// In case of logical or arithmatic or relational operator...
			// printf(" OP:%s(", TerminalStrings[INDEX_TERMINAL(node->symbol)]);
			printf(" \033[036m%s\033[032m(\033[0m", TerminalSymbols[INDEX_TERMINAL(node->symbol)]);
			if ( node->operator.left != NULL )
				printASTNode(node->operator.left);
			if ( node->operator.right != NULL) 
				printASTNode(node->operator.right);
			printf(" \033[032m)\033[0m");
	}
}

void printAST(ASTPStmt mainprogram){
	printf("\033[47m\033[30mPrinting AST using PRE-ORDER TRAVERSAL.\033[0m\n");
	printf("\033[47m\033[30mSymbols like ==> <== ( ) [ ] ; are not part of AST and used for representational purposes only.\033[0m\n");
	printASTStmt(mainprogram, 0);
}

int countASTNode(ASTPNode node);
int countASTStmt(ASTPStmt stmt){
	int count = 0;
	if (stmt == NULL ) return count;
	count ++; // increment count
	switch( stmt->symbol ){
		case NT_PROGRAM:
			for ( int i = 0 ; i < stmt->func_def.numstmts; i ++ ){
				count += countASTStmt(&(stmt->func_def.stmts[i]));
			}
			break;
		case NT_FUNC_DEF:
			for ( int i = 0 ; i <  stmt->func_def.numinparams ; i ++ ){
				count += countASTNode(&(stmt->func_def.inparams[i]));
			}
			for ( int i = 0 ; i <  stmt->func_def.numoutparams ; i ++ ){
				count += countASTNode(&(stmt->func_def.outparams[i]));
			}
			for ( int i = 0 ; i < stmt->func_def.numstmts; i ++ ){
				count += countASTStmt(&(stmt->func_def.stmts[i]));
			}
			break;
		case NT_DECL_STMT:
			for ( int i = 0 ; i < stmt->decl_stmt.numvars; i ++ )
				count  += countASTNode( &(stmt->decl_stmt.vars[i]) );
			break;
		case NT_COND_STMT:
			count += countASTNode(stmt->cond_stmt.condition);
			for ( int i = 0; i <  stmt->cond_stmt.numifstmts; i ++){
				count += countASTStmt(&(stmt->cond_stmt.ifstmts[i]));
			}
			if (  stmt->cond_stmt.numelsestmts > 0 ){
				for ( int i = 0; i <  stmt->cond_stmt.numelsestmts; i ++){
					count += countASTStmt(&(stmt->cond_stmt.elsestmts[i]));
				}
			}
			break;
		case T_READ:
		case T_PRINT:
			count += countASTNode(stmt->io_stmt.var);
			break;
		case NT_FUNC_CALL:
			for ( int i = 0 ; i <  stmt->func_call.numinargs ; i ++ ){
				count += countASTNode(&(stmt->func_call.inargs[i]));
			}
			break;
		case NT_EXPR1:
		case NT_EXPR2:
			// printf(" EXPR: LHS");
			switch( stmt->assign_stmt.lhs.symbol ) {
				case T_ID:
					count += countASTNode(stmt->assign_stmt.lhs.var);
					break;
				case NT_VARLIST:
					for ( int i = 0 ; i < stmt->assign_stmt.lhs.numvars; i ++ ){
						count += countASTNode(&(stmt->assign_stmt.lhs.varlist[i]));
					}
					break;
			}
			switch ( stmt->assign_stmt.rhs.symbol){
				case T_SIZE:
				case T_ID:
				case NT_ARITH_EXPR:
					count += countASTNode(stmt->assign_stmt.rhs.var);
					break;
				case NT_FUNC_CALL:
					for ( int i = 0 ; i <  stmt->assign_stmt.rhs.func_call->func_call.numinargs ; i ++ ){
						count += countASTNode(&(stmt->assign_stmt.rhs.func_call->func_call.inargs[i]));
					}
					break;
			}
			break;
	}
	return count;
}

int countASTNode(ASTPNode node){
	int count = 0;
	if ( node == NULL ) 
		return count;
	count ++;
	switch ( node->symbol ){
		case NT_MATRIX:
		case T_STR:
		case T_NUM:
		case T_RNUM:
		case T_ID:	
		case NT_MATRIX_ELEMENT:
			return count;
		default:
			if ( node->operator.left != NULL )
				count += countASTNode(node->operator.left);
			if ( node->operator.right != NULL) 
				count += countASTNode(node->operator.right);
	}
	return count;
}

void createASTnode( PTPNode node, void * astnode);
/// Note: this is done outside to reduce the size of createAST node function ...
void createASTConditionNode(PTPNode node, ASTPNode condition){
	if ( node == NULL ) {
		printf("Null node passed to createASTConditionNode function..\n");
		return;
	}
	switch ( node->symbol ){
		case NT_CONDITION:{
			switch( node->nonterminal.child->symbol ){
				case NT_BOOL_EXPR: {
					// if boolean expression then call it again with this node ..
					createASTConditionNode(node->nonterminal.child, condition);
					// printf("Bool expr..\n");
					break;
				}
				case T_NOT: {
					PTPNode notnode = node->nonterminal.child;
					condition->linenumber = notnode->linenumber;
					condition->symbol = T_NOT; // set the node symbol itself
					condition->operator.right= NULL;

					// How to know the other operand .. ?
					PTPNode cnode = notnode->next->next ; // condition pt node 
					condition->operator.left = malloc(sizeof(ASTNode));
					// printf("not operator...\n");
					createASTConditionNode(cnode, condition->operator.left);
					break;
				}
				case T_OP:{
					PTPNode cnode1 = node->nonterminal.child->next;
					PTPNode boolop = cnode1->next->next;
					PTPNode cnode2 = boolop->next->next; // get the next condition as well
					// printf("%s operator...\n", TerminalStrings[INDEX_TERMINAL(boolop->nonterminal.child->symbol)]);
					condition->linenumber = boolop->nonterminal.child->linenumber;
					condition->symbol = boolop->nonterminal.child->symbol; // get the logical operator ..
					condition->operator.left = malloc(sizeof(ASTNode));
					createASTConditionNode(cnode1, condition->operator.left);
					
					condition->operator.right = malloc(sizeof(ASTNode));
					createASTConditionNode(cnode2, condition->operator.right);
					break;
				}
			}
			break;
		}
		case NT_BOOL_EXPR:{

			PTPNode booloperand1 = node->nonterminal.child;
			PTPNode relop = booloperand1->next;
			PTPNode booloperand2 = relop->next;
			condition->linenumber = relop->nonterminal.child->linenumber;
			condition->symbol = relop->nonterminal.child->symbol;
			// printf("REL OP: %s ...\n", TerminalStrings[INDEX_TERMINAL(condition->symbol)] );
			condition->operator.left = malloc(sizeof(ASTNode));
			createASTnode(booloperand1->nonterminal.child, condition->operator.left);
			condition->operator.right = malloc(sizeof(ASTNode));
			createASTnode(booloperand2->nonterminal.child, condition->operator.right);
			break;
		}
	}
}

ASTPNode createASTArithnode(PTPNode arithexpr){
	switch ( arithexpr->symbol ){
		case NT_ARITH_EXPR: {
			// printf("ASTArithNode: Inside ARITH_EXPR\n");
			PTPNode prod_term = arithexpr->nonterminal.child;
			PTPNode more_in_arith_expr = prod_term->next;
			ASTPNode prod = createASTArithnode(prod_term);
			while ( more_in_arith_expr->nonterminal.child != NULL ){
				// Update the values ...
				PTPNode opnode = more_in_arith_expr->nonterminal.child;
				PTPNode prod_term2 = opnode->next;
				more_in_arith_expr = prod_term2->next;

				// Make the new structure ...
				ASTPNode op = malloc(sizeof(ASTNode));
				op->linenumber = opnode->linenumber;
				op->symbol = opnode->symbol; // put the operator ...
				// printf("OPERATOR: %s\n", TerminalTokens[INDEX_TERMINAL(op->symbol)] );
				op->operator.left = prod ; // set the existing one to left ...
				op->operator.right = createASTArithnode(prod_term2);
				prod = op; // Now for next iteration set it as left node ...
			}
			return prod;
			break; // Not needed ...
		}
		case NT_PROD_TERM: {
			// printf("ASTArithNode: Inside PROD_TERM\n");
			PTPNode rvalue = arithexpr->nonterminal.child;
			PTPNode more_in_prod =  rvalue->next;
			ASTPNode rval = createASTArithnode(rvalue);
			while ( more_in_prod->nonterminal.child != NULL ){
				// Update the values...
				PTPNode opnode = more_in_prod->nonterminal.child;
				PTPNode rvalue2 = opnode->next;
				more_in_prod = rvalue2->next;

				// Make the new structure ...
				ASTPNode op = malloc(sizeof(ASTNode));
				op->linenumber = opnode->linenumber;
				op->symbol = opnode->symbol; // put the operator ...
				// printf("OPERATOR: %s\n", TerminalTokens[INDEX_TERMINAL(op->symbol)] );
				op->operator.left = rval ; // set the existing one to left ...
				op->operator.right = createASTArithnode(rvalue2);
				rval = op; // Now for next iteration set it as left node ...

			}
			return rval;
			break;
		}
		case NT_RVALUE: {
			// printf("ASTArithNode: Inside RVALUE\n");
			if ( arithexpr->nonterminal.child->symbol == T_OP ){
				PTPNode newarithexpr = arithexpr->nonterminal.child->next ; // go to the next arithmatric expression
				return createASTArithnode(newarithexpr); // loop back ...
			}
			else {
				// Else some constant or variable ...
				ASTPNode newnode = malloc(sizeof(ASTNode));
				createASTnode(arithexpr->nonterminal.child, newnode);
				return newnode;
			}
			break;
		}
	}
	

}

// This function creates an AST from Parse tree..
// this function also takes an input as some ast node 
// type casts accordingly..
// WARNING: Later verify line numbers ...
void createASTnode( PTPNode node, void * astnode){
	if ( node == NULL ) {
		printf("Null node passed to createASTnode function..\n");
		return;
	}
	switch( node->symbol ){
		case NT_PROGRAM:{
			PTPNode mainnode =  node->nonterminal.child;
			ASTPStmt program = (ASTPStmt) astnode;
			program->linenumber = mainnode->linenumber; // get the line number from main node...
			program->symbol = NT_PROGRAM;
			program->func_def._funid = mainnode->terminal.lexeme._id; // since we know main function ...
			// printf("MAIN NODE: LEXEME %s\n", mainnode->terminal.lexeme._id);
			program->func_def.numinparams = 0;
			program->func_def.intypes = NULL;
			program->func_def.inparams= NULL;
			program->func_def.numoutparams = 0;
			program->func_def.outtypes = NULL;
			program->func_def.outparams = NULL;
			int numstmts = 0;
			PTPNode stmt = mainnode->next->next->next; // skip [,]
			PTPNode stmts = stmt->next ; // go to stmts node ..
			numstmts ++; // to include the first stmt ..
			while ( stmts->nonterminal.child != NULL ){
				stmt = stmts->nonterminal.child;
				stmts = stmt->next;
				numstmts ++;
			}
			program->func_def.numstmts  = numstmts;
			program->func_def.stmts  = malloc(sizeof(ASTStmt) * numstmts);
			// printf("Number of stmts: %d\n", numstmts);

			// Now call the function on each stmt ...
			int stmtindex = 0;
			stmt = mainnode->next->next->next;
			stmts = stmt->next;
			createASTnode(stmt->nonterminal.child, &(program->func_def.stmts[stmtindex++]));
			while ( stmts->nonterminal.child != NULL ){
				stmt = stmts->nonterminal.child;
				stmts = stmt->next;
				createASTnode(stmt->nonterminal.child, &(program->func_def.stmts[stmtindex++]));
			}
			break;
		}
		case NT_FUNC_DEF:{
			// Note: In and out params have been interchanged ...
			PTPNode funcnode = node->nonterminal.child;
			ASTPStmt funcdef = (ASTPStmt) astnode;
			funcdef->linenumber = funcnode->linenumber;
			funcdef->symbol = NT_FUNC_DEF;
			


			///// OUTPARAMS
			int numparams = 0;
			PTPNode paramsnode = funcnode->next->next; // skip [
			// Now find the count of in params ...
			PTPNode typenode = paramsnode->nonterminal.child;
			PTPNode idnode = typenode->next;
			PTPNode moreparams = idnode->next;
			numparams++;
			while ( moreparams->nonterminal.child != NULL) {
				typenode = moreparams->nonterminal.child->next; // skip ,
				idnode = typenode->next;
				moreparams = idnode->next;
				numparams++;
			}
			// printf("Number of in params: %d\n",numparams);
			// Allocate out params..
			funcdef->func_def.numoutparams = numparams;
			funcdef->func_def.outtypes = malloc(sizeof(SYMBOL)*numparams);
			funcdef->func_def.outparams = malloc(sizeof(ASTNode)*numparams);
			// Assign types and corresponding id...
			int curparams=0;
			typenode = paramsnode->nonterminal.child;
			// Check later if type is valid ...
			funcdef->func_def.outtypes[curparams] = typenode->nonterminal.child->symbol; // get the type .. 
			idnode = typenode->next;
			moreparams = idnode->next;
			createASTnode(idnode, &(funcdef->func_def.outparams[curparams++]));
			while ( moreparams->nonterminal.child != NULL ){
				typenode = moreparams->nonterminal.child->next;
				funcdef->func_def.outtypes[curparams] = typenode->nonterminal.child->symbol; // get the type .. 
				idnode = typenode->next;
				moreparams = idnode->next;
				createASTnode(idnode, &(funcdef->func_def.outparams[curparams++]));	
			}
			// for ( int i = 0 ; i < numparams ; i ++) 
			// 	printf("%d TYPE: %s\n",i, TerminalStrings[INDEX_TERMINAL(funcdef->func_def.outtypes[i])] );
			
			// define function name properly ..
			PTPNode funcidnode = paramsnode->next->next->next;
			funcdef->func_def._funid = funcidnode->terminal.lexeme._id;
			// printf("FUNCID: %s\n",funcdef->func_def._funid);

			paramsnode = funcidnode->next->next; // skip [
			numparams = 0; 


			/////////////// IN PARAMS ....
			// Now find the count of out params ...
			typenode = paramsnode->nonterminal.child;
			idnode = typenode->next;
			moreparams = idnode->next;
			numparams++;
			while ( moreparams->nonterminal.child != NULL) {
				typenode = moreparams->nonterminal.child->next; // skip ,
				idnode = typenode->next;
				moreparams = idnode->next;
				numparams++;
			}
			// printf("Number of out params: %d\n",numparams);
			// Allocate in params..
			funcdef->func_def.numinparams = numparams;
			funcdef->func_def.intypes = malloc(sizeof(SYMBOL)*numparams);
			funcdef->func_def.inparams = malloc(sizeof(ASTNode)*numparams);
			// Assign types and corresponding id...
			curparams=0;
			typenode = paramsnode->nonterminal.child;
			// Check later if type is valid ...
			funcdef->func_def.intypes[curparams] = typenode->nonterminal.child->symbol; // get the type .. 
			idnode = typenode->next;
			moreparams = idnode->next;
			createASTnode(idnode, &(funcdef->func_def.inparams[curparams++]));
			while ( moreparams->nonterminal.child != NULL ){
				typenode = moreparams->nonterminal.child->next;
				funcdef->func_def.intypes[curparams] = typenode->nonterminal.child->symbol; // get the type .. 
				idnode = typenode->next;
				moreparams = idnode->next;
				createASTnode(idnode, &(funcdef->func_def.inparams[curparams++]));	
			}
			// for ( int i = 0 ; i < numparams ; i ++) 
			 //	printf("%d OUT PARAM TYPE: %s\n",i, TerminalStrings[INDEX_TERMINAL(funcdef->func_def.intypes[i])] );
			




			//// STMTS ....
			// Now for function stmts ...
			 int numstmts = 0;
			PTPNode stmt = paramsnode->next->next; // skip [
			PTPNode stmts = stmt->next ; // go to stmts node ..
			numstmts ++; // to include the first stmt ..
			while ( stmts->nonterminal.child != NULL ){
				stmt = stmts->nonterminal.child;
				stmts = stmt->next;
				numstmts ++;
			}
			funcdef->func_def.numstmts  = numstmts;
			funcdef->func_def.stmts  = malloc(sizeof(ASTStmt) * numstmts);
			// printf("Number of stmts: %d\n", numstmts);

			// Now call the function on each stmt ...
			int stmtindex = 0;
			stmt = paramsnode->next->next;
			stmts = stmt->next;
			createASTnode(stmt->nonterminal.child, &(funcdef->func_def.stmts[stmtindex++]));
			while ( stmts->nonterminal.child != NULL ){
				stmt = stmts->nonterminal.child;
				stmts = stmt->next;
				createASTnode(stmt->nonterminal.child, &(funcdef->func_def.stmts[stmtindex++]));
			}
			break;
		}
		case NT_DECL_STMT:{
			// If called with declaration stmt...
			ASTPStmt declstmt = (ASTPStmt) astnode;
			declstmt->symbol = NT_DECL_STMT;
			PTPNode typenode = node->nonterminal.child; // get the type node ...
			declstmt->linenumber = typenode->linenumber;
			//printf("Decl Stmt line number: %d\n",declstmt->linenumber );
			declstmt->decl_stmt.type = typenode->nonterminal.child->symbol ;
			// compute number of nodes ...
			int numvars = 0; 
			PTPNode varlist = typenode->next;
			PTPNode idnode = varlist->nonterminal.child;
			PTPNode morevars = idnode->next;
			numvars ++;
			while ( morevars->nonterminal.child != NULL ) {
				idnode = morevars->nonterminal.child->next; //skip comma
				morevars = idnode->next;
				numvars ++;
			}
			// Now allocate the nodes
			declstmt->decl_stmt.numvars = numvars;
			declstmt->decl_stmt.vars = malloc(sizeof(ASTNode) * numvars);

			// Obtain the nodes ..
			int curvars = 0;
			idnode = varlist->nonterminal.child;	
			morevars = idnode->next;
			createASTnode(idnode, &(declstmt->decl_stmt.vars[curvars++]));
			while ( morevars->nonterminal.child != NULL ){
				idnode = morevars->nonterminal.child->next; //skip comma
				morevars = idnode->next;
				createASTnode(idnode, &(declstmt->decl_stmt.vars[curvars++]));
			}
			// printf("Number of Declaration variables of type: %s is %d \n", TerminalStrings[INDEX_TERMINAL(declstmt->decl_stmt.type)], numvars);
			break;
		}
		case NT_COND_STMT:{
			ASTPStmt condstmt = (ASTPStmt) astnode;
			PTPNode ifnode = node->nonterminal.child;
			condstmt->linenumber = ifnode->linenumber;
			condstmt->symbol = NT_COND_STMT;
			PTPNode condition = ifnode->next->next; 
			int numifstmts =0;
			PTPNode stmt = condition->next->next;
			PTPNode stmts = stmt->next;
			PTPNode elsestmt = stmts->next;

			numifstmts ++; // to include the first stmt ...
			// Now get the count of ifstmts...
			while ( stmts->nonterminal.child != NULL ){
				stmt = stmts->nonterminal.child;
				stmts = stmt->next;
				numifstmts ++;
			}
			// Now allocate and then get the stmts..
			condstmt->cond_stmt.numifstmts = numifstmts;
			condstmt->cond_stmt.ifstmts = malloc(sizeof(ASTStmt)*numifstmts);

			// Now obtain the stmts ...
			int ifstmtindex = 0;
			stmt = condition->next->next;
			stmts = stmt->next;
			createASTnode(stmt->nonterminal.child, &(condstmt->cond_stmt.ifstmts[ifstmtindex++]));
			while ( stmts->nonterminal.child != NULL ){
				stmt = stmts->nonterminal.child;
				stmts = stmt->next;
				createASTnode(stmt->nonterminal.child,&(condstmt->cond_stmt.ifstmts[ifstmtindex++]));
			}


			// Now obtain else stmts if any ..
			if ( elsestmt->nonterminal.child == NULL ){
				condstmt->cond_stmt.elsestmts = NULL;
				condstmt->cond_stmt.numelsestmts = 0;
			}
			else {
				// Add the else stmts ...
				int numelsestmts =0;
				stmt = elsestmt->nonterminal.child->next; // skip else
				stmts = stmt->next;
				numelsestmts++;
				while (  stmts->nonterminal.child != NULL ){
					stmt = stmts->nonterminal.child;
					stmts = stmt->next;
					numelsestmts ++;
				}
				// Now allocate..
				condstmt->cond_stmt.numelsestmts = numelsestmts;
				condstmt->cond_stmt.elsestmts = malloc(sizeof(ASTStmt)*numelsestmts);

				// Now obtain the stmts
				int elsestmtindex = 0;
				stmt = elsestmt->nonterminal.child->next; // skip else
				stmts = stmt->next;
				createASTnode(stmt->nonterminal.child, &(condstmt->cond_stmt.elsestmts[elsestmtindex++]));
				while ( stmts->nonterminal.child != NULL ){
					stmt = stmts->nonterminal.child;
					stmts = stmt->next;
					createASTnode(stmt->nonterminal.child,&(condstmt->cond_stmt.elsestmts[elsestmtindex++]));
				}
			}
			// printf("Number of if stmts: %d else stmts: %d\n", condstmt->cond_stmt.numifstmts, condstmt->cond_stmt.numelsestmts);
			// Finally the condition ...
			// But one this for sure there is an operator ...
			// unlike in arithmatic expression ...
			condstmt->cond_stmt.condition = malloc(sizeof(ASTNode));
			createASTConditionNode(condition, condstmt->cond_stmt.condition);
			break;
		}
		case NT_IO_STMT:{
			ASTPStmt iostmt = (ASTPStmt) astnode;
			PTPNode ionode = node->nonterminal.child;
			PTPNode idnode = ionode->next->next; // skip (
			iostmt->linenumber = ionode->linenumber;
			iostmt->symbol = ionode->symbol; // get the operation (read  or print)
			iostmt->io_stmt.var = malloc(sizeof(ASTNode));
			createASTnode(idnode, iostmt->io_stmt.var);
			// printf("IO STMT is %s\n", TerminalStrings[INDEX_TERMINAL(iostmt->symbol)]);
			break;
		}
		
		case NT_FUNC_CALL:{
			ASTPStmt func_call = (ASTPStmt) astnode;
			PTPNode funcidnode = node->nonterminal.child;
			func_call->func_call._funid = funcidnode->terminal.lexeme._id ;
			func_call->linenumber = funcidnode->linenumber;
			func_call->symbol = NT_FUNC_CALL;
			// Later change these values ... (<---------)
			func_call->func_call.numoutargs = 0;
			func_call->func_call.outargs = NULL;
			
			// Find Number of arguments...
			uint numinargs = 0;
			PTPNode argnode = funcidnode->next->next; // skip (
			PTPNode argsnode = argnode->next ;
			numinargs ++;
			while ( argsnode->nonterminal.child != NULL ) {
				argnode = argsnode->nonterminal.child->next; // skip ,
				argsnode = argnode->next;
				numinargs ++;
			}
			// Allocate it ..
			func_call->func_call.numinargs = numinargs;
			func_call->func_call.inargs = malloc(sizeof(ASTNode)*numinargs);
			// Obtain the nodes ...
			int curargs = 0;
			argnode = funcidnode->next->next;
			argsnode = argnode->next ;
			createASTnode(argnode->nonterminal.child, &(func_call->func_call.inargs[curargs++]) );
			while ( argsnode->nonterminal.child != NULL ) {
				argnode = argsnode->nonterminal.child->next; // skip ,
				argsnode = argnode->next;
				createASTnode(argnode->nonterminal.child, &(func_call->func_call.inargs[curargs++]) );
			}
			// printf("FUNC CALL %s IN PARAMS: %d\n",func_call->func_call._funid, func_call->func_call.numinargs);
			break;
		}
		case NT_ASSIGN_STMT:{
			ASTPStmt assignstmt = (ASTPStmt) astnode;
			// Check for type of expression ...
			if ( node->nonterminal.child->symbol == T_ID ){
				// The expr1
				PTPNode idnode = node->nonterminal.child ;
				PTPNode assign = idnode->next; 
				PTPNode expr1 = idnode->next->next; // skip =
				assignstmt->linenumber = assign->linenumber;
				assignstmt->symbol = expr1->symbol;
				assignstmt->assign_stmt.lhs.symbol = T_ID;
				assignstmt->assign_stmt.lhs.var = malloc(sizeof(ASTNode));
				createASTnode(idnode, assignstmt->assign_stmt.lhs.var);
				
				// Now check on RHS
				// Incase of size expression ..
				if ( expr1->nonterminal.child->symbol == T_SIZE ){
					PTPNode sizenode = expr1->nonterminal.child;
					PTPNode rhsid = sizenode->next; // get the rhs node ..
					assignstmt->assign_stmt.rhs.symbol = T_SIZE;
					assignstmt->assign_stmt.rhs.var = malloc(sizeof(ASTNode));
					createASTnode(rhsid, assignstmt->assign_stmt.rhs.var ); // Now assign the variable
				}
				// Now in case of func call
				else if (  expr1->nonterminal.child->symbol == NT_FUNC_CALL ){
					PTPNode func_call =  expr1->nonterminal.child;
					assignstmt->assign_stmt.rhs.symbol = NT_FUNC_CALL;
					assignstmt->assign_stmt.rhs.func_call = malloc(sizeof(ASTStmt));
					createASTnode( func_call, assignstmt->assign_stmt.rhs.func_call);
				}
				// Now need to process expression ...
				// <-------------------------- EXPR --------------------
				// First check if single operand (no expression to evaluate...)
				else if (  expr1->nonterminal.child->symbol == NT_ARITH_EXPR  ){
					
					PTPNode arithexpr =  expr1->nonterminal.child;
					assignstmt->assign_stmt.rhs.symbol = NT_ARITH_EXPR;
					assignstmt->assign_stmt.rhs.var = createASTArithnode(arithexpr);
					// printASTNode(assignstmt->assign_stmt.rhs.var);
				}
			}
			else {
				// THen expr2 ...
				PTPNode varlist = node->nonterminal.child->next; // skip [
				PTPNode assign = varlist->next->next;
				PTPNode expr2 = varlist->next->next->next; // skip ],=
				assignstmt->linenumber = assign->linenumber;
				assignstmt->symbol = expr2->symbol;
				assignstmt->assign_stmt.lhs.symbol = NT_VARLIST;

				///////////////////// 
					// Handle the varlist ...
					int numvars = 0;
					PTPNode idnode = varlist->nonterminal.child;
					PTPNode morevars = idnode->next;
					numvars ++;
					while ( morevars->nonterminal.child != NULL ) {
						idnode = morevars->nonterminal.child->next; //skip comma
						morevars = idnode->next;
						numvars ++;
					}
					// Now allocate the nodes ..
					assignstmt->assign_stmt.lhs.numvars = numvars;
					assignstmt->assign_stmt.lhs.varlist = malloc(sizeof(ASTNode)*numvars);
					// Now fetch the nodes ...
					int curvars = 0;
					idnode = varlist->nonterminal.child;	
					morevars = idnode->next;
					createASTnode(idnode, &(assignstmt->assign_stmt.lhs.varlist[curvars++]));
					while ( morevars->nonterminal.child != NULL ){
						idnode = morevars->nonterminal.child->next; //skip comma
						morevars = idnode->next;
						createASTnode(idnode, &(assignstmt->assign_stmt.lhs.varlist[curvars++]));
					}
					// printf("Number of variable in list (lhs) is %d \n", assignstmt->assign_stmt.lhs.numvars );
				//////////////////////

				// Now check on RHS
				// Incase of size expression ..
				if ( expr2->nonterminal.child->symbol == T_SIZE ){
					PTPNode sizenode = expr2->nonterminal.child;
					PTPNode rhsid = sizenode->next; // get the rhs node ..
					assignstmt->assign_stmt.rhs.symbol = T_SIZE;
					assignstmt->assign_stmt.rhs.var = malloc(sizeof(ASTNode));
					createASTnode(rhsid, assignstmt->assign_stmt.rhs.var ); // Now assign the variable
				}
				// Now in case of func call
				else if (  expr2->nonterminal.child->symbol == NT_FUNC_CALL ){
					PTPNode func_call =  expr2->nonterminal.child;
					assignstmt->assign_stmt.rhs.symbol = NT_FUNC_CALL;
					assignstmt->assign_stmt.rhs.func_call = malloc(sizeof(ASTStmt));
					createASTnode(func_call, assignstmt->assign_stmt.rhs.func_call);
				}
			}
			break;
		}
		case T_NUM:{
			ASTPNode intnode = (ASTPNode) astnode;
			intnode->linenumber = node->linenumber;
			intnode->symbol = T_NUM;
			intnode->constant._int = node->terminal.lexeme._int ;
			// printf("Create a new int node %d\n", intnode->constant._int);
			break;
		}
		case T_RNUM:{
			ASTPNode rnumnode = (ASTPNode) astnode;
			rnumnode->linenumber = node->linenumber;
			rnumnode->symbol = T_RNUM;
			rnumnode->constant._real = node->terminal.lexeme._real ;
			// printf("Create a new rnum node %f\n", rnumnode->constant._real);
			break;
		}
		case T_STR:{
			ASTPNode strnode = (ASTPNode) astnode;
			strnode->linenumber = node->linenumber;
			strnode->symbol = T_STR;
			strnode->constant._str = node->terminal.lexeme._str ;
			// printf("Create a new str node %s\n", strnode->constant._str);
			break;
		}
		/*
struct {
			uint numrows;
			uint numcols;
			uint ** matrix;
		} matrix; /
		*/
		case NT_MATRIX:{
			ASTPNode matrix = (ASTPNode) astnode;
			matrix->linenumber = node->linenumber;
			matrix->symbol = NT_MATRIX;

			PTPNode row, morerows, sqo = node->nonterminal.child, moreinrow, num ;
	
			// Now identify sizes of matrix ...
			// And check if there is any dimension mismatch ...
			row = sqo->next; morerows = row->next;
			moreinrow = row->nonterminal.child->next; // Jump across num ...
			uint numrows =1, numcols =0, tmpnumcols =1; // for first num ...
			while (  moreinrow->nonterminal.child != NULL  ) {
				tmpnumcols ++;
				moreinrow = moreinrow->nonterminal.child->next->next; // skip comma and num ...
			}
			numcols =tmpnumcols;
			while ( morerows->nonterminal.child != NULL ) {
				numrows ++;
				row = morerows->nonterminal.child->next; // ignore the semicolon
				moreinrow = row->nonterminal.child->next; // Jump across num ...
				tmpnumcols =1; // for first num ...
				while (  moreinrow->nonterminal.child != NULL  ) {
					tmpnumcols ++;
					moreinrow = moreinrow->nonterminal.child->next->next; // skip comma and num ...
				}
				if ( tmpnumcols != numcols ) {
					printf("\033[031m ERROR: [Line(%d)] SEMANTIC-ERROR: Column Counts Don't Match. In 1 Row Number of Colums: %d In %d Row Number of Columns: %d\033[0m\n",matrix->linenumber, numcols, numrows, tmpnumcols);
					if ( tmpnumcols > numcols ) numcols = tmpnumcols;
				}
				morerows = row->next; // shift to next more rows ...
			}
			
			// Now once dimensions obtained ... 
			// Allocate the matrix..
			matrix->matrix.numrows = numrows;
			matrix->matrix.numcols = numcols;
			matrix->matrix.matrix = malloc(sizeof(uint *)*numrows);
			for ( int i = 0 ; i < numrows; i ++ ){
				matrix->matrix.matrix[i] = malloc(sizeof(uint)*numcols); 
				for ( int j =0; j < numcols; j ++) {
					matrix->matrix.matrix[i][j] = 0;
				}
			}

			// Now Start filling the values ...
			uint currow=0, curcol=0;
			row = sqo->next; num = row->nonterminal.child;  morerows = row->next;
			matrix->matrix.matrix[currow][curcol++]= num->terminal.lexeme._int;
			moreinrow = num->next;
			while (  moreinrow->nonterminal.child != NULL  ) {
				num = moreinrow->nonterminal.child->next;
				matrix->matrix.matrix[currow][curcol++] = num->terminal.lexeme._int;
				moreinrow = num->next; // skip comma and num ...
			}
			currow++;
			while ( morerows->nonterminal.child != NULL ) {
				curcol = 0;
				row = morerows->nonterminal.child->next; // ignore the semicolon
				num = row->nonterminal.child;  moreinrow = num->next;
				matrix->matrix.matrix[currow][curcol++]= num->terminal.lexeme._int;
				moreinrow = row->nonterminal.child->next; // Jump across num ...
				while (  moreinrow->nonterminal.child != NULL  ) {
					num = moreinrow->nonterminal.child->next;
					matrix->matrix.matrix[currow][curcol++] = num->terminal.lexeme._int;
					moreinrow = num->next; // skip comma and num ...
				}
				morerows = row->next; // shift to next more rows ...
				currow++;
			}
			// printf("MATRIX: \n");
			// for ( int i = 0; i < numrows; i ++) {
			// 	for ( int j = 0 ;j < numcols; j ++) {
			// 		printf("%u ", matrix->matrix.matrix[i][j]);
			// 	}
			// 	printf("\n");
			// }
			break;
		}
		case T_ID: {
			// Needs Modification ... <------------------------
			// If called with id ...
			if ( node->next == NULL ||  node->next->symbol != NT_MATRIX_ELEMENT || ( node->next->symbol == NT_MATRIX_ELEMENT && node->next->nonterminal.child == NULL ) ) {
				// If no matrix element ...
				ASTPNode idnode = (ASTPNode) astnode;
				idnode->linenumber = node->linenumber;
				idnode->symbol = T_ID;
				idnode->id._id = node->terminal.lexeme._id;
				// printf("Created new ID node: %s\n", idnode->id._id );
			}
			else {
				// If matrix element ...
				ASTPNode menode = (ASTPNode) astnode;
				menode->linenumber = node->linenumber;
				menode->symbol = NT_MATRIX_ELEMENT;
				menode->matrix_element._id = node->terminal.lexeme._id;
				PTPNode matrix_element = node->next;
				menode->matrix_element.indices = malloc(sizeof(int)*2);
				menode->matrix_element.indices[0] =  matrix_element->nonterminal.child->next->terminal.lexeme._int;
				menode->matrix_element.indices[1] =  matrix_element->nonterminal.child->next->next->next->terminal.lexeme._int;
				// printf("Created new matrix element node: %s [%d, %d]\n", menode->matrix_element._id, menode->matrix_element.indices[0], menode->matrix_element.indices[1]);
			}
			break;
		}
	}
}

