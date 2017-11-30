#include "Lexer.h"
#include <regex.h>
#include <string>

#include "../../EXCEPTION.h"

using namespace std;

Lexer::Lexer(){
    this->token_protos[0] = new TokenProto(Token::NONE, TokenProto::SPACE_PATTERN);
    this->token_protos[1] = new TokenProto(Token::STR, TokenProto::DOUBLE_QUOTE_STR_PATTERN);
    this->token_protos[2] = new TokenProto(Token::STR, TokenProto::SINGLE_QUOTE_STR_PATTERN);
    this->token_protos[3] = new TokenProto(Token::IDENTIFIER, TokenProto::IDENTIFIER_PATTERN);
    this->token_protos[4] = new TokenProto(Token::FLOATS , TokenProto::FLOAT_PATTERN);
    this->token_protos[5] = new TokenProto(Token::INTS , TokenProto::INT_PATTERN);
    this->token_protos[6] = new TokenProto(Token::SYMBOL, TokenProto::SYMBOL_PATTERN);
    this->token_protos[7] = new TokenProto(Token::EQUALITY, TokenProto::EQUALITY_PATTERN);
    this->token_protos[8] = new TokenProto(Token::KEYWORD, TokenProto::KEYWORD_PATTERN);
    this->token_protos[9] = new TokenProto(Token::IDENTIFIER, TokenProto::DEFAULT_PATTERN);
}

Lexer::~Lexer(){
    for(int i = 0; i < PROTO_NUM; i++){
        delete this->token_protos[i];
    }
}

void Lexer::toLower(std::string & text){
    for(int i = 0; i < text.size(); i++){
        if(text[i] <= 'Z' && text[i] >= 'A'){
            text[i] = text[i] - 'A' + 'a';
        }
    }
}

void Lexer::loadText(string & raw_text){
    int scan_index = 0;
    while(scan_index < raw_text.size()){
        // try to find the longest match
        int max_match_len = 0;
        int match_proto_index = 0;
        for(int i = 0; i < PROTO_NUM; i++){
            int tmp_match_len = this->token_protos[i]->regexMatch(&(raw_text.c_str()[scan_index]));
            if(max_match_len < tmp_match_len){
                max_match_len = tmp_match_len;
                match_proto_index = i;
            }
        }
        if(max_match_len == 0){
            throw LexingError(getErrInfo(scan_index, raw_text));
        }
        else{
            Token::TokenType match_type = this->token_protos[match_proto_index]->target_token_type;
            if(match_type == Token::KEYWORD){
                std::string buf = raw_text.substr(scan_index, max_match_len);
                toLower(buf);
                this->result.push_back(Token(match_type, buf));
            }
            else if(match_type != Token::NONE){
                this->result.push_back(Token(match_type, raw_text.substr(scan_index, max_match_len)));
            }
            scan_index += max_match_len;    
        }
    }
    // add an empty token at end specify the end of token stream
    this->result.push_back(Token());
}

string Lexer::getErrInfo(int beg_index, const string & raw_text){
    regex_t p_err_reg;
    size_t nmatch = 1;
    regmatch_t pmatch[1];
    regcomp(&p_err_reg, "^\\s*([^\\s]+)", REG_EXTENDED);
    regexec(&p_err_reg, &(raw_text.c_str()[beg_index]), nmatch, pmatch, 0);
    string error_info = raw_text.substr(beg_index+pmatch[0].rm_so, pmatch[0].rm_eo - pmatch[0].rm_so);
    regfree(&p_err_reg);
    return error_info;
}