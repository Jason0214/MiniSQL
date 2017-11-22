#ifndef __LEXER_H__
#define __LEXER_H__

#include "Token.h"
#include "TokenProto.h"
#include <list>

#define PROTO_NUM 10

class TokenStream{
public:
    TokenStream(){}
    ~TokenStream(){}
    Token pop_front(){
        Token ret = this->token_list_.front();
        this->token_list_.pop_front();
        return ret;
    }
    const Token & front() const{
        return this->token_list_.front();
    }
    void push_back(const Token & t){
        this->token_list_.push_back(t);
    }
    int size() const{
        return (int)(this->token_list_.size());
    }

    std::list<Token> token_list_;
};

class Lexer{
public:
    Lexer();
    ~Lexer();

    void loadText(std::string & raw_text);
    void clear(){
      this->result.token_list_.clear();
    }
    
    TokenStream result;
private:
    Lexer(const Lexer &);
    const Lexer & operator =(const Lexer &);

    void toLower(std::string & text);

    std::string getErrInfo(int beg_index, const std::string & raw_text);
    TokenProto* token_protos[PROTO_NUM];
};

#endif