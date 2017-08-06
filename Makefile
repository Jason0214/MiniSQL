CC=g++
CFLAGS= -Wall -g -std=c++11
OBJS=APIFunctions.o APIMain.o APICommands.o IO.o BufferManager.o \
	Block.o Catalog.o BPlusTree.o RecordManager.o \
	Table.o TableIterator.o SharedFunc.o

BUF_DIR=src/BufferManager/
INDEX_DIR=src/IndexManager/
CATALOG_DIR=src/CatalogManager/
API_DIR=src/API/
RECORD_DIR=src/RecordManager/

Executable: $(OBJS)
	$(CC) -o miniSQL_backend.exe $(OBJS) $(CFLAGS) 

SharedFunc.o: src/SharedFunc.cpp
	$(CC) -o SharedFunc.o -c src/SharedFunc.cpp  $(CFLAGS) 

Block.o: src/BufferManager/Block.cpp
	$(CC) -o Block.o -c src/BufferManager/Block.cpp  $(CFLAGS) 

BufferManager.o: src/BufferManager/BufferManager.cpp
	$(CC) -o BufferManager.o -c src/BufferManager/BufferManager.cpp $(CFLAGS) 

BPlusTree.o: src/IndexManager/BPlusTree.cpp
	$(CC) -o BPlusTree.o -c src/IndexManager/BPlusTree.cpp $(CFLAGS) 

Catalog.o: src/CatalogManager/Catalog.cpp
	$(CC) -o Catalog.o -c src/CatalogManager/Catalog.cpp $(CFLAGS) 

TableIterator.o: src/RecordManager/TableIterator.cpp
	$(CC) -o TableIterator.o -c  src/RecordManager/TableIterator.cpp $(CFLAGS) 

Table.o: src/RecordManager/Table.cpp
	$(CC) -o Table.o -c src/RecordManager/Table.cpp $(CFLAGS) 

RecordManager.o: src/RecordManager/RecordManager.cpp
	$(CC) -o RecordManager.o -c src/RecordManager/RecordManager.cpp $(CFLAGS) 

APIFunctions.o: src/API/APIFunctions.cpp
	$(CC) -o APIFunctions.o -c src/API/APIFunctions.cpp $(CFLAGS) 

APICommands.o: src/API/APICommands.cpp
	$(CC) -o APICommands.o -c src/API/APICommands.cpp $(CFLAGS) 

APIMain.o: src/API/APIMain.cpp
	$(CC) -o APIMain.o -c src/API/APIMain.cpp $(CFLAGS) 

IO.o: src/API/IO.cpp
	$(CC) -o IO.o -c src/API/IO.cpp $(CFLAGS) 

.PHONY:clean
clean:
	-rm *.o