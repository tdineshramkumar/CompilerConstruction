/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 1
*/


#include "parserDef.h"
#include "symbolhashs.h"
#include "lexer.h"
#include "lexerDef.h"
#include "parserStack.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ASSUMES GRAMMAR IS LL1 COMPATIBLE >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//	Sample of using lexer
// int main(int argc, char **argv) {
// 	initializeLexer(argv[1]);
// 	Token token  ;
// 	do {
// 		token = getNextToken();
// 		printTokenInfo(token);
// 		//printf("linenumber: %d token: %d\n",linenumber, token.token);
// 	}while (token.token != T_EOF) ;
// 	finalizeLexer();
// 	return 0;
// }

#define EXITERROR(msg) ( { perror(msg),exit(EXIT_FAILURE);  } )
#define EXIT(msg) ( { printf("ERROR:%s\n",msg),exit(EXIT_FAILURE);  } )
#define REPORT_PARSER_ERROR(linenumber,msg,...) ({ printf( "\033[31m ERROR: [LINE(%d)] PARSER-ERROR:" msg ".\n\033[0m",linenumber, ##__VA_ARGS__); })

#ifndef _NO_DEBUG_
#define DEBUG_MSG(msg) ({ printf("DEBUG: %s\n", msg); })
#else
#define DEBUG_MSG(msg) ()
#endif

Grammar grammar ; // This contains the grammar and the first and follow
ParseTable parsetable ; // This contains the parsetable
PTPNode PTRoot ; // This will contain pointer start of PARSE TREE / ROOT(Note: Pointer)

#define MAX_SYMBOL_SIZE 100
// Don't Modify the grammar and expect it to work
// Then you also need to modify symbolshash (library and c file)

// Note: NULL is also now a symbol ... treated like a terminal ?
// This function reads grammar from a file and constructs the Grammar structure (declared as a global variable)
// This function Assumes a valid grammar (Don't Modify the grammar file and expect it to work)
// It depends on 'symbolhashs' as it identifies symbols by their hash values
// It needs no modification if no new non-terminal or terminal added
// It any addition needed then modify 'symbolhashs' to accomodate it
// WARNING: IT MAY NOT WORK IF GRAMMAR FORMAT IS ERRONEOUS (So Keep the grammar file is Some INCONSISTENT STATE)
void initializeGrammar(char *grammarfile) {
	FILE *fp = fopen(grammarfile,"r") ;
	if (fp == NULL) EXITERROR("Unable to open grammarfile.");
	char symbolstr[MAX_SYMBOL_SIZE]; 
	int numruleset ; // This contains the number of rules ( Number of NON-terminals )
	fscanf(fp,"%d",&numruleset); // Obtain it 
	int numderivations ; // This contains the number of rules given Non-terminal derives
	int numsymbols ;// The number of symbols in each rule
	SYMBOL derivingNonterminal,currSymbol ; // This contains LHS and RHS symbol(s) respectively
	for (int i = 0 ; i < numruleset ; i ++) {
		fscanf(fp,"%d",&numderivations); // get the number of derivations the non-terminal derives
		fscanf(fp,"%99s",&symbolstr); // get the non-terminal
		//printf("\n%s",symbolstr );
		derivingNonterminal = toSymbol(symbolstr); // get the deriving non-terminal symbol
		grammar[derivingNonterminal].symbol = derivingNonterminal ; // Set the non-terminal
		grammar[derivingNonterminal].rules = (RULE *) malloc(sizeof(RULE)*numderivations); // allocate the rule sets
		grammar[derivingNonterminal].numrules = numderivations ; // Set the number of rules it derives
		grammar[derivingNonterminal].FIRST = 0L ; 
		grammar[derivingNonterminal].FOLLOW = 0L ; // Later set the first and follow
		fscanf(fp,"%99s",&symbolstr); // This should have contained -> 
		//printf("->");
		for (int j = 0 ; j < numderivations; j ++ ) { 
			// Now get individual rules
			fscanf(fp,"%d",&numsymbols) ; // get number of symbols is that rule ..
			grammar[derivingNonterminal].rules[j].symbol = derivingNonterminal ; // set the deriving symbol
			grammar[derivingNonterminal].rules[j].size = numsymbols ;
			grammar[derivingNonterminal].rules[j].symbols = (SYMBOL *) malloc(sizeof(SYMBOL)*numsymbols);
			grammar[derivingNonterminal].rules[j].FIRST = 0L ; // Later calculate the first of this rule
			for (int k=0 ; k < numsymbols ; k++) {
				fscanf(fp,"%s",symbolstr);
				//printf("%s",symbolstr);
				// Assuming valid grammar
				currSymbol = toSymbol(symbolstr); // get the current symbol
				if ( currSymbol == T_NULL ) 
					// if null production then must be single symbol (only)
					free(grammar[derivingNonterminal].rules[j].symbols), // Free the already allocated memory
					grammar[derivingNonterminal].rules[j].size = 0, // Set the size to zero 
					grammar[derivingNonterminal].rules[j].symbols = NULL ; // Set the pointer to NULL
				else grammar[derivingNonterminal].rules[j].symbols[k] = currSymbol ; // set it to current symbol
			}
			if ( j != ( numderivations -1 ) ) fscanf(fp,"%s",symbolstr);// printf("%s", symbolstr); //If not last rule read the separator |
		}
	} 
	fclose(fp); // close the grammar file
} 



