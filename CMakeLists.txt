cmake_minimum_required(VERSION 3.8)
project(miniSQL)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        src/API/APIFunctions.cpp
        src/API/APIFunctions.h
        src/API/APIStructures.h
        src/BufferManager/Block.cpp
        src/BufferManager/Block.h
        src/BufferManager/BlockPtr.h
        src/BufferManager/BufferManager.cpp
        src/BufferManager/BufferManager.h
        src/CatalogManager/Catalog.cpp
        src/CatalogManager/Catalog.h
        src/IndexManager/BPlusTree.cpp
        src/IndexManager/BPlusTree.h
        src/IndexManager/IndexExecutor.h
        src/IndexManager/IndexManager.h
        src/Interpreter/Executor/ExeTree.cpp
        src/Interpreter/Executor/ExeTree.h
        src/Interpreter/Executor/QueryExecutor.cpp
        src/Interpreter/Executor/QueryExecutor.h
        src/Interpreter/Lexer/Lexer.cpp
        src/Interpreter/Lexer/Lexer.h
        src/Interpreter/Lexer/Token.cpp
        src/Interpreter/Lexer/Token.h
        src/Interpreter/Lexer/TokenProto.cpp
        src/Interpreter/Lexer/TokenProto.h
        src/Interpreter/Parser/ASTree.cpp
        src/Interpreter/Parser/ASTree.h
        src/Interpreter/Parser/Generator.cpp
        src/Interpreter/Parser/Generator.h
        src/Interpreter/Parser/Parser.cpp
        src/Interpreter/Parser/Parser.h
        src/Interpreter/Parser/ParserSymbol.h
        src/Interpreter/Parser/Reducer.cpp
        src/Interpreter/Parser/Reducer.h
        src/Interpreter/test.cpp
        src/RecordManager/RecordManager.cpp
        src/RecordManager/RecordManager.h
        src/RecordManager/RecordStructures.cpp
        src/RecordManager/RecordStructures.h
        src/RecordManager/Table.cpp
        src/RecordManager/Table.h
        src/RecordManager/TableIterator.cpp
        src/RecordManager/TableIterator.h
        src/CONSTANT.h
        src/EXCEPTION.h
        src/SharedFunc.cpp
        src/SharedFunc.h
        src/Interpreter/Parser/ASTreeNodeStack.h)

add_executable(miniSQL ${SOURCE_FILES})