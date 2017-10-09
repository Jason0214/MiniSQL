#ifndef MINISQL_TOKEN_H
#define MINISQL_TOKEN_H

#include <string>


class Token{
public:
    typedef enum{
        NONE,
        IDENTIFIER,
        INTS,
        FLOATS,
        STR,
        SYMBOL,
        KEYWORD,
        EQUALITY
    }TokenType;

    Token(TokenType type = NONE);
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
