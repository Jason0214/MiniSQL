#include "Token.h"

using namespace std;

Token::Token(TokenType type):type(type),content(""){

}

Token::Token(TokenType type, const string & raw_token):type(type){
    if(type == Token::STR || (type == Token::IDENTIFIER && raw_token[0] == '`')){
        int len = raw_token.size();
        this->content = raw_token.substr(1, len-2);
    }
    else{
        this->content = raw_token;
    }
}