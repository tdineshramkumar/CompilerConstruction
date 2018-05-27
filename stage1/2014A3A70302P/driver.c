/*
	T Dinesh Ram Kumar
	2014A3A70302P
	Compiler Submission Stage 1
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lexer.h"
#include "parser.h"
int main(int argc, char *argv[]) {
	if ( argc != 3 ) {
		printf("FORMAT: <program.exe> <code> <parsetreeoutput> \n");
		exit(EXIT_FAILURE);
	}
	char *program = argv[1]; 
	char *output = argv[2];
	printf("\n\tCompiler By: \n\t\t T Dinesh Ram Kumar\n\t\t 2014A3A70302P \n\n");
	printf("\n\tNote: First and Follow sets automated.\n\t Both lexical and syntax analyser developed. \n\t Compilers and Runs well. \n\t Generates Parse Tree Even on invalid Input Program.\n\t Does Error Recovery to an extent.\n\t It tolerates some amount of missing symbols and it can insert them.\n\n\n");
	//printtokenlist(program);
	//commentfreecode(program); 
	// initializeLexer(program);
	// printtokenlist();
	// finalizeLexer();
	bool parsingresult ;
	int option ;
	while(true) {
		printf("Enter any of the below option:\n");
		printf("1. To view comment free code\n");
		printf("2. To view list of tokens\n");
		printf("3. To parse the code and output parse tree to file.\n");
		printf("Any Other Number to Exit (No other character)\n");
		printf("OPTION: ");
		scanf("%d",&option);
		switch(option){
			case 1:
				commentfreecode(program);
				break;
			case 2:
				printtokenlist(program);
				break;
			case 3:
				parsingresult=  parseInputFile(program) ;
				if (parsingresult) printf("\033[32m Parsing Complete.. Program is syntactically correct...\033[0m \n");
				else printf("\033[31m Parsing Successful.. Program is syntactically wrong...\033[0m \n");
				printparsetreetofile(output);
				break ;
			case 4:
			default: 
				printf("Exiting ...\n");
			 	return 0;
		}
	}
	return 0;
}
