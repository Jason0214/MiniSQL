#ifndef MINISQL_TOKEN_H
#define MINISQL_TOKEN_H

#include <string>


class Token{
public:
    typedef enum{
        NONE,
        INTS,
        FLOATS,
        STR,
        SYMBOL,
        KEYWORD,
        EQUALITY
    }TokenType;

    Token();
    Token(TokenType type, const std::string & raw_token);
    Token(const Token & left_v){
        this->type = left_v.type;
        this->content = left_v.content;
    }
    Token & operator=(const Token & left_v){
        if(this != &left_v){
            this->type = left_v.type;
            this->content = left_v.content;
        }
        return *this;
    }
    ~Token(){}
    TokenType type;
    std::string content;
};

#endif
