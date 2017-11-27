#ifndef MINISQL_INTERPRETER_H
#define MINISQL_INTERPRETER_H

#include <iostream>
#include "../EXCEPTION.h"
#include "./Lexer/Lexer.h"
#include "./Parser/Parser.h"
#include "./Executor/Executor.h"



class Interpreter {
public:
    Interpreter(){}
    ~Interpreter(){}
    void run(std::string sentence){
        try{
            this->lexer_.loadText(sentence);
            this->parser_.parseSentence(this->lexer_.result);
            this->executor_.run(this->parser_.getASTree());
        }
        catch (Exception & e){
            std::cout << e.err << std::endl;
        }
        this->parser_.clear();
        this->lexer_.clear();
    }
private:
    Lexer lexer_;
    Parser parser_;
    Executor executor_;
};

#endif //MINISQL_INTERPRETER_H
