#ifndef MINISQL_TOKEN_H
#define MINISQL_TOKEN_H

#include <string>

#define EMPTY_TOKEN 0x0
#define STR_TOKEN  0x1
#define INT_TOKEN  0x2
#define FLOAT_TOKEN 0x4
#define ID_TOKEN    0x8
#define KEYWORD_TOKEN 0x10

typedef int TokenType;

typedef enum{
    STR_TOKEN,
    INT_TOKEN,
    FLOAT_TOKEN,
    META_TOKEN,
    DEFAULT,
    NONE
}TokenType;


class Token{
public:
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
