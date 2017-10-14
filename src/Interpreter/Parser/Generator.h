#ifndef __GENERATOR_H__
#define __GENERATOR_H__

#include <stack>

#include "../Lexer/Lexer.h"
#include "ASTree.h"
#include "ParserSymbol.h"

#define GENERATOR_CNT 24

namespace Generator{
    class ASTgenerator{
    public:
        ASTgenerator(){};
        virtual ~ASTgenerator(){};
        virtual ParserSymbol::SLRstate 
                Accept(TokenStream & token_stream, ASTNodeStack & s) = 0;
    };


    class wait_select:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 1;
    };

    class wait_attr_id:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 2;
    };

    class reduce_attr_id:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 3;
    };

    class wait_addr_dot_right:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 4;
    };

    class reduce_attr_id_with_table_id:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 5;
    };

    class reduce_attr:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 6;
    };

    class wait_attr_alias:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 7;
    };

    class reduce_attr_with_alias:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 8;
    };

    class reduce_attr_set:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 9;
    };

    class wait_from:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 10;
    };

    class wait_table_id:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 11;
    };

    class reduce_table_id:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 12;
    };

    class reduce_table:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 13;
    };

    class wait_table_alias:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 14;
    };

    class reduce_table_with_alias:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 15;
    };

    class reduce_table_set:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 16;
    };

    class wait_where:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 17;
    };

    class wait_condition:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 18;
    };

    class wait_num_or_str:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 19;
    };

    class wait_equality:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 20;
    };

    class reduce_condition:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 21;
    };

    class reduce_condition_set:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 22;
    };

    class reduce_query:public ASTgenerator{
    public:
        ParserSymbol::SLRstate Accept(TokenStream & token_stream, ASTNodeStack & s);
        static const int bind_state = 23;
    };
};



#endif