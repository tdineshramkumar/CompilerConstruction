/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 1
*/


#ifndef _PARSER_DEF_
#define _PARSER_DEF_

#include "lexerDef.h"
// Grammar Definition
typedef TOKEN SYMBOL; // Now onwards it is called symbol
typedef unsigned long int ulong;

typedef struct {
	SYMBOL symbol; // The deriving non-terminal (Just for information) Needed ?
	uint size ; // Number of Symbols in the rule
	SYMBOL * symbols ; // The RHS of the rule
	ulong FIRST ; // This contains the first terminal symbols derived by the rule 
} RULE;// This contains the RULE that the symbol derives

typedef struct {
	SYMBOL symbol ; // Symbol (Non-terminal Exclusively)
	RULE * rules ; // Rules Array
	uint numrules ; // Number of rules it derives (Including NUll productions)
	ulong FIRST; // Stores the first of the given non-terminal
	ulong FOLLOW; // Stores the follow of the given non-terminal
} RULESET; // This contains a set of rules derived by the non-terminal
//#define NUM_NON_TERMINALS 34 // defined in lexerDef.h
typedef RULESET Grammar[NUM_NON_TERMINALS] ; // Grammar
// Note: Number of Non-terminals 34 
// 		 Number of Terminals 38 + (NULL,EOF) 

// Parse Table Definition
// Note: Index it properly ..
typedef RULE * ParseTable[NUM_NON_TERMINALS][NUM_TERMINALS]; // Output is the RULE itself (Part of Grammar)

// Parse Tree Definition
typedef struct _ptnode PTNode ;
typedef struct _ptnode * PTPNode ; // Parse Tree Pointer node
#pragma pack(1)
struct _ptnode { // Parse Tree Node
	SYMBOL symbol ; // This contains the Current Symbol (Terminal or Non-terminal)
	PTNode *next ; // This dictates the next in the rule
	uint linenumber;// Gives the linenumber
	union {
		struct {
			PTNode *child ; // This gives the derivation applied
		} nonterminal ; // This contains information if it was non-terminal ( Just for readability)
		struct {
			union {
				int _int;
				char *_str;
				float _real;
				char *_id;
			} lexeme ;  // This contains the lexeme contained by the token ( Just for readability)
		} terminal ;// This contains information if it was terminal ( Just for readability )
	}; // Anonymous UNION
};
#pragma pack() // Restore previous packing



#endif