#define IS_TERMINAL(symbol) ( symbol < 0 ) // Terminal value is less than zero .. ( Also it returns true for control symbols as well so more like is it not a non-terminal)
#define IS_NON_TERMINAL(symbol) (symbol >= 0) // Checks if Non-terminal or Not
// Note: Assuming T_ASSIGN is first terminal declared (if modified modify it here as well)
#define INDEX_TERMINAL(symbol) ( symbol - T_ASSIGN ) // Subtract from first terminal symbol ( To start indexing from begining)
#define IS_NULL_PRODUCTION(rule) (	rule.size == 0	) // checks if NULL production (Used ?)
// Note: Non-terminal is directly accessed using their symbol
// THIS ONE IS USED IN FORST COMPUTATION
#define INDEX_NON_TERMINAL(symbol) ( symbol + 38 ) // Dont Modify that const array 38 is Number of TERMINALS in the list
#define NULL_INDEX ( T_NULL-T_ASSIGN ) // This is used to return the Index of NULL
#define CONTAINS_NULL(first) (first & (1L << NULL_INDEX)) // This is used to check if given first CONTAINS NULL
#define MASK_NULL(first) (first & ~( 1L << NULL_INDEX ) ) // This is used to remove NULL from given set if it exists
#define NOT_ALREADY_CONTAINS(first,second) (second - ( first & second )) // Checks if First set already contains all elements of Second set
// CHECK FOR NULL AS WELL in FIRST SET T_NULL
// Note: Here How to handle NULL ?
void computefirst() {

	// Used by first computation ...
	// No Needed to declare as global array as it is used only once..
	const TOKEN alltokens[] = { // ARRAY of ALL Symbols (Terminals and Non-terminals exclusively)
		// All terminal Symbols
		T_ASSIGN, T_FUNID, T_ID, T_NUM, T_RNUM, // 5
		T_STR, T_END, T_INT, T_REAL, T_STRING, T_MATRIX, // 6
		T_MAIN, T_SQO, T_SQC, T_OP, T_CL, T_SEMICOLON, T_COMMA, // 7
		T_IF, T_ELSE, T_ENDIF, T_READ, T_PRINT, T_FUNCTION, // 6
		T_PLUS, T_MINUS, T_MUL, T_DIV, T_SIZE, T_AND, T_OR, T_NOT, // 8
	 	T_LT, T_LE, T_GT, T_GE, T_EQ, T_NE, // 6  <------------ 38
	 	// All Non-terminal symbols
	 	NT_PROGRAM, NT_ARG, NT_ARGS, NT_ARITH_EXPR,  NT_ASSIGN_STMT,  NT_BOOL_EXPR,  NT_BOOL_OP,  
		NT_BOOL_OPERAND,  NT_CONDITION,  NT_COND_STMT,  NT_DECL_STMT,  NT_ELSE_STMT,  
		NT_EXPR1,  NT_EXPR2,  NT_FUNC_CALL,  NT_FUNC_DEF,  NT_IO_STMT,  NT_MATRIX,  
		NT_MATRIX_ELEMENT,  NT_MOREVARS,  NT_MORE_IN_ARITH_EXPR,  NT_MORE_IN_PROD,  
		NT_MORE_IN_ROW,  NT_MORE_PARAMS,  NT_MORE_ROWS,  NT_PARAMS,  NT_PROD_TERM,  
		NT_REL_OP,  NT_ROW,  NT_RVALUE,  NT_STMT,  NT_STMTS,  NT_TYPE,  NT_VARLIST // <------ 34
	};
	const int totalTokens = 72 ; // 34 + 38
	// Actual First Computations starts here
	bool anychange = false ;
	// This is used because Grammar has no place for storing FIRST of terminal symbols
	// NOTE Only bits for TERMINALS and NULL is being used ..
	ulong first[totalTokens] ; // This contains FIRST of All Symbols (this is then used to update grammar)
	// Set first of all terminals to itself
	for (int i=0; i < totalTokens; i++) { // The loop is inefficient BUT Okay used only once (Used to avoid many changes on modification)
		// For each of the Terminals set the first to itself
		if (IS_TERMINAL(alltokens[i])) first[i] = 1L << INDEX_TERMINAL(alltokens[i]);// if terminal then set the bit to 1
		else first[i] = 0L ; // Non-terminal  Then initialize with zero (Later modified by the following loop)
	}
	// Now compute first of all non-terminals
	do {
		anychange = false ; // This flag is used to check if any modification is being made to first sets of any symbol
		for (int i= 0;i <totalTokens; i++) { // Note i is the index ... of the symbol in first
			SYMBOL  symbol = alltokens[i]; // get the current token
			if (IS_NON_TERMINAL(symbol)) { // if non-terminal then go through its rules
				int numrules = grammar[symbol].numrules ;
				for ( int j = 0; j < numrules; j++ ) { // go through each rule
					int rulesize = grammar[symbol].rules[j].size ; // get size to determine if NULL production
					if (rulesize == 0) { // if NULL production
						if ( CONTAINS_NULL(first[i]) ) // if already contained NULL
							continue; // no change so dont do anything just continue
						else // ADD NULL
							first[i] = first[i] | ( 1L << NULL_INDEX ), // add null to first set 
							anychange = true; // Set the flag to check if any other change can be done
					}
					for (int k=0; k < rulesize; k++) { // go through each symbol ? If NULLable then look at next else break
						int currSymbol = grammar[symbol].rules[j].symbols[k]; // get the symbol
						if ( IS_TERMINAL(currSymbol) ) { // if current symbol is TERMINAL
							ulong mask = 1L << INDEX_TERMINAL(currSymbol);
							if ( NOT_ALREADY_CONTAINS(first[i], mask) ) // if current terminal not present
								first[i] = first[i] | mask , // Add that terminal (ORing is sufficient to add)
								anychange = true ; // Update the flag to indicate modification
							break; // Then don't look at any more symbols (Just break enough of going through)
						}
						else { // Non-terminal
							if ( CONTAINS_NULL( first[INDEX_NON_TERMINAL(currSymbol)] ) ) { // if that one contains NULL
								if ( k == ( rulesize - 1 ) ) { // if last symbol
									ulong mask = first[INDEX_NON_TERMINAL(currSymbol)] ;
									if ( NOT_ALREADY_CONTAINS(first[i], mask) ) // if it contains null it is automatically added
										first[i] = first[i] | mask, // add all those elements (including NULL)
										anychange = true;  // Update the flag
								}
								else { // if not symbol add all except null
									ulong mask = first[INDEX_NON_TERMINAL(currSymbol)] ; // get the first 
									mask = MASK_NULL(mask);// remove the NULL 
									if ( NOT_ALREADY_CONTAINS(first[i], mask) )  // if some elements present additionaly (apart from NULL)
										first[i] = first[i] | mask, // add all those elements
										anychange = true;  // Update the flag
								}
							}
							else { // Does Not contain NULL
								ulong mask = first[INDEX_NON_TERMINAL(currSymbol)];
								if ( NOT_ALREADY_CONTAINS(first[i], mask) ) // if terminals in mask not present
									first[i] = first[i] | mask , // Add those terminal
									anychange = true ; // Update the flag
								break; // break out of loop ( No more looking )
							}

						}
					}
				}
			}
		}
		// Wat a depth
	} while(anychange) ; // Keep looping till NO CHANGE (Initially flag set to know change)

	// Once the first Set is computed ...
	// Update the grammar first set for each non-terminal..
	for (int i=0; i < totalTokens; i++) {
		if (IS_NON_TERMINAL(alltokens[i])) // If non-terminal
			grammar[alltokens[i]].FIRST =  first[i];
	}
	// Now Update for each rule the terminals it can derive (including NULL)
	// NOTE: USING NUM_NON_TERMINALS constant
	for (int i=0; i < NUM_NON_TERMINALS ; i ++ ) { // Now loop across each rule
		int numrules = grammar[i].numrules ; // get number of RULES for each non-terminal
		for (int j=0; j < numrules; j++) {
			int rulesize = grammar[i].rules[j].size;
			if (rulesize == 0) { // if given RULE is NULL production
				grammar[i].rules[j].FIRST = (1L<< NULL_INDEX); // Then the given RULE just derives NULL
				continue ;
			}
			for (int k=0; k< rulesize; k++) { // for each symbol
				SYMBOL currSymbol = grammar[i].rules[j].symbols[k];
				if ( IS_TERMINAL(currSymbol) ){ // if terminal then add it and break;
					grammar[i].rules[j].FIRST = grammar[i].rules[j].FIRST | ( 1L << INDEX_TERMINAL(currSymbol) );
					break ;
				}
				else { // if Non-terminal
					if ( CONTAINS_NULL(grammar[currSymbol].FIRST) ) {
						if (k==rulesize-1)  // if last symbol add the null as well
							grammar[i].rules[j].FIRST = grammar[i].rules[j].FIRST | grammar[currSymbol].FIRST; // Add all symbols including NULL
						else {
							ulong mask = grammar[currSymbol].FIRST;
							mask = MASK_NULL(mask); // remove NULL and add remaining rules
							grammar[i].rules[j].FIRST = grammar[i].rules[j].FIRST | mask ; // add elements except NULL
							continue ;
						}
					}
					else { // if No NULL
						grammar[i].rules[j].FIRST = grammar[i].rules[j].FIRST | grammar[currSymbol].FIRST;
						break;
					}
				}
			}
		}
	}
}
char *NonTerminalStrings[] = {
	"PROGRAM",	"ARG", "ARGS", "ARITH_EXPR",  "ASSIGN_STMT",  "BOOL_EXPR",  "BOOL_OP",  
	"BOOL_OPERAND",  "CONDITION",  "COND_STMT",  "DECL_STMT",  "ELSE_STMT",  
	"EXPR1",  "EXPR2",  "FUNC_CALL",  "FUNC_DEF",  "IO_STMT",  "MATRIX",  
	"MATRIX_ELEMENT",  "MOREVARS",  "MORE_IN_ARITH_EXPR",  "MORE_IN_PROD",  
	"MORE_IN_ROW",  "MORE_PARAMS",  "MORE_ROWS",  "PARAMS",  "PROD_TERM",  
	"REL_OP",  "ROW",  "RVALUE",  "STMT",  "STMTS",  "TYPE",  "VARLIST"
};
char *TerminalStrings[] = {
	"assign", "funid", "id", "num", "rnum",
	"str", "end", "int", "real", "string", "matrix", 
	"main", "sqo", "sqc", "op", "cl", "semicolon", "comma",
	"if", "else", "endif", "read", "print", "function",
	"plus", "minus", "mul", "div", "size", "and", "or", "not",
 	"lt", "le", "gt", "ge", "eq", "ne","eof"
};
char *TerminalSymbols[] = {
	"assign", "<funid>", "<id>", "<num>", "<rnum>",
	"<str>", "end", "int", "real", "string", "matrix", 
	"_main", "[", "]", "(", ")", ";" , ",",
	"if", "else", "endif", "read", "print", "function",
	"+", "-", "*", "/", "@", ".and.", ".or.", ".not.",
 	"<", "<=", ">", ">=", "==", "=/=","EOF"
};
char *NonTerminalTokens[] ={ 
	"NT_PROGRAM",	"NT_ARG", "NT_ARGS", "NT_ARITH_EXPR",  "NT_ASSIGN_STMT",  "NT_BOOL_EXPR",  "NT_BOOL_OP",  
	"NT_BOOL_OPERAND",  "NT_CONDITION",  "NT_COND_STMT",  "NT_DECL_STMT",  "NT_ELSE_STMT",  
	"NT_EXPR1",  "NT_EXPR2",  "NT_FUNC_CALL",  "NT_FUNC_DEF",  "NT_IO_STMT",  "NT_MATRIX",  
	"NT_MATRIX_ELEMENT",  "NT_MOREVARS",  "NT_MORE_IN_ARITH_EXPR",  "NT_MORE_IN_PROD",  
	"NT_MORE_IN_ROW",  "NT_MORE_PARAMS",  "NT_MORE_ROWS",  "NT_PARAMS",  "NT_PROD_TERM",  
	"NT_REL_OP",  "NT_ROW",  "NT_RVALUE",  "NT_STMT",  "NT_STMTS",  "NT_TYPE",  "NT_VARLIST" 
} ;
char * TerminalTokens[] = {
	"T_ASSIGN", "T_FUNID", "T_ID", "T_NUM", "T_RNUM",
	"T_STR", "T_END", "T_INT", "T_REAL", "T_STRING", "T_MATRIX", 
	"T_MAIN", "T_SQO", "T_SQC", "T_OP", "T_CL", "T_SEMICOLON", "T_COMMA",
	"T_IF", "T_ELSE", "T_ENDIF", "T_READ", "T_PRINT", "T_FUNCTION",
	"T_PLUS", "T_MINUS", "T_MUL", "T_DIV", "T_SIZE", "T_AND", "T_OR", "T_NOT",
 	"T_LT", "T_LE", "T_GT", "T_GE", "T_EQ", "T_NE","T_EOF"
};

