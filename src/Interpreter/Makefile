CC = g++
CFLAGS = -std=c++11 -Wall  -g

TARGET = test
OBJS = Lexer/Lexer.o Lexer/Token.o Lexer/TokenProto.o \
		Parser/Parser.o Parser/Reducer.o Parser/ASTree.o \
		Parser/Generator.o test.o

TARGET: $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $(TARGET)

Lexer/%.o: Lexer/%.cpp
	$(CC) -c $(CFLAGS) $<  -o $@

Parser/%.o: Parser/%.cpp
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY:
clean: 
	rm Lexer/*.o Parser/*.o