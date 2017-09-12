#include <string>
#include "Token.h"

class TokenProto{
public:
    TokenProto(TokenType target, const std::string & pattern);
    ~TokenProto(){}
    int regexMatch(const char *str);

    TokenType target_token_type;

    static const std::string INT_PATTERN;
    static const std::string FLOAT_PATTERN;
    static const std::string SINGLE_QUOTE_STR_PATTERN;
    static const std::string DOUBLE_QUOTE_STR_PATTERN;
    static const std::string SPACE_PATTERN;
    static const std::string DB_META_PATTERN;
    static const std::string DEFAULT_PATTERN;
private:
    TokenProto(const TokenProto &);
    TokenProto & operator= (const TokenProto &);

    std::string regex_pattern;
};