#include "TokenProto.h"
#include <regex.h>
using namespace std;

const string TokenProto::INT_PATTERN = "^[0-9]+";
const string TokenProto::FLOAT_PATTERN = "^[0-9]*\\.[0-9]+";
const string TokenProto::SINGLE_QUOTE_STR_PATTERN = "^\"[^\"]*\"";
const string TokenProto::DOUBLE_QUOTE_STR_PATTERN = "^\'[^\']*\'";
const string TokenProto::SPACE_PATTERN = "^[ \t\r\n]+";
const string TokenProto::DB_META_PATTERN = "^`[^`]*`";
const string TokenProto::DEFAULT_PATTERN = "^[_a-zA-Z][_a-zA-Z0-9]*";

TokenProto::TokenProto(TokenType target, const string & pattern)
:regex_pattern(pattern){
    this->target_token_type = target;
}

int TokenProto::regexMatch(const char *str){
    regex_t preg;
    size_t nmatch = 1;
    regmatch_t pmatch[1];

    regcomp(&preg, this->regex_pattern.c_str(), REG_EXTENDED);

    int error = regexec(&preg, str, nmatch, pmatch, 0);

    regfree(&preg);

    if(!error){  
        return pmatch[0].rm_eo - pmatch[0].rm_so;        
    }
    else{
        return 0;
    }
}