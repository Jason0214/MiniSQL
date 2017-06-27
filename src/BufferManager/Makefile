CC = g++
OBJS = BM_test.o BufferManager.o Block.o
OUT = test.exe
FLAGS = -g
OUT: $(OBJS)
	$(CC) $(OBJS) $(FLAGS) -o $(OUT)
%.o: %.cpp
	$(CC) $< -c $(FLAGS) -o $@  

.PHONY:
clean: 
	rm ./*.o