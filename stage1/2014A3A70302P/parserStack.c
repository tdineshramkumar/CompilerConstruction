/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 1
*/


#include <stdio.h>
#include <stdlib.h>
#include "lexerDef.h"
#include "parserDef.h"
#include <stdbool.h>

typedef struct _node Node ;
typedef struct _node * Nodeptr;
typedef struct _head Stack;
typedef struct _head * StackPtr;

struct _node {
	PTPNode treenode ; // Points to TREE Node (Note: Only A Pointer)
	struct _node *prev ;
};
struct _head {
	struct _node *top ;
};

StackPtr stack ;
void initializeParserStack(){ // Pushes EOF (T_EOF) and Start Symbol (NT_PROGRAM) must be explicitly pushed
	stack = (StackPtr) malloc(sizeof(Stack)); // Create a header
	stack->top = (Nodeptr) malloc(sizeof(Node)); // Create the First Node
	PTPNode tn = (PTPNode) malloc(sizeof(PTNode)) ;
	tn->symbol = T_EOF ; // Make it point to EOF and No Need TO initialize other fields
	stack->top->treenode = tn ; // Make the Stack top Node Point to this node
	stack->top->prev = NULL ; // Set the first Node's Previous to NULL
	// Not Start Symbol MUST be explicitly Pushed
}
PTPNode topParserStack(){ // Returns top of Stack
	return stack->top->treenode ;
}
bool popParserStack(){ // Pops the top of Stack
	if ( stack->top->treenode->symbol == T_EOF ) // If EOF you can't POP the stack
		return false ;
	else {
		Nodeptr top = stack->top ;
		stack->top = stack->top->prev ;
		free(top);
		return true ;
	}
}
// void pushParserStack( RULE *rule){// Pushs symbols of the rule (in reverse order) to Stack
// 	int numsymbols = rule->size ; // Also Handles Null Production (does Nothing)
// 	for (int i = numsymbols-1 ; i >= 0; i -- ) {
// 		SYMBOL symbol = rule->symbols[i]; // Get the symbol
// 		Nodeptr tmp = (Nodeptr) malloc(sizeof(Node)); // Create a new node
// 		tmp->symbol = symbol ; // Set the symbol
// 		tmp->prev = stack->top ; // Make it point to TOP of stack
// 		stack->top = tmp ;  // Update stack top to point to it
// 	}
// }
// This one Allows Pushing ONLY one symbol at a time
void pushParserStack( PTPNode treenode ){// YOU NEED TO Push symbols of the rule (in reverse order) to Stack
	Nodeptr tmp = (Nodeptr) malloc(sizeof(Node)); // Create a new node
	tmp->treenode = treenode ; // Set the TREE NODE (Make sure to Initialize the TREE Node and Make Sure its Not NULL)
	tmp->prev = stack->top ; // Make it point to TOP of stack
	stack->top = tmp ;  // Update stack top to point to it
}
bool emptyParserStack(){// Empties the Stack
	if ( stack->top->treenode->symbol == T_EOF ) return true ;
	else return false ;
}