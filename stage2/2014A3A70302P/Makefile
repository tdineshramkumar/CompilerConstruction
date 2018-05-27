# T Dinesh Ram Kumar
# 2014A3A70302P
# Compiler Submission Stage 2

compiler: driver.c lexer.o parser.o parserStack.o symbolhashs.o commentfree.o symboltable.o intermediatecode.c ast.c semanticanalysis.c
	gcc -o compiler driver.c ast.c semanticanalysis.c intermediatecode.c intermediatecode.h lexer.o parser.o symboltable.o parserStack.o symbolhashs.o commentfree.o -I ./ -g

lexer.o: lexer.c lexer.h lexerDef.h
	gcc -c lexer.c lexer.h lexerDef.h -I ./ -g

commentfree.o: commentfree.c lexer.h lexerDef.h
	gcc -c commentfree.c lexer.c lexerDef.h -I ./ -g

parser.o: parser.c
	gcc -c parser.c parser.h stable.h parserDef.h parserStack.h symbolhashs.h -I ./ -g

symboltable.o: symboltable.c stable.h
	gcc -c symboltable.c stable.h -I ./ -g

parserStack.o: parserStack.c
	gcc -c parserStack.c parserStack.h parserDef.h -g

symbolhashs.o: symbolhashs.c
	gcc -c symbolhashs.c -I ./ -g

clean:
	rm compiler test.asm out *.o *.gch -rf
