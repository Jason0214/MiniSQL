CC = g++
CFLAGS = -std=c++11 -Wall  -g

test_lexer: Lexer.o test_lexer.o Token.o TokenProto.o
	$(CC) Lexer.o test_lexer.o Token.o TokenProto.o $(CFLAGS) -o test_lexer

%.o: %.cpp
	$(CC) -c $(CFLAGS) $<  -o $@

.PHONY:
clean: 
	rm *.o