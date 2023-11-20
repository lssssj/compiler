bison -d parser.y
flex -o lexer.cpp lexer.l

clang++ -fstandalone-debug -g -o test lexer.cpp parser.tab.c 
