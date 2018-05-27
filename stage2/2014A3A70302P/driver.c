/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 2
	Driver with all options ...
*/

#include "parser.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "lexer.h"
#include "abstractnstableDef.h"
#include "stable.h"
#include "abstractDef.h"
#include "intermediatecode.h"
#define LOGO "|_     _|    _   _   _  |_      _ _          |             _   _\n|     | | | | | |_| |_  | |    |  _| |\\ /|   |/ | | |\\ /|  _| |\n|_    |_| | | | |_   _| | |    | |_| | | |   |\\ |_| | | | |_| |\n _  _        _  _   _  _   _  _  _  _   _\n _|| || |_| |_| _| |_|  / | | _|| | _| |_| \n|_ |_||   | | | _| | | |  |_| _||_||_  |\n"
int main(int argc, char *argv[])
{
	if ( argc != 3 ){
		printf("FORMAT: executable inputcode outputasm\n");
		return -1;
	}
	char *program = argv[1]; 
	char *output = argv[2];

	int option ;
	int ptcount = 0, astcount = 0;
	bool parsed = false;
	bool succparsed = false;
	bool succstable = false;
	bool succcode = false;
	PTPNode root;
	ASTStmt mainprogram;
	printf("\033[035m%s\033[0m\n",LOGO);
	printf("\033[031mT DINESH RAM KUMAR\n2014A3A70302P\nCOMPILER CONSTRUCTION\033[0m\n\n");
	printf("\033[032mLEVEL 4: All modules work (AST, Symbol Table, Type Checking and Semantic Analysis, IR and Code Generation).\nNote: Type Checking and Semantic Analysis are together, IR and Code Generation are together.\033[0m\n");
	while ( 1 ){
			printf("\n");
			printf(
				"Select one of the options:\n"
				"[\033[036m0\033[0m] Exit Driver\n"
				"[\033[036m1\033[0m] Print Token List\n"
				"[\033[036m2\033[0m] Verify Syntactic Correctness and print parse tree\n"
				"[\033[036m3\033[0m] Print Abstract Syntax Tree \n"
				"[\033[036m4\033[0m] Print Node Ratio\n"
				"[\033[036m5\033[0m] Print Symbol Table\n"
				"[\033[036m6\033[0m] Verify Sematic Correctness\n"
				"[\033[036m7\033[0m] Generate Assembly Code\n"
			//	"[\033[036m8\033[0m] Print Comment Free Code\n"
				// "8. Execute Assembly Code\n"
			);
			printf("\033[032mENTER OPTION\033[0m > ");
			char inputbuffer[BUFSIZ];
			fgets(inputbuffer, BUFSIZ, stdin);
			option = ( sscanf(inputbuffer,"%d",&option) == 0 ) ? 0 : option ; // obtain the option
			fflush(stdin); // remove other symbols 
			//fseek(stdin,0,SEEK_END);
			switch ( option ) {
				case 1:
					printtokenlist(program);
					break;
				case 2:
					parsed = true;
					succparsed = parseInputFile(program);
					printparsetree();
					printprogramfromparsetree();
					root =  getparsetree(); 
					ptcount = countparsetreenode( root );
					if ( succparsed == true ) {
						createASTnode(root, &mainprogram);
						astcount = countASTStmt( &mainprogram );
						printf("\033[032mSuccessful parsing.\033[0m\n");
					}
					else {
						printf("\033[031mError in parsing.\033[0m\n");
					}
					deleteparsetree(); // delete the parse tree ...
					break;
				case 3:
					if ( !parsed ) {
						parsed = true;
						succparsed = parseInputFile(program);
						root =  getparsetree(); 
						ptcount = countparsetreenode( root );
						if ( succparsed == true ) {
							createASTnode(root, &mainprogram);
							astcount = countASTStmt( &mainprogram );
							printf("\033[032mSuccessful parsing.\033[0m\n");
						}
						// printf("\033[036mEnter option 2 First.\033[0m\n");
					}
					if  ( !succparsed )
						printf("\033[031mError in parsing.\033[0m\n");
					else 
						printAST( &mainprogram ); // then print the ast ...
					break;
				case 4:
					if ( !parsed ) {
						parsed = true;
						succparsed = parseInputFile(program);
						root =  getparsetree(); 
						ptcount = countparsetreenode( root );
						if ( succparsed == true ) {
							createASTnode(root, &mainprogram);
							astcount = countASTStmt( &mainprogram );
							printf("\033[032mSuccessful parsing.\033[0m\n");
						}
						// printf("\033[036mEnter option 2 First.\033[0m\n");
					}
					
					if ( !succparsed )
						printf("\033[031mError in parsing.\033[0m\n");
					else {
						printf("PT NODE COUNT: \033[032m%d\033[0m\nAST NODE COUNT: \033[032m%d\033[0m\n", ptcount, astcount);
						printf("NODE COMPRESSION PERCENT: \033[032m%f\033[0m\n", ((float) ptcount - astcount)/( (float) ptcount)  ) ;
						printf("\033[034mAST and PT use different type of nodes. AST contains more attributes for semantics and code generation. Size comparison is not valid.\033[0m\n");
					}
					break;
				
				case 5:
					if ( !parsed ) {
						parsed = true;
						succparsed = parseInputFile(program);
						root =  getparsetree(); 
						ptcount = countparsetreenode( root );
						if ( succparsed == true ) {
							createASTnode(root, &mainprogram);
							astcount = countASTStmt( &mainprogram );
							printf("\033[032mSuccessful parsing.\033[0m\n");
						}
						// printf("\033[036mEnter option 2 First.\033[0m\n");
					}
					
					if ( !succparsed )
						printf("\033[031mError in parsing.\033[0m\n");
					else if ( !succstable )
						printf("\033[036mEnter option 6 First.\033[0m\n");
					else
						printAllSTable(mainprogram.func_def.stable);
					break;
				case 6:
					if ( !parsed ) {
						parsed = true;
						succparsed = parseInputFile(program);
						root =  getparsetree(); 
						ptcount = countparsetreenode( root );
						if ( succparsed == true ) {
							createASTnode(root, &mainprogram);
							astcount = countASTStmt( &mainprogram );
							printf("\033[032mSuccessful parsing.\033[0m\n");
						}
						// printf("\033[036mEnter option 2 First.\033[0m\n");
					}
					if ( !succparsed )
						printf("\033[031mError in parsing.\033[0m\n");
					else {
						succstable = true ;
						if ( createandpopulatehashtables(&mainprogram, 1, NULL) ) {
							printf("\033[032mSuccessful in type checking and semantics.\033[0m\n");
						}
						else {
							printf("\033[031mError in type checking and semantics.\033[0m\n");
							printf("NOTE 1: Initialization of checked using an avoidance scheme.\n");
							printf("NOTE 2: Matrix passed within function, size is defined from arithmatic expression. So some operator must be of known size.\n");
						}
					}
					break;
				case 7:
					parsed = true;
					succparsed = parseInputFile(program);
					root =  getparsetree(); 
					ptcount = countparsetreenode( root );
					if ( succparsed == true ) {
						printf("\033[032mSuccessful parsing.\033[0m\n");
						createASTnode(root, &mainprogram);
						astcount = countASTStmt( &mainprogram );
						deleteparsetree(); // delete the parse tree ...
						if ( createandpopulatehashtables(&mainprogram, 1, NULL) ) {
							// generate Intermediate code (stores as part of AST node structre as attributes)
							printf("\033[032mSuccessful in type checking and semantics.\033[0m\n");
							addalltemps( &mainprogram );
							FILE *fd = fopen(output ,"w");
							if ( fd == NULL ) {
							 	perror("\033[031mUnable to open output file.\033[0m\n");
							 	return -1;
							}
							generateNASMCode( &mainprogram, fd );
							succcode = true ;
							printf("\033[032mSuccessful in code generation.\033[0m\n");
							printf("To compile and run the code. Use:\n \033[033m nasm -felf64 -o out.o %s && gcc out.o -no-pie -o out && ./out\033[0m\n", output);
						}
						else {
							printf("\033[031mError in type checking and semantics.\033[0m\n");
						}
					}
					else {
						printf("\033[031mError in parsing.\033[0m\n");
						deleteparsetree(); // delete the parse tree ...
					}
					break;
				// case 8:
				// 	if ( !succcode )
				// 		printf("Enter option 7 first.\n");
				// 	else {
				// 		char command[BUFSIZ];
				// 		sprintf(command, "nasm -felf64 -o assembly.o %s", output);
				// 		system(command);
				// 		sprintf(command, "gcc assembly.o -no-pie -o assembly");
				// 		system(command);
				// 		sprintf(command, "./assembly", output);
				// 		system(command);
				// 	}
				// 	break;
				// case 8:
				// 	commentfreecode(program);
				// 	break;
				case 0:
					printf("Exiting.\n");
					return 0;
				default:
					printf("\033[031mInvalid Option.\033[0m\n");
					// exit(EXIT_FAILURE);
			}
	}

	return 0;
}
/*
int main(int argc, char *argv[]) {
	if ( argc != 3 ){
		printf("FORMAT: executable inputcode outputasm\n");
		return -1;
	}
	commentfreecode(argv[1]);
	bool parsed = parseInputFile(argv[1]);
	printprogramfromparsetree();
	if ( parsed ){
		printf("Successful in Parsing...\n");
		ASTStmt mainprogram;
		PTPNode PTRoot = getparsetree();
		createASTnode(PTRoot, &mainprogram);
		deleteparsetree();
		for ( int i =0 ; i < 20; i ++)
			printf("===");
		printf("\nPRINTING AST: \n");
		for ( int i =0 ; i < 20; i ++)
			printf("===");
		printf("\n");
		printAST(&mainprogram);
		if ( createandpopulatehashtables(&mainprogram, 1, NULL) ) {
			// Successfully created hash table and types are valid ...
					//	printf("Sucess...\n");
			printAllSTable(mainprogram.func_def.stable);
			// addtempvariablesallstmts( &mainprogram, NULL);
			 addalltemps( &mainprogram );
			 printf("AFTER ADDING NEW SYMBOLS..\n");
			 printAllSTable(mainprogram.func_def.stable);
			 FILE *fd = fopen(argv[2],"w");
			 if ( fd == NULL ) {
			 	perror("Unable to open output file.\n");
			 	return -1;
			 }
			 generateNASMCode( &mainprogram, fd );
		}
		else {
			printAllSTable(mainprogram.func_def.stable);
			printf("Correct the errors...\n");
		}
		// Print the hash table now ...
		
	}
	else {
		printf("Failed parsing..\n");
		
	}
	printf("\033[034mUnInitialized variables is a warning. Variable may or may not be initialized. It follows a avoidance of UnInitialized variables not detection (Partial detection..).\033[0m\n");
	printf("MATRIX SIZES in function causes errors if some variable involved anywhere must have sizes ....\n");
	return 0;
}
*/