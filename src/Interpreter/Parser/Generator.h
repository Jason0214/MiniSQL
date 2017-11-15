#ifndef __QUERY_GENERATOR_H__
#define __QUERY_GENERATOR_H__

#include <stack>

#include "Generator.h"
#include "../Lexer/Lexer.h"
#include "ASTree.h"
#include "ParserSymbol.h"

#define QUERY_STATE_CNT 25

namespace QueryGenerator{


    class wait_select:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 1;
    };

    class wait_attr_id:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 2;
    };

    class reduce_attr_id:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 3;
    };

    class wait_addr_dot_right:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 4;
    };

    class reduce_attr_id_with_table_id:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 5;
    };

    class reduce_attr:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 6;
    };

    class wait_attr_alias:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 7;
    };

    class reduce_attr_with_alias:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 8;
    };

    class reduce_attr_set:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 9;
    };

    class wait_from:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 10;
    };

    class wait_table_id:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 11;
    };

    class reduce_table_id:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 12;
    };

    class reduce_table:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 13;
    };

    class wait_table_alias:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 14;
    };

    class reduce_table_with_alias:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 15;
    };

    class reduce_table_set:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 16;
    };

    class wait_where:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 17;
    };

    class reduce_query_without_condition:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 18;
    };

    class wait_condition:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 19;
    };

    class wait_num_or_str:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 20;
    };

    class wait_equality:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 21;
    };

    class reduce_condition:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 22;
    };

    class reduce_condition_set:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 23;
    };

    class reduce_query_with_condition:public QueryGenerator{
    public:
        ParserSymbol::QueryState Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 24;
    };
};



#endif