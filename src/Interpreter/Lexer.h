#include "Token.h"
#include "TokenProto.h"
#include <vector>

#define PROTO_NUM 7

class LexingError{};

class Lexer{
public:
    Lexer();
    ~Lexer();
    void loadText(const std::string & raw_text);
    
    std::vector<Token> result;
    std::string error_info;
private:
    void setErrInfo(int beg_index, const std::string & raw_text);
    TokenProto* token_protos[PROTO_NUM];
};