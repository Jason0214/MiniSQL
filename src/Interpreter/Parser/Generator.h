#ifndef __GENERATOR_H__
#define __GENERATOR_H__

#include "ParserSymbol.h"

namespace Generator{
    class ASTgenerator{
    public:
        ASTgenerator(){};
        virtual ASTgenerator(){};
        virtual SLRstate Accept(TokenStream & token_stream, stack<ASTreeNode*> & s) = 0;
    };


    class wait_select:public ASTgenerator{
    public:
        SLRstate Accept(TokenStream & token_stream, stack<ASTreeNode*> & s);
        const int bind_state = 1;
    };

    class wait_attr_set:public ASTgenerator{
    public:
        SLRstate Accept(TokenStream & token_stream, stack<ASTreeNode*> & s);
        const int bind_state = 2;
    };

    class reduce_attr_id:public ASTgenerator{
    public:
        SLRstate Accept(TokenStream & token_stream, stack<ASTreeNode*> & s);
        const int bind_state = 3;
    }

    class wait_addr_dot_right:public ASTgenerator{
    public:
        SLRstate Accept(TokenStream & token_stream, stack<ASTreeNode*> & s);
        const int bind_state = 4;
    };

    class reduce_attr_id_with_table_id:public ASTgenerator{
    public:
        SLRstate Accept(TokenStream & token_stream, stack<ASTreeNode*> & s);
        const int bind_state = 5;
    };

    class reduce_attr:public ASTgenerator{
    public:
        SLRstate Accept(TokenStream & token_stream, stack<ASTreeNode*> & s);
        const int bind_state = 6;
    };

    class wait_attr_alias:public ASTgenerator{
    public:
        SLRstate Accept(TokenStream & token_stream, stack<ASTreeNode*> & s);
        const int bind_state = 7;
    };

    class reduce_attr_with_alias:public ASTgenerator{
    public:
        SLRstate Accept(TokenStream & token_stream, stack<ASTreeNode*> & s);
        const int bind_state = 8;
    };

    class reduce_attr_set:public ASTgenerator{
    public:
        SLRstate Accept(TokenStream & token_stream, stack<ASTreeNode*> & s);
        const int bind_state = 9;
    };

    class wait_attr_set_again:public ASTgenerator{
    public:
        SLRstate Accept(TokenStream & token_stream, stack<ASTreeNode*> & s);
        const int bind_state = 10;
    };

    class reduce_attr_set_again:public ASTgenerator{
    public:
        SLRstate Accept(TokenStream & token_stream, stack<ASTreeNode*> & s);
        const int bind_state = 11;
    };

    class wait_from:public ASTgenerator{
    public:
        SLRstate Accept(TokenStream & token_stream, stack<ASTreeNode*> & s);
        const int bind_state = 12;
    };

    class wait_table:public ASTgenerator{
    public:
        SLRstate Accept(TokenStream & token_stream, stack<ASTreeNode*> & s);
        const int bind_state = 13;
    };

    class reduce_table:public ASTgenerator{
    public:
        SLRstate Accept(TokenStream & token_stream, stack<ASTreeNode*> & s);
        const int bind_state = 14;
    };



}



#endif