void printfirst() {
	printf("\n\n\t\t<<FIRST SET>>\n\n");
	
	for (int i=0; i < NUM_NON_TERMINALS ; i ++ ) {
		printf("%24s ", NonTerminalStrings[i]);
		int count = 0 ;
		
		// Find The Count 
		for (int j=0 ; j <NUM_TERMINALS -1 ; j ++) // -1 to ignore NULL
			if ( grammar[i].FIRST & ( 1L << j ) ) // if that Symbol is present
				 count ++ ;
		if ( CONTAINS_NULL(grammar[i].FIRST) ) 
			count ++ ;
		// Print the value of COUNT (number of elements in the first of given elements)
		printf("(%d):", count);
		// Now print the elements in the first
		for (int j=0 ; j <NUM_TERMINALS -1 ; j ++) { // -1 to ignore NULL
			if ( grammar[i].FIRST & ( 1L << j ) ) // if that Symbol is present
				printf(" %s", TerminalStrings[j]) ;
		}
		if ( CONTAINS_NULL(grammar[i].FIRST) ) 
			printf(" -null- ") ; // If first set contains NULL
		printf("\n");
	}
	// Now also Print First Of Each Rule
	printf("\n\n\t<< FIRST OF RULES>>\n\n");
	for (int i=0; i < NUM_NON_TERMINALS ; i ++ ) {
		int numrules = grammar[i].numrules ;
		for (int j=0; j < numrules; j++) {
			// Print each rule
			printf("%s -> ", NonTerminalStrings[i]);
			int rulesize = grammar[i].rules[j].size;
			if (rulesize == 0) {
				printf(" -null- "); // If NULL production
			}
			for (int k=0; k< rulesize; k++) {
				SYMBOL currSymbol = grammar[i].rules[j].symbols[k];
				if ( IS_TERMINAL(currSymbol)) printf(" %s ", TerminalStrings[INDEX_TERMINAL(currSymbol)]);
				else printf(" %s ", NonTerminalStrings[currSymbol] );
			}
			printf("\n");
			printf("\t%s ", NonTerminalStrings[i]);
			
			int count = 0 ;
			// Find The Count 
			for (int k=0 ; k < NUM_TERMINALS -1 ; k ++) { // -1 to ignore NULL
				if ( grammar[i].rules[j].FIRST & ( 1L << k ) ) // if that Symbol is present
					 count ++ ;
			}
			if ( CONTAINS_NULL(grammar[i].rules[j].FIRST) ) 
				count ++ ;
			printf("(%d) ==>", count);
			for (int k=0 ; k < NUM_TERMINALS -1 ; k ++) { // -1 to ignore NULL
				if ( grammar[i].rules[j].FIRST & ( 1L << k ) ) // if that Symbol is present
					printf(" %s", TerminalStrings[k]) ;
			}
			if ( CONTAINS_NULL(grammar[i].rules[j].FIRST) ) 
				printf(" -null- ") ;
			printf("\n");
		}
	}
}

