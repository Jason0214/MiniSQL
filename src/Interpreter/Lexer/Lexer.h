#include "Token.h"
#include "TokenProto.h"
#include <list>

#define PROTO_NUM 10

typedef std::list<Token> TokenStream;

class LexingError{};

class Lexer{
public:
    Lexer();
    ~Lexer();

    void loadText(const std::string & raw_text);
    
    TokenStream result;
    std::string error_info;
private:
    void setErrInfo(int beg_index, const std::string & raw_text);
    TokenProto* token_protos[PROTO_NUM];
};