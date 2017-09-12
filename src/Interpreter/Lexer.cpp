#include "Lexer.h"
#include <regex.h>
#include <string>

using namespace std;

Lexer::Lexer(){
    this->token_protos[0] = new TokenProto(NONE, TokenProto::SPACE_PATTERN);
    this->token_protos[1] = new TokenProto(STR_TOKEN , TokenProto::DOUBLE_QUOTE_STR_PATTERN);
    this->token_protos[2] = new TokenProto(STR_TOKEN , TokenProto::SINGLE_QUOTE_STR_PATTERN);
    this->token_protos[3] = new TokenProto(FLOAT_TOKEN , TokenProto::FLOAT_PATTERN);
    this->token_protos[4] = new TokenProto(INT_TOKEN , TokenProto::INT_PATTERN);
    this->token_protos[5] = new TokenProto(META_TOKEN, TokenProto::DB_META_PATTERN);
    this->token_protos[6] = new TokenProto(DEFAULT, TokenProto::DEFAULT_PATTERN);
}

Lexer::~Lexer(){
    for(int i = 0; i < PROTO_NUM; i++){
        delete this->token_protos[i];
    }
}

void Lexer::loadText(const string & raw_text){
    int scan_index = 0;
    while(scan_index < raw_text.size()){
        // denote the match length
        int match_offset = 0;

        // tmp pointer to token prototype
        TokenProto* tmp_ptr = NULL;

        // try to match token with token prototype by pirority
        for(int i = 0; i < PROTO_NUM; i++){
            tmp_ptr = this->token_protos[i];
            match_offset = tmp_ptr->regexMatch(&(raw_text.c_str()[scan_index]));
            if(match_offset != 0){
                // if matched and prototype is not space, add to result
                if(tmp_ptr->target_token_type != NONE){
                    this->result.push_back(Token(tmp_ptr->target_token_type, 
                                raw_text.substr(scan_index, match_offset)));
                }
                scan_index += match_offset;
                break;
            }
        }
        if(match_offset == 0){
            this->setErrInfo(scan_index, raw_text);
            throw LexingError();
        }
    }
}

void Lexer::setErrInfo(int beg_index, const string & raw_text){
    regex_t p_err_reg;
    size_t nmatch = 1;
    regmatch_t pmatch[1];
    regcomp(&p_err_reg, "^\\s*([^\\s]+)", REG_EXTENDED);
    regexec(&p_err_reg, &(raw_text.c_str()[beg_index]), nmatch, pmatch, 0);
    this->error_info = raw_text.substr(beg_index+pmatch[0].rm_so, pmatch[0].rm_eo - pmatch[0].rm_so);
    regfree(&p_err_reg);
}