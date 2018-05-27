driver: driver.c lexer.o commentfree.o parser.o parserStack.o symbolhashs.o
	gcc -o stage1exe driver.c lexer.o commentfree.o parser.o parserStack.o symbolhashs.o -I ./

lexer.o: lexer.c lexer.h lexerDef.h
	gcc -c lexer.c lexer.h lexerDef.h -I ./

commentfree.o: commentfree.c lexer.h lexerDef.h
	gcc -c commentfree.c lexer.c lexerDef.h -I ./

parser.o: parser.c 
	gcc -c parser.c parser.h parserDef.h parserStack.h symbolhashs.h -I ./

parserStack.o: parserStack.c
	gcc -c parserStack.c parserStack.h parserDef.h

symbolhashs.o: symbolhashs.c
	gcc -c symbolhashs.c -I ./

clean:
	rm stage1exe *.o *.gch -rf