void computefollow()  {
	// Set the Follow of Start to EOF
	grammar[0].FOLLOW = 1L << INDEX_TERMINAL(T_EOF);
	bool anychange = false ;
	// Till No more elements can be added to follow set
	do {
		anychange= false ; // Initially Set the flag to FALSE
		for (int i=0 ; i < NUM_NON_TERMINALS; i ++) {
			// For each rule set
			int numrules = grammar[i].numrules ;
			for (int j=0; j < numrules; j++) {
				int rulesize = grammar[i].rules[j].size ;
				for (int k=0; k < rulesize; k++) {
					SYMBOL symbol = grammar[i].rules[j].symbols[k];
					if ( IS_NON_TERMINAL(symbol)) {
						// Look at remaining symbols
						if ( (k + 1)== rulesize  ) {
							// If Last in the RULE
							// NOTE: FOLLOW DOES NOT CONTAIN NULL
							ulong mask = grammar[i].FOLLOW ; // get the follow of deriving symbol
							if ( NOT_ALREADY_CONTAINS(grammar[symbol].FOLLOW, mask) )
								grammar[symbol].FOLLOW = grammar[symbol].FOLLOW | mask, // ADD those elements
								anychange = true ;
						}
						// If Not Last Symbol
						for ( int m = k+1; m < rulesize ; m++ ) {
							SYMBOL currSymbol = grammar[i].rules[j].symbols[m];
							if ( IS_TERMINAL(currSymbol) ) {
								ulong mask = 1L << INDEX_TERMINAL(currSymbol);
								if ( NOT_ALREADY_CONTAINS(grammar[symbol].FOLLOW, mask) )
									grammar[symbol].FOLLOW = grammar[symbol].FOLLOW | mask, // ADD those elements
									anychange = true;
								break;
							}
							else {
								if ( CONTAINS_NULL(grammar[currSymbol].FIRST) ) {
									ulong mask = grammar[currSymbol].FIRST ;
									mask = MASK_NULL(mask);
									if ( NOT_ALREADY_CONTAINS(grammar[symbol].FOLLOW, mask) ) // if Follow does not contain these elements
										grammar[symbol].FOLLOW = grammar[symbol].FOLLOW | mask, // ADD those elements
										anychange = true; // Set the flag
									// If all On the Left Derive NULL add the FOLLOW of deriving symbols
										if ( (m + 1) == rulesize ) {
											// If Last Symbol
											ulong mask = grammar[i].FOLLOW ; // get the follow of deriving symbol
											if ( NOT_ALREADY_CONTAINS(grammar[symbol].FOLLOW, mask) )
												grammar[symbol].FOLLOW = grammar[symbol].FOLLOW | mask, // ADD those elements
												anychange = true ;
										}
								}
								else {
									if ( NOT_ALREADY_CONTAINS(grammar[symbol].FOLLOW,grammar[currSymbol].FIRST) ) 
										grammar[symbol].FOLLOW = grammar[symbol].FOLLOW | grammar[currSymbol].FIRST, 
										anychange  = true ; // Set the flag 
									break;
								}
							}
						}
						
					}
				}
			}
		}
	} while (anychange);
}
#define CONTAINS_EOF(follow) ( follow & (1L << INDEX_TERMINAL(T_EOF)) )
// NOTE: TOO similar to printing First Set (Just More Readable)
void printfollow() {
	printf("\n\n\t\t<<FOLLOW SET>>\n\n");
	for (int i=0; i < NUM_NON_TERMINALS ; i ++ ) {
		printf("%24s ", NonTerminalStrings[i]);
		int count = 0 ;
		// Find The Count 
		for (int j=0 ; j <NUM_TERMINALS -1 ; j ++)  // -1 to ignore NULL
			if ( grammar[i].FOLLOW & ( 1L << j ) ) // if that Symbol is present
				 count ++ ;
		if ( CONTAINS_EOF(grammar[i].FOLLOW) ) //grammar[0].FOLLOW = 1L << INDEX_TERMINAL(T_EOF);
			count ++ ;
		// Print the count
		printf("(%d):", count);
		for (int j=0 ; j <NUM_TERMINALS -1 ; j ++) { // -1 to ignore NULL
			if ( grammar[i].FOLLOW & ( 1L << j ) ) // if that Symbol is present
				printf(" %s", TerminalStrings[j]) ;
		}
		if ( CONTAINS_EOF(grammar[i].FOLLOW) ) 
			printf(" T_EOF") ;
		printf("\n");
	}
}
/*
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
typedef RULESET Grammar[NUM_NON_TERMINALS] ; // Grammar
typedef RULE * ParseTable[NUM_NON_TERMINALS][NUM_TERMINALS]; // Output is the RULE itself (Part of Grammar)

*/
void fillParseTable() {
	// Initialize With NULL (To Indicate By Default Error)
	for (int i=0; i< NUM_NON_TERMINALS; i++) 
		for (int j=0; j< NUM_TERMINALS; j++) 
			parsetable[i][j] = NULL ; // Initialize with NULL
		
	for (int i=0; i< NUM_NON_TERMINALS; i++) {
		uint numrules = grammar[i].numrules ;
		for (int j=0; j< numrules; j++) {
			// For each rule in the grammar
			ulong mask = grammar[i].rules[j].FIRST ;
			// Now based on FIRST of RULE add RULES to Parse Table
			for (int k=0 ; k < NUM_TERMINALS -1 ; k ++) { // Note NUM_TERMINALS also counts EOF so -1
				if ( mask & ( 1L << k ) ){ 
					// if that Terminal Symbol is present
					parsetable[i][k] = &(grammar[i].rules[j]); //Add an entry to parse table
				}
			}
			// If FIRST contains NULL then 
			// LOOK at follow and add those NULL Productions (Direct or Indirect)
			if ( CONTAINS_NULL(grammar[i].rules[j].FIRST) ) {
				ulong mask = grammar[i].FOLLOW ;
				for (int k=0; k< NUM_TERMINALS -1; k ++) {
					if ( mask & ( 1L << k ) ){ 
						// if that Terminal Symbol is present in follow
						parsetable[i][k] = &(grammar[i].rules[j]); //Add an entry to parse table
					}
				}
				// If FOLLOW contains EOF then add it as well
				if ( CONTAINS_EOF(grammar[i].FOLLOW) ) {
					parsetable[i][NUM_TERMINALS-1] = &(grammar[i].rules[j]);// Add entry for EOF as well
				}
			}
		}
	}
}
// This function is Used to Initialize Parser
void initializeParser(char* grammarfile) {
	// Built the grammar from the file
	initializeGrammar(grammarfile);
	// Update first and follow of the grammar
	computefirst(); 
	computefollow();
	
	// Using first and follow fill the parse table
	fillParseTable();

	// Now to begin Parsing Process
	// Initialize Parser's Stack
	initializeParserStack();
	// Push the start symbol
	PTRoot = (PTPNode) malloc(sizeof(PTNode));
	PTRoot->symbol = NT_PROGRAM ;// Start Symbol 
	PTRoot->next = NULL ; // No Rule Following It
	PTRoot->nonterminal.child = NULL ; // Currently No derivation
	PTRoot->linenumber = 0 ; // Set Default LINE NUMBER to 0
	pushParserStack(PTRoot);// Push the first Node
	// Now parsing can begin...
}
// Print parse tree to file
void printparsetreenodetofile( PTPNode node, PTPNode parent,FILE *fp) { 
	if ( node == NULL) fprintf(fp,"WARNING: NULL NODE\n");
	//Leftmost child ‐‐> parent node‐‐> remaining siblings (excluding the leftmost)
	if ( IS_TERMINAL(node->symbol) ) {
		//printf("IS TERMINAL\n");
		// If terminal symbol Then Print Accordingly
		switch( node->symbol ) { 
			case T_NUM: 	fprintf(fp,"%-22d %-6d %-20s %-20s %-6s %-24s\n",node->terminal.lexeme._int,node->linenumber,TerminalStrings[INDEX_TERMINAL(node->symbol)],NonTerminalStrings[parent->symbol],"YES",TerminalTokens[INDEX_TERMINAL(node->symbol)]); break;
			case T_RNUM: 	fprintf(fp,"%-22.2f %-6d %-20s %-20s %-6s %-24s\n",node->terminal.lexeme._real,node->linenumber,TerminalStrings[INDEX_TERMINAL(node->symbol)],NonTerminalStrings[parent->symbol],"YES",TerminalTokens[INDEX_TERMINAL(node->symbol)]); break;
			case T_ID: fprintf(fp,"%-22s %-6d %-20s %-20s %-6s %-24s\n",node->terminal.lexeme._id,node->linenumber,TerminalStrings[INDEX_TERMINAL(node->symbol)],NonTerminalStrings[parent->symbol],"YES",TerminalTokens[INDEX_TERMINAL(node->symbol)]); break;
			case T_FUNID: fprintf(fp,"%-22s %-6d %-20s %-20s %-6s %-24s\n",node->terminal.lexeme._id,node->linenumber,TerminalStrings[INDEX_TERMINAL(node->symbol)],NonTerminalStrings[parent->symbol],"YES",TerminalTokens[INDEX_TERMINAL(node->symbol)]); break;
			case T_STR: fprintf(fp,"%-22s %-6d %-20s %-20s %-6s %-24s\n",node->terminal.lexeme._str,node->linenumber,TerminalStrings[INDEX_TERMINAL(node->symbol)],NonTerminalStrings[parent->symbol],"YES",TerminalTokens[INDEX_TERMINAL(node->symbol)]); break;
			default: fprintf(fp,"%-22s %-6d %-20s %-20s %-6s %-24s\n",TerminalSymbols[INDEX_TERMINAL(node->symbol)],node->linenumber,TerminalStrings[INDEX_TERMINAL(node->symbol)],NonTerminalStrings[parent->symbol],"YES",TerminalTokens[INDEX_TERMINAL(node->symbol)]); break;
		}
	}
	else {
		//printf("IS NONTERMINAL\n");
		// Non-terminal ...
		// MAY BE IT HAS NO CHILDREN (Parsing Error)
		if ( node->nonterminal.child == NULL )  //printf("%s %d NOCHILD\n",NonTerminalStrings[node->symbol], node->linenumber); 
			if (parent != NULL)	fprintf(fp,"%-22s %-6d %-20s %-20s %-6s %-24s\n","-------",node->linenumber,NonTerminalStrings[node->symbol],NonTerminalStrings[parent->symbol],"YES",NonTerminalTokens[node->symbol]);
			else fprintf(fp,"%-22s %-6d %-20s %-20s %-6s %-24s\n","-------",node->linenumber,NonTerminalStrings[node->symbol],"ROOT","YES",NonTerminalTokens[node->symbol]);
		
		else { 
			// Print the first Child
			PTPNode tmp = node->nonterminal.child ; // Get the first child
			printparsetreenodetofile(tmp,node,fp); // Print the first child
			
			// Print the current node ...
			if (parent!= NULL) fprintf(fp,"%-22s %-6d %-20s %-20s %-6s %-24s\n","-------",node->linenumber,NonTerminalStrings[node->symbol],NonTerminalStrings[parent->symbol],"NO",NonTerminalTokens[node->symbol]);
			else fprintf(fp,"%-22s %-6d %-20s %-20s %-6s %-24s\n","-------",node->linenumber,NonTerminalStrings[node->symbol],"ROOT","NO",NonTerminalTokens[node->symbol]);

			// Print rest of children
			while ( (tmp = tmp->next ) != NULL ) {
				// For each of other children
				printparsetreenodetofile(tmp, node,fp);
			}
		}
	}
}
void printparsetreetofile(char *outfile) {
	FILE *fp = fopen(outfile,"w");
	if ( fp == NULL )
		perror("Unable to write to file");
	else {
		fprintf(fp,"\n\n\t\t\tPARSE TREE (IN-ORDER TRAVERSAL)\n" );
		for (int i=0; i < 90; i++) fprintf(fp,"="); fprintf(fp,"\n");
		fprintf(fp,"%-22s %-6s %-20s %-20s %-6s %-24s\n", "LEXEME","LINENO","TOKEN","PARENT","ISLEAF","SYMBOL");
		for (int i=0; i < 90; i++) fprintf(fp,"_"); fprintf(fp,"\n");
		printparsetreenodetofile(PTRoot,NULL,fp);
	}
	fclose(fp);
	
}
//In-Order Traversal <<<<<<<<<<<<<<<<<<<<<<<<<<<<<< MODIFIED
// NOTE: I have to preserved the lexeme in case of REAL or INT
// NOTE: node contains current node and parent its parent
void printparsetreenode( PTPNode node, PTPNode parent ) { 
	if ( node == NULL) printf("WARNING: NULL NODE\n");
	//Leftmost child ‐‐> parent node‐‐> remaining siblings (excluding the leftmost)
	if ( IS_TERMINAL(node->symbol) ) {
		//printf("IS TERMINAL\n");
		// If terminal symbol Then Print Accordingly
		switch( node->symbol ) { 
			case T_NUM: 	printf("%-22d %-6d %-20s %-20s %-6s %-24s\n",node->terminal.lexeme._int,node->linenumber,TerminalStrings[INDEX_TERMINAL(node->symbol)],NonTerminalStrings[parent->symbol],"YES",TerminalTokens[INDEX_TERMINAL(node->symbol)]); break;
			case T_RNUM: 	printf("%-22.2f %-6d %-20s %-20s %-6s %-24s\n",node->terminal.lexeme._real,node->linenumber,TerminalStrings[INDEX_TERMINAL(node->symbol)],NonTerminalStrings[parent->symbol],"YES",TerminalTokens[INDEX_TERMINAL(node->symbol)]); break;
			case T_ID: printf("%-22s %-6d %-20s %-20s %-6s %-24s\n",node->terminal.lexeme._id,node->linenumber,TerminalStrings[INDEX_TERMINAL(node->symbol)],NonTerminalStrings[parent->symbol],"YES",TerminalTokens[INDEX_TERMINAL(node->symbol)]); break;
			case T_FUNID: printf("%-22s %-6d %-20s %-20s %-6s %-24s\n",node->terminal.lexeme._id,node->linenumber,TerminalStrings[INDEX_TERMINAL(node->symbol)],NonTerminalStrings[parent->symbol],"YES",TerminalTokens[INDEX_TERMINAL(node->symbol)]); break;
			case T_STR: printf("%-22s %-6d %-20s %-20s %-6s %-24s\n",node->terminal.lexeme._str,node->linenumber,TerminalStrings[INDEX_TERMINAL(node->symbol)],NonTerminalStrings[parent->symbol],"YES",TerminalTokens[INDEX_TERMINAL(node->symbol)]); break;
			default: printf("%-22s %-6d %-20s %-20s %-6s %-24s\n",TerminalSymbols[INDEX_TERMINAL(node->symbol)],node->linenumber,TerminalStrings[INDEX_TERMINAL(node->symbol)],NonTerminalStrings[parent->symbol],"YES",TerminalTokens[INDEX_TERMINAL(node->symbol)]); break;
		}
	}
	else {
		//printf("IS NONTERMINAL\n");
		// Non-terminal ...
		// MAY BE IT HAS NO CHILDREN (Parsing Error)
		if ( node->nonterminal.child == NULL )  //printf("%s %d NOCHILD\n",NonTerminalStrings[node->symbol], node->linenumber); 
			if (parent != NULL)	printf("%-22s %-6d %-20s %-20s %-6s %-24s\n","-------",node->linenumber,NonTerminalStrings[node->symbol],NonTerminalStrings[parent->symbol],"YES",NonTerminalTokens[node->symbol]);
			else printf("%-22s %-6d %-20s %-20s %-6s %-24s\n","-------",node->linenumber,NonTerminalStrings[node->symbol],"ROOT","YES",NonTerminalTokens[node->symbol]);
		
		else { 
			// Print the first Child
			PTPNode tmp = node->nonterminal.child ; // Get the first child
			printparsetreenode(tmp,node); // Print the first child
			
			// Print the current node ...
			if (parent!= NULL) printf("%-22s %-6d %-20s %-20s %-6s %-24s\n","-------",node->linenumber,NonTerminalStrings[node->symbol],NonTerminalStrings[parent->symbol],"NO",NonTerminalTokens[node->symbol]);
			else printf("%-22s %-6d %-20s %-20s %-6s %-24s\n","-------",node->linenumber,NonTerminalStrings[node->symbol],"ROOT","NO",NonTerminalTokens[node->symbol]);

			// Print rest of children
			while ( (tmp = tmp->next ) != NULL ) {
				// For each of other children
				printparsetreenode(tmp, node);
			}
		}
	}
}

void printparsetree() {
	printf("\n\n\t\t\tPARSE TREE (IN-ORDER TRAVERSAL)\n" );
	for (int i=0; i < 90; i++) printf("="); printf("\n");
	printf("%-22s %-6s %-20s %-20s %-6s %-24s\n", "LEXEME","LINENO","TOKEN","PARENT","ISLEAF","SYMBOL");
	for (int i=0; i < 90; i++) printf("_"); printf("\n");
	printparsetreenode(PTRoot,NULL);
}
#define _NO_PT_DEBUG_

#define  GRAMMAR_FILE "grammar-final.txt"
// DEBUG MESSAGE WHILE GENERATING PARSE TREE
#ifndef _NO_PT_DEBUG_
//# define MYLOG(FormatLiteral, ...)  fprintf (stderr, "%s(%u): " FormatLiteral "\n", __FILE__, __LINE__, __VA_ARGS__)
#define PT_MSG(msg, ...) ({ printf("[%s,%d] DEBUG: " msg "\n",__FILE__, __LINE__, ##__VA_ARGS__ ); })
#define PT_PRINT1(msg, ...)  ({ printf("[%s,%d] DEBUG: " msg ,__FILE__, __LINE__, ##__VA_ARGS__ ); })
#define PT_PRINT(FORMAT, ...) ({ printf(FORMAT,##__VA_ARGS__); })
#else
#define PT_MSG(msg,...) 
#define PT_PRINT1(msg,...) 
#define PT_PRINT(FORMAT,...) 
#endif
// This function Pops the current TOP of stack and Add the rules it derives
// YOU USE LookAhead used to update the linenumber
// YOU MUST UPDATE TOP OF STACK 
void addruletostack(RULE *rule) {
	//PT_MSG("TOP (non-terminal): %s lookahead: %s (MATCHING NON-TERMINAL)", 
	//NonTerminalStrings[top->symbol], TerminalStrings[ INDEX_TERMINAL(lookahead.token) ] );
	
	// Set the linenumber of the non-terminal
	//top->linenumber = lookahead.linenumber; // Copy the linenumber
	PTPNode top = topParserStack();
	int size = rule->size ; // get the rule size
	if ( size != 0) { // If not a NULL production then add that rule to stack
		PTPNode prev = NULL ; // to check if first NODE (stores the previous PTPNode)
		PTPNode * ptpnodes = (PTPNode *) malloc(sizeof(PTPNode) * size); // Create an array useful for pushing to stack
		// For each symbol in the rule ...
		for (int i=0 ; i < size; i++){
			// For each symbol in the rule
			// Create a TREE NODE
			PTPNode tmp = (PTPNode) malloc(sizeof(PTNode)); // create a new node
			tmp->nonterminal.child = NULL ; // <<<<<<--------------------- MODIFIED SETTING DEFAULT CHILD TO NULL
			tmp->symbol = rule->symbols[i];// set the symbol
			tmp->next = NULL ; // In case if last symbol keep it ...
			// Now link the nodes to form the rule
			// First Step Link with its Deriving Symbol (The Parent)
			if ( prev == NULL ) // First Child
				top->nonterminal.child = tmp ; // Update the Parent ..
			else  // If Not FIRST child ( then update the next of prev)
				prev->next = tmp ; // Set the previous node's next to tmp
			prev = tmp ; // Update the previous
			ptpnodes[i] = tmp ; // Add to array to use later for Pushing In Reverse Order to Stack
		}
		
		// Pop The Token of stack
		popParserStack(); // It is the current parent
		for (int i = size-1 ; i >= 0; i--) {
			pushParserStack(ptpnodes[i]);// Now push these in reverse order ...
		}
		free(ptpnodes); // Free the array
		//top = topParserStack(); // Update the top
	}
	else {
		// If NULL production
		// We Don't Differentiate NULL and ERROR
		// If symbol simply poped of THEN IT MUST BE NULL 
		top->nonterminal.child = NULL ;
		popParserStack();
		//top = topParserStack();
	}
}

#define REPORT_PARSER_ERRORex1(top,lookahead) ({  switch (lookahead.token ) { \
	case T_STR: REPORT_PARSER_ERROR( lookahead.linenumber, " expected a %s '%s'. unexpectedly received %s '%s' before it. inserting it.", TerminalStrings[ INDEX_TERMINAL(top->symbol) ],TerminalSymbols[ INDEX_TERMINAL(top->symbol) ],TerminalStrings [ INDEX_TERMINAL(lookahead.token) ] , lookahead.lexeme._str );break;\
	case T_ID: case T_FUNID:REPORT_PARSER_ERROR( lookahead.linenumber, " expected a %s '%s'. unexpectedly received %s '%s' before it. inserting it.",TerminalStrings[ INDEX_TERMINAL(top->symbol) ],TerminalSymbols[ INDEX_TERMINAL(top->symbol) ], TerminalStrings [ INDEX_TERMINAL(lookahead.token) ] , lookahead.lexeme._id );break;\
	case T_NUM: REPORT_PARSER_ERROR( lookahead.linenumber, " expected a %s '%s'. unexpectedly received %s '%d' before it. inserting it.", TerminalStrings[ INDEX_TERMINAL(top->symbol) ],TerminalSymbols[ INDEX_TERMINAL(top->symbol) ], TerminalStrings [ INDEX_TERMINAL(lookahead.token) ] , lookahead.lexeme._int ); break;\
	case T_RNUM: REPORT_PARSER_ERROR( lookahead.linenumber, " expected a %s '%s'. unexpectedly received %s '%f' before it. inserting it.", TerminalStrings[ INDEX_TERMINAL(top->symbol) ],TerminalSymbols[ INDEX_TERMINAL(top->symbol) ], TerminalStrings [ INDEX_TERMINAL(lookahead.token) ] , lookahead.lexeme._real ); break;\
	default:REPORT_PARSER_ERROR( lookahead.linenumber, " expected a %s '%s'. unexpectedly received %s '%s' before it. inserting it.", TerminalStrings[ INDEX_TERMINAL(top->symbol) ],TerminalSymbols[ INDEX_TERMINAL(top->symbol) ],TerminalStrings [ INDEX_TERMINAL(lookahead.token) ] ,  TerminalSymbols[ INDEX_TERMINAL(lookahead.token) ] );\
} })
// This one top is a non-terminal
#define REPORT_PARSER_ERRORex2(top,lookahead) ({  switch (lookahead.token ) { \
	case T_STR: REPORT_PARSER_ERROR( lookahead.linenumber, " expecting a derivation of %s . unexpectedly received %s '%s' before it.", NonTerminalStrings[top->symbol], TerminalStrings [ INDEX_TERMINAL(lookahead.token) ] , lookahead.lexeme._str );break;\
	case T_ID: case T_FUNID:REPORT_PARSER_ERROR( lookahead.linenumber, "  expecting a derivation of %s . unexpectedly received %s '%s' before it.", NonTerminalStrings[top->symbol], TerminalStrings [ INDEX_TERMINAL(lookahead.token) ] , lookahead.lexeme._id );break;\
	case T_NUM: REPORT_PARSER_ERROR( lookahead.linenumber, "  expecting a derivation of%s . unexpectedly received %s '%d' before it.",  NonTerminalStrings[top->symbol], TerminalStrings [ INDEX_TERMINAL(lookahead.token) ] , lookahead.lexeme._int ); break;\
	case T_RNUM: REPORT_PARSER_ERROR( lookahead.linenumber, "  expecting a derivation of %s . unexpectedly received %s '%f' before it.", NonTerminalStrings[top->symbol], TerminalStrings [ INDEX_TERMINAL(lookahead.token) ] , lookahead.lexeme._real ); break;\
	default:REPORT_PARSER_ERROR( lookahead.linenumber, "  expecting a derivation of %s . unexpectedly received %s '%s' before it.",  NonTerminalStrings[top->symbol] ,TerminalStrings [ INDEX_TERMINAL(lookahead.token) ] ,  TerminalSymbols[ INDEX_TERMINAL(lookahead.token) ] );\
} })
#define REPORT_PARSER_ERRORex3(lookahead) ({  switch (lookahead.token ) { \
	case T_STR: REPORT_PARSER_ERROR( lookahead.linenumber, "unexpectedly received %s '%s'  after end of main. dropping it.",TerminalStrings [ INDEX_TERMINAL(lookahead.token) ] , lookahead.lexeme._str );break;\
	case T_ID: case T_FUNID:REPORT_PARSER_ERROR( lookahead.linenumber, "unexpectedly received %s '%s'  after end of main. dropping it.", TerminalStrings [ INDEX_TERMINAL(lookahead.token) ] , lookahead.lexeme._id );break;\
	case T_NUM: REPORT_PARSER_ERROR( lookahead.linenumber, "unexpectedly received %s '%d' after end of main. dropping it.", TerminalStrings [ INDEX_TERMINAL(lookahead.token) ] , lookahead.lexeme._int ); break;\
	case T_RNUM: REPORT_PARSER_ERROR( lookahead.linenumber, "unexpectedly received %s '%f'  after end of main. dropping it.",TerminalStrings [ INDEX_TERMINAL(lookahead.token) ] , lookahead.lexeme._real ); break;\
	default:REPORT_PARSER_ERROR( lookahead.linenumber, "unexpectedly received %s '%s' after end of main. dropping it.",TerminalStrings [ INDEX_TERMINAL(lookahead.token) ] ,  TerminalSymbols[ INDEX_TERMINAL(lookahead.token) ] );\
} })
//#define EXPRESSION_SYMBOLS (NT_ARITH_EXPR | NT_EXPR1 | NT_EXPR2 | NT_MORE_IN_ARITH_EXPR | NT_MORE_IN_PROD | NT_PROD_TERM | NT_RVALUE)
//#define RESYNC_SET (T_END | T_INT | T_REAL | T_STRING | T_MATRIX | T_SQO | T_IF | T_ELSE | T_ENDIF | T_READ | T_PRINT |T_FUNCTION | T_EOF)
//#define CONTAINS_NONTERMINAL( ntset, nonterminalsymbol) ( ntset & (1L << nonterminalsymbol ) )

#define CONTAINS_TERMINAL(ffset, terminalsymbol) (ffset & (1L << INDEX_TERMINAL(terminalsymbol)))

bool parseInputFile(char *programfile) {
	bool successfulparsing = true ; // Flag to check if any error occured ... NOTE: it does not take into account the lexer...
	// read the grammar file, generate first follow and 
	// and initializes parser stack
	initializeParser(GRAMMAR_FILE); 
	initializeLexer(programfile);
	// Run the parser ....
	Token lookahead = getNextToken() ; // Get the first token
	PTPNode top = topParserStack(); // get the top of stack
	while ( !emptyParserStack() ) { // If some elements on stack apart from EOF
		if ( IS_TERMINAL(top->symbol) ) {
			// If top of stack is an INPUT
			if ( top->symbol == lookahead.token ) {
				PT_MSG("TOP (terminal): %s lookahead: %s (MATCHING TERMINAL)", TerminalStrings[ INDEX_TERMINAL(top->symbol) ], TerminalStrings[ INDEX_TERMINAL(lookahead.token) ] );
				// If top of stack symbol matches lookahead symbol
				//Don't MODIFY it top->next (It is a part of RULE)
				top->linenumber = lookahead.linenumber; // Copy the linenumber
				// Copy the lexeme
				switch( lookahead.token ) {
					case T_NUM:	top->terminal.lexeme._int = lookahead.lexeme._int; 	break;
					case T_RNUM: top->terminal.lexeme._real = lookahead.lexeme._real;	break;
					case T_ID:	top->terminal.lexeme._id = lookahead.lexeme._id ; break;
					case T_FUNID: top->terminal.lexeme._id = lookahead.lexeme._id ; break;
					case T_STR: top->terminal.lexeme._str = lookahead.lexeme._str;	break;
					// Else no other have value
				}
				popParserStack(); // Pop the top of stack
				lookahead = getNextToken(); // Update the lookahead
				top = topParserStack(); // Update the top of stack
			}
			else {
					successfulparsing = false ; // Just for indication 
				// PANIC MODE RECOVERY
				// If NOT matching the TERMINAL
				// IF TERMINAL on top of stack connot be matched ,
				// Simplest idea is to pop the terminal and issue a message...
				successfulparsing = false ;
				REPORT_PARSER_ERRORex1(top,lookahead);
				
				PT_MSG("TOP (terminal): %s lookahead: %s (NOT MATCHING TERMINAL)", TerminalStrings[ INDEX_TERMINAL(top->symbol) ], TerminalStrings[ INDEX_TERMINAL(lookahead.token) ] );
				top->linenumber = lookahead.linenumber; // Copy the linenumber of the Unexpected Symbol
				
				// Initialize with default values
				switch( top->symbol ) { 
					case T_NUM:	top->terminal.lexeme._int = 0; 	break;
					case T_RNUM: top->terminal.lexeme._real = 0.0f;	break;
					case T_ID:	top->terminal.lexeme._id = "invalidID" ; break;
					case T_FUNID: top->terminal.lexeme._id = "invalidFUNID" ; break;
					case T_STR: top->terminal.lexeme._str = "\"invalid string\"";	break;
				}
				popParserStack(); // Pop the top of stack
				// NOTE: LOOKAHEAD Not Updated ..
				top = topParserStack(); // Update the top of stack
			}
		}
		else {
			// If non-terminal on top
			if ( parsetable[top->symbol][INDEX_TERMINAL(lookahead.token)] == NULL ) {
					successfulparsing = false ; // Just to indicate an error occured 
				// If no RULE in the ParseTable
				// PANIC MODE ....
				PT_MSG("TOP (non-terminal): %s lookahead: %s (NOT MATCHING NON-TERMINAL)", 
				NonTerminalStrings[top->symbol], TerminalStrings[ INDEX_TERMINAL(lookahead.token) ] );
				// PANIC MODE
				//REPORT_PARSER_ERROR(lookahead.linenumber,"Unexpected %s '%s'. Expecting a derivation of %s.",TerminalStrings[INDEX_TERMINAL(lookahead.token)],TerminalSymbols[INDEX_TERMINAL(lookahead.token)],NonTerminalStrings[top->symbol]);
				REPORT_PARSER_ERRORex2(top,lookahead);
				// Skip through symbols till recovery possible ...
				while (true) {
					
					// Get the Next Element From The Stream ...
					//lookahead = getNextToken(); // The Next Token (Should it be at the first or last ?)
					// If EOF at lookahead then pop all symbols from stack ...
					if (lookahead.token == T_EOF) {
						top->linenumber = lookahead.linenumber; // Set the linenumber of current symbol as well
						if (IS_NON_TERMINAL(top->symbol)) { // If top of stack is non-terminal set it to NULL
							top->nonterminal.child = NULL ; // No children ..
							PT_MSG("TOP (non-terminal): %s Forced to NULL", NonTerminalStrings[top->symbol]);
						}
						else 
						 	PT_MSG("TOP (terminal): %s Popped ", TerminalStrings[INDEX_TERMINAL(top->symbol)]);
						// If end of Input STREAM
						while (!emptyParserStack()) {
							popParserStack();
							top = topParserStack();
							top->linenumber = lookahead.linenumber;
							if (IS_NON_TERMINAL(top->symbol)) {	// If top of stack is non-terminal set it to NULL
								top->nonterminal.child = NULL ; // No children ..
								PT_MSG("TOP (non-terminal): %s  Forced to NULL", NonTerminalStrings[top->symbol]);
							}
							else  
								PT_MSG("TOP (terminal): %s Popped ", TerminalStrings[INDEX_TERMINAL(top->symbol)]);
						} 
						break;
					}
					// If symbol now matchs first then rule can be applied ...
					if ( CONTAINS_TERMINAL( grammar[top->symbol].FIRST,lookahead.token) ) {
						PT_MSG("TOP (non-terminal): %s derives %s. Using that production.",NonTerminalStrings[top->symbol], TerminalStrings[ INDEX_TERMINAL(lookahead.token) ] );
						// If contained in first set then expand with appropriate RULE
						// Update the linenumber of TOP ..
						top->linenumber = lookahead.linenumber; // Copy the linenumber
						// Use it
						// Now find the corresponding RULE... 
						// One Option is to use parsetable or other is Too Look through rules
						// Since It's Error Recovery (Not Part of Successful compilation)
						// Obtain the RULE
						RULE * rule = parsetable[top->symbol][INDEX_TERMINAL(lookahead.token)];
						// Pop the stack and Apply That Rule..
						addruletostack(rule);
						// After Adding that RULE Update the top of stack
						top = topParserStack();// Update the top of stack
						break; // break out of the recovery loop
					}
					else if ( CONTAINS_TERMINAL( grammar[top->symbol].FOLLOW, lookahead.token) ) {
						// FIRST CHECK IF GIVEN SYMBOL DERIVES NULL THEN USE IT INSTEAD OF FORCING
						if (  CONTAINS_NULL( grammar[top->symbol].FIRST) ) {
							PT_MSG("TOP (non-terminal): %s does not derive %s but FOLLOW set contains it. Also it derives NULL so using that production.",NonTerminalStrings[top->symbol],TerminalStrings[ INDEX_TERMINAL(lookahead.token)]);
							// Update the linenumber with the TOKEN
							top->linenumber = lookahead.linenumber; // Copy the linenumber
							// First find that production...
							for ( int i= 0 ; i < grammar[top->symbol].numrules ; i++) {
								RULE * rule = &(grammar[top->symbol].rules[i]);
								ulong mask = grammar[top->symbol].rules[i].FIRST ;
								if ( CONTAINS_NULL( mask) ) {
									// Pop the stack and Apply That Rule..
									addruletostack(rule);
									break;
								}
							}
							// After Adding that RULE Update the top of stack
							top = topParserStack();// Update the top of stack
							// Note: LOOKAHEAD NOT UPDATED ..
							break; // Break out of the loop
						}
						// ELSE THEN YOU NEED TO POP THE SYMBOL AND LOOK AT NEXT SYMBOL
						// MAY BE YOU MAY USE AN RECOVERY SET LATER
						/////////////////////////////////////////
						PT_MSG("TOP (non-terminal): %s does not derive %s but FOLLOW set contains it. Forcing Current Production to NULL.",NonTerminalStrings[top->symbol], TerminalStrings[ INDEX_TERMINAL(lookahead.token) ] );
						// Update the linenumber of TOP ..
						top->linenumber = lookahead.linenumber; // Copy the linenumber
						// Set the derivation to NULL
						top->nonterminal.child = NULL ;
						// Pop That TOP of stack with ERROR warning then Proceed...
						// Already One Error So But do you need to Tell More ?
						popParserStack(); 
						top = topParserStack();
						break;
					}

					PT_MSG("TOP (non-terminal): %s does not derive %s and also not in FOLLOW set as well. Skipping it.",NonTerminalStrings[top->symbol], TerminalStrings[ INDEX_TERMINAL(lookahead.token) ] );
					// Update the lookahead
					lookahead = getNextToken(); // Add it at the bottom and checking
				}
				// If no panic mode .. just exit (comment out previous lines)
				// EXIT("No RULE in the PARSE TABLE");
			}
			else {
				PT_MSG("TOP (non-terminal): %s lookahead: %s (MATCHING NON-TERMINAL)", 
				NonTerminalStrings[top->symbol], TerminalStrings[ INDEX_TERMINAL(lookahead.token) ] );
				
				// Update the linenumber of the non-terminal on top of the stack
				top->linenumber = lookahead.linenumber; // Copy the linenumber
				// Obtain the RULE
				RULE * rule = parsetable[top->symbol][INDEX_TERMINAL(lookahead.token)];

				// PRINT THE RULE
				int size = rule->size ;
				PT_PRINT1("%s (%s) -> ", NonTerminalStrings[rule->symbol],NonTerminalStrings[top->symbol]);
				if (size == 0) PT_PRINT(" -null- ");
				for (int i=0;i < size; i++) 
					if (IS_TERMINAL( rule->symbols[i] )) PT_PRINT(" %s ", TerminalStrings[INDEX_TERMINAL(rule->symbols[i])] );
					else PT_PRINT(" %s ", NonTerminalStrings[rule->symbols[i]] );
				PT_PRINT("\n");

				// Pop that Symbol and Add that RULE to top of stack 
				addruletostack(rule);
				// UPDATE the TOP of stack
				top = topParserStack();
			}
		} // Check if more elements in the INPUT
	}

	// Now stack empty Last step check if INPUT is empty
	//lookahead = getNextToken();
	while ( lookahead.token != T_EOF ) {
			successfulparsing = false ; // if too many input to error ...
		// <--------------- MODIFIED  COMPLETE IT !!!
		PT_MSG("Unexpected Tokens after EOF");
		REPORT_PARSER_ERRORex3(lookahead);
		lookahead = getNextToken();
	}
	return successfulparsing ; // This tells if program was syntactically correct or not ...
}

// int main(int argc, char *argv[]) // first Argument is Program File
// {
// 	initializeParser(GRAMMAR_FILE);
// 	//printfollow();
// 	initializeLexer(argv[1]);
// 	Token lookahead = getNextToken() ; // Get the first token
// 	PTPNode top = topParserStack(); // get the top of stack
// 	while ( !emptyParserStack() ) {
// 		// Note: T_EOF treated like any other symbol
// 		// Not EMPTY stack
// 		if ( IS_TERMINAL(top->symbol) ) {
// 			// If top of stack is an INPUT
// 			if ( top->symbol == lookahead.token ) {
// 				PT_MSG("TOP (terminal): %s lookahead: %s (MATCHING TERMINAL)", TerminalStrings[ INDEX_TERMINAL(top->symbol) ], TerminalStrings[ INDEX_TERMINAL(lookahead.token) ] );
// 				// If top of stack symbol matches lookahead symbol
// 				//Don't MODIFY it top->next (It is a part of RULE)
// 				top->linenumber = lookahead.linenumber; // Copy the linenumber
// 				//top->info.terminal.lexeme = lookahead.lexeme ; // Copy the lexeme
// 				switch( lookahead.token ) {
// 					case T_NUM:	top->terminal.lexeme._int = lookahead.lexeme._int; 	break;
// 					case T_RNUM: top->terminal.lexeme._real = lookahead.lexeme._real;	break;
// 					case T_ID:	top->terminal.lexeme._id = lookahead.lexeme._id ; break;
// 					case T_FUNID: top->terminal.lexeme._id = lookahead.lexeme._id ; break;
// 					case T_STR: top->terminal.lexeme._str = lookahead.lexeme._str;	break;
// 					// Else no other have value
// 				}
// 				popParserStack(); // Pop the top of stack
// 				lookahead = getNextToken(); // Update the lookahead
// 				top = topParserStack(); // Update the top of stack
// 			}
// 			else {
// 				// PANIC MODE RECOVERY
// 				// If NOT matching the TERMINAL
// 				// IF TERMINAL on top of stack connot be matched ,
// 				// Simplest idea is to pop the terminal and issue a message...
// 				//REPORT_PARSER_ERROR( lookahead.linenumber, " expected a %s '%s'. unexpectedly received %s '%s' before it. inserting it.", TerminalStrings[ INDEX_TERMINAL(top->symbol) ],TerminalSymbols[ INDEX_TERMINAL(top->symbol) ], TerminalStrings [ INDEX_TERMINAL(lookahead.token) ] , TerminalSymbols[ INDEX_TERMINAL(lookahead.token) ] );
// 				REPORT_PARSER_ERRORex1(top,lookahead);
// 				PT_MSG("TOP (terminal): %s lookahead: %s (NOT MATCHING TERMINAL)", TerminalStrings[ INDEX_TERMINAL(top->symbol) ], TerminalStrings[ INDEX_TERMINAL(lookahead.token) ] );
// 				top->linenumber = lookahead.linenumber; // Copy the linenumber of the Unexpected Symbol
// 				// Initialize with default values
// 				switch( top->symbol ) { 
// 					case T_NUM:	top->terminal.lexeme._int = 0; 	break;
// 					case T_RNUM: top->terminal.lexeme._real = 0.0f;	break;
// 					case T_ID:	top->terminal.lexeme._id = "invalidID" ; break;
// 					case T_FUNID: top->terminal.lexeme._id = "invalidFUNID" ; break;
// 					case T_STR: top->terminal.lexeme._str = "\"invalid string\"";	break;
// 				}
// 				popParserStack(); // Pop the top of stack
// 				// NOTE: LOOKAHEAD Not Updated ..
// 				//lookahead = getNextToken();
// 				top = topParserStack(); // Update the top of stack
// 				// EXIT("TERMINAL ON TOP OF STACK NOT MATCHING WITH TERMINAL IN THE INPUT STREAM");
// 			}
// 		}
// 		else {
// 			// If non-terminal on top
// 			if ( parsetable[top->symbol][INDEX_TERMINAL(lookahead.token)] == NULL ) {
// 				// If no RULE in the ParseTable
// 				PT_MSG("TOP (non-terminal): %s lookahead: %s (NOT MATCHING NON-TERMINAL)", 
// 				NonTerminalStrings[top->symbol], TerminalStrings[ INDEX_TERMINAL(lookahead.token) ] );
// 				// PANIC MODE
// 				//REPORT_PARSER_ERROR(lookahead.linenumber,"Unexpected %s '%s'. Expecting a derivation of %s.",TerminalStrings[INDEX_TERMINAL(lookahead.token)],TerminalSymbols[INDEX_TERMINAL(lookahead.token)],NonTerminalStrings[top->symbol]);
// 				REPORT_PARSER_ERRORex2(top,lookahead);
// 				// First Check if given symbol can derive NULL then derive it ...
// 				// if (  CONTAINS_NULL( grammar[top->symbol].FIRST) ) {
// 				// 	PT_MSG("TOP (non-terminal): %s derives NULL so using that production.",NonTerminalStrings[top->symbol]);
// 				// 	// Update the linenumber with the TOKEN
// 				// 	top->linenumber = lookahead.linenumber; // Copy the linenumber
// 				// 	// Use it
// 				// 	// First find that production...
// 				// 	for ( int i= 0 ; i < grammar[top->symbol].numrules ; i++) {
// 				// 		RULE * rule = &(grammar[top->symbol].rules[i]);
// 				// 		ulong mask = grammar[top->symbol].rules[i].FIRST ;
// 				// 		if ( CONTAINS_NULL( mask) ) {
// 				// 			// Pop the stack and Apply That Rule..
// 				// 			/////////////////////////////
// 				// 			addruletostack(rule);
// 				// 			/////////////////////////////////
// 				// 			break;
// 				// 		}
// 				// 	}
// 				// 	// After Adding that RULE Update the top of stack
// 				// 	top = topParserStack();// Update the top of stack
// 				// 	continue; // Don't Look Ahead loop back
// 				// }
// 				//PT_MSG("TOP (non-terminal): %s does not derive a NULL production. Skipping through input symbols to find a match.",NonTerminalStrings[top->symbol]);
// 				// If Not containing NULL production
// 				// Then Check With First and Follow ...
// 				// Synchronizing Set of Non-terminal initially INCLUDES elements in the FIRST and FOLLOW set
// 				// Later add elements to the Synchronizing Set the Synchronizing Set of Higher Constructs (Resync Set Later)
// 				while (true) {
// 					// Get the Next Element From The Stream ...
// 					//lookahead = getNextToken(); // The Next Token (Should it be at the first ?)
// 					if (lookahead.token == T_EOF) {
// 						top->linenumber = lookahead.linenumber; // Set the linenumber of current symbol as well
// 						if (IS_NON_TERMINAL(top->symbol)) { /////////// <-     MODIFIED ADDED Needed ?
// 								// If top of stack is non-terminal set it to NULL
// 								top->nonterminal.child = NULL ; // No children ..
// 								PT_MSG("TOP (non-terminal): %s Forced to NULL", NonTerminalStrings[top->symbol]);
// 						}
// 						else  PT_MSG("TOP (terminal): %s Popped ", TerminalStrings[INDEX_TERMINAL(top->symbol)]);
// 						// If end of Input STREAM
// 						while (!emptyParserStack()) {
// 							popParserStack();
// 							top = topParserStack();
// 							top->linenumber = lookahead.linenumber;
// 							if (IS_NON_TERMINAL(top->symbol)) {
// 								// If top of stack is non-terminal set it to NULL
// 								top->nonterminal.child = NULL ; // No children ..
// 								PT_MSG("TOP (non-terminal): %s  Forced to NULL", NonTerminalStrings[top->symbol]);
// 							}
// 							else  PT_MSG("TOP (terminal): %s Popped ", TerminalStrings[INDEX_TERMINAL(top->symbol)]);
// 						} 
// 						break;
// 					}
// 					if ( CONTAINS_TERMINAL( grammar[top->symbol].FIRST,lookahead.token) ) {
// 						PT_MSG("TOP (non-terminal): %s derives %s. Using that production.",NonTerminalStrings[top->symbol], TerminalStrings[ INDEX_TERMINAL(lookahead.token) ] );
// 						// If contained in first set then expand with appropriate RULE
// 						// Update the linenumber of TOP ..
// 						top->linenumber = lookahead.linenumber; // Copy the linenumber
// 						// Use it
// 						// Now find the corresponding RULE... 
// 						// One Option is to use parsetable or other is Too Look through rules
// 						// Since It's Error Recovery (Not Part of Successful compilation)
// 						// Obtain the RULE
// 						RULE * rule = parsetable[top->symbol][INDEX_TERMINAL(lookahead.token)];
// 						// Pop the stack and Apply That Rule..
// 						addruletostack(rule);
// 						// After Adding that RULE Update the top of stack
// 						top = topParserStack();// Update the top of stack
// 						break; // break out of the recovery loop
// 					}
// 					else if ( CONTAINS_TERMINAL( grammar[top->symbol].FOLLOW, lookahead.token) ) {
// 						// FIRST CHECK IF GIVEN SYMBOL DERIVES NULL THEN USE IT INSTEAD OF FORCING

// 						//////////////////////////////////////////
// 						if (  CONTAINS_NULL( grammar[top->symbol].FIRST) ) {
// 							PT_MSG("TOP (non-terminal): %s does not derive %s but FOLLOW set contains it. Also it derives NULL so using that production.",NonTerminalStrings[top->symbol],TerminalStrings[ INDEX_TERMINAL(lookahead.token)]);
// 							// Update the linenumber with the TOKEN
// 							top->linenumber = lookahead.linenumber; // Copy the linenumber
// 							// Use it
// 							// First find that production...
// 							for ( int i= 0 ; i < grammar[top->symbol].numrules ; i++) {
// 								RULE * rule = &(grammar[top->symbol].rules[i]);
// 								ulong mask = grammar[top->symbol].rules[i].FIRST ;
// 								if ( CONTAINS_NULL( mask) ) {
// 									// Pop the stack and Apply That Rule..
// 									/////////////////////////////
// 									addruletostack(rule);
// 									/////////////////////////////////
// 									break;
// 								}
// 							}
// 							// After Adding that RULE Update the top of stack
// 							top = topParserStack();// Update the top of stack
// 							break; // Break out of the loop
// 						}
// 						/////////////////////////////////////////
// 						PT_MSG("TOP (non-terminal): %s does not derive %s but FOLLOW set contains it. Forcing Current Production to NULL.",NonTerminalStrings[top->symbol], TerminalStrings[ INDEX_TERMINAL(lookahead.token) ] );
					
// 						// If contained in follow set then POP the current Symbol 	
// 						// Update the linenumber of TOP ..
// 						top->linenumber = lookahead.linenumber; // Copy the linenumber
// 						// Set the derivation to NULL
// 						top->nonterminal.child = NULL ;
// 						// Pop That TOP of stack with ERROR warning then Proceed...
// 						popParserStack(); 
// 						top = topParserStack();
// 						break;
// 					}
// 					PT_MSG("TOP (non-terminal): %s does not derive %s and also not in FOLLOW set as well. Skipping it.",NonTerminalStrings[top->symbol], TerminalStrings[ INDEX_TERMINAL(lookahead.token) ] );
// 					lookahead = getNextToken(); // Add it at the bottom and checking
// 				}

// 				// EXIT("No RULE in the PARSE TABLE");
// 			}
// 			else {
// 				PT_MSG("TOP (non-terminal): %s lookahead: %s (MATCHING NON-TERMINAL)", 
// 				NonTerminalStrings[top->symbol], TerminalStrings[ INDEX_TERMINAL(lookahead.token) ] );
				
// 				// Update the linenumber of the non-terminal on top of the stack
// 				top->linenumber = lookahead.linenumber; // Copy the linenumber
// 				// Obtain the RULE
// 				RULE * rule = parsetable[top->symbol][INDEX_TERMINAL(lookahead.token)];
				
// 				// PRINT THE RULE
// 				int size = rule->size ;
// 				PT_PRINT1("%s (%s) -> ", NonTerminalStrings[rule->symbol],NonTerminalStrings[top->symbol]);
// 				if (size == 0) PT_PRINT(" -null- ");
// 				for (int i=0;i < size; i++) 
// 					if (IS_TERMINAL( rule->symbols[i] )) PT_PRINT(" %s ", TerminalStrings[INDEX_TERMINAL(rule->symbols[i])] );
// 					else PT_PRINT(" %s ", NonTerminalStrings[rule->symbols[i]] );
// 				PT_PRINT("\n");

// 				// Pop that Symbol and Add that RULE to top of stack 
// 				addruletostack(rule);
// 				// UPDATE the TOP of stack
// 				top = topParserStack();
// 			}
// 		} // Check if more elements in the INPUT
// 	}

// 	// Now stack empty Last step check if INPUT is empty
// 	//lookahead = getNextToken();
// 	while ( lookahead.token != T_EOF ) {
// 		// <--------------- MODIFIED  COMPLETE IT !!!
// 		PT_MSG("Unexpected Tokens after EOF");
// 		REPORT_PARSER_ERRORex3(lookahead);
// 		lookahead = getNextToken();
// 	}


// 	//printparsetreenode(PTRoot,NULL);
// 	printparsetree();
// 	finalizeLexer();
// 	return 0;
// }
// 	initializeLexer(argv[1]);
// 	Token token  ;
// 	do {
// 		token = getNextToken();
// 		printTokenInfo(token);
// 		//printf("linenumber: %d token: %d\n",linenumber, token.token);
// 	}while (token.token != T_EOF) ;
// 	finalizeLexer();