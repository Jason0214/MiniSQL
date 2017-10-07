#include "Generator.h"

SLRstate Generator::wait_select::Accept(TokenStream & token_stream, stack<ASTreeNode*> & s){
    SLRstate Accept(TokenStream & token_stream, stack<ASTreeNode*> & s){
        SLRstate next_state = WAIT_ATTR_SET; 
        token_stream.pop_front();
        return next_state;
    }    
}


SLRstate Generator::wait_attr_set::Accept(TokenStream & token_stream, stack<ASTreeNode*> & s){
    Token tkn_to_eat = token_stream.pop_front();
    SLRstate = next_state;
    if(tkn_to_eat->type == Token::IDENTIFIER){
        s.push(new ASTreeNode(tkn_to_eat));
        next_state = REDUCE_ATTR;
    }
    else{
        throw ParseError(tkn_to_eat.content, "wait attr set error");
    }
    return next_state;
}

SLRstate Generator::reduce_attr_id::Accept(TokenStream & token_stream, stack<ASTreeNode*> & s){
    SLRstate next_state;
    const Token & lookahead = token_stream.front();
    //SLR
    if(lookahead->type == Token::SYMBOL && lookahead->content == "."){
        token_stream.pop_front();
        next_state = WAIT_ATTR_DOT_RIGHT;
    }
    else{
        ASTreeNode* attrIdNode = reduceAttrId(s);
        if(s.empty() || s.top()->getTag() == ParserSymbol::attr){
            // select attrID
            next_state = WAIT_ATTR_AS; //6
        }
        else if(s.top()->getTag() == attr_set || s.top()->getTag() == condition){
            next_state = WAIT_EUQALITY; //27
        }
        else if(s.top()->getTag() == attrID){
            next_state = WAIT_EUQALITY_RIGHT;
        }
        else{
            delete attrIdNode;
            throw ParseError("","reduce attr ID error");
        }
        s.push(attrIdNode);
    }
    return next_state;
}

SLRstate Generator::wait_addr_dot_right::Accept(TokenStream & token_stream, stack<ASTreeNode*> & s){
    SLRstate next_state;
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat->type == Token::IDENTIFIER){
        s.push(new ASTreeNode(tkn_to_eat));
        next_state = REDUCE_ADDR_ID_WITH_TABLE_ID;
    }
    else{
        throw ParseError(tkn_to_eat->content, "wait attr dot right value error");
    }
    return next_state;
}

SLRstate Generator::reduce_attr_id_with_table_id::Accept(TokenStream & token_stream, stack<ASTreeNode*> & s){
    SLRstate = next_state;
    ASTreeNode* attrIdWithTableID = reduceAttrIdWithTableId(s); 
    if(s.empty() || s.top()->getTag() == ParserSymbol::attr){
        // select attrID
        state = REDUCE_ATTR; //6
    }
    else if(s.top()->getTag() == attr_set || s.top()->getTag() == condition){
        state = WAIT_EUQALITY; //27
    }
    else if(s.top()->getTag() == attrID){
        state = WAIT_EUQALITY_RIGHT;
    }
    else{
        delete attrIdWithTableID;
        throw ParseError("","reduce attr ID with table ID error");
    }
    s.push(attrIdWithTableID);
    return next_state;
}

SLRstate Generator::reduce_attr::Accept(TokenStream & token_stream, stack<ASTreeNode*> & s){
    SLRstate next_state;
    const Token & lookahead = token_stream.front();
    if(lookahead->type == Token::SYMBOL && lookahead->content == "as"){
        token_stream.pop_front();
        next_state = WAIT_ATTR_ALIAS; //7
    }
    else{
        ASTreeNode* attr_node = reduceAttr(s);
        s.push(attr_node);
        next_state = REDUCE_ATTR_SET;
    }
    return next_state;
}

SLRstate Generator::wait_attr_alias::Accept(TokenStream & token_stream, stack<ASTreeNode*> & s){
    SLRstate next_state;
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat->type == Token::IDENTIFIER){
        s.push(new ASTreeNode(tkn_to_eat));
        next_state = REDUCE_ATTR_WITH_ALIAS;
    }
    else{
        throw ParseError(tkn_to_eat->content, "wait attr alias error")
    }
    return next_state;
}


SLRstate Generator::reduce_attr_with_alias::Accept(TokenStream & token_stream, stack<ASTreeNode*> & s){
    SLRstate = next_state;
    ASTreeNode* attrNode_with_alias = reduceAttrWithAlias(s);
    s.push(attr_node);
    next_state = REDUCE_ATTR_SET;
    return next_state;
}

SLRstate Generator::reduce_attr_set::Accept(TokenStream & token_stream, stack<ASTreeNode*> & s){
    SLRstate next_state;
    const Token & lookahead = token_stream.front();
    if(lookahead->type == Token::SYMBOL && lookahead->content = ","){
        token_stream.pop_front();
        next_state = WAIT_ATTR_SET_AGAIN;
    }
    else{
        ASTreeNode* attrSetNode = reduceAttrSetSingle(s);
        if(s.empty()){
            next_state = WAIT_FROM;
        }
        else if(s.top()->getTag() == ParserSymbol::attr){
            next_state = REDUCE_ATTR_SET_AGAIN;
        }
        else{
            delete attrSetNode;
            throw ParseError("", "reduce atrr set error"); 
        }
        s.push(attrSetNode);
    }
    return next_state;
}

SLRstate Generator::wait_attr_set_again::Accept(TokenStream & token_stream, stack<ASTreeNode*> & s){
/* redundant, same of state 2 */
    SLRstate = next_state;
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(tkn_to_eat));
        next_state = REDUCE_ATTR;
    }
    else{
        throw ParseError(tkn_to_eat->content, "reduce attr again error");
    }
    return next_state;
}

SLRstate Generator::reduce_attr_set_again::Accept(TokenStream & token_stream, stack<ASTreeNode*> & s){
    SLRstate next_state;
    ASTreeNode* attrSetNode = reduceAttrSetMuplti(s);
    if(s.top()->getTag() == ParserSymbol:attr){
        next_state = REDUCE_ATTR_SET_AGAIN;
    }
    else{
        next_state = WAIT_FROM;
    }
    s.push(attrSetNode);
    return next_state;
}


SLRstate Generator::wait_from::Accept(TokenStream & token_stream, stack<ASTreeNode*> & s){
    SLRstate next_state;
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat->type == Token::KEYWORD && tkn_to_eat.content == "from"){
        next_state = WAIT_TABLE;
    }
    else{
        throw ParseError("", "expect from");
    }
    return next_state;
}


SLRstate Generator::wait_table::Accept(TokenStream & token_stream, stack<ASTreeNode*> & s){
    SLRstate next_state;
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat->type == Token::SYMBOL && tkn_to_eat->content = "("){
        next_state = WAIT_SELECT;
    }
    else if(tkn_to_eat->type == Token::IDENTIFIER){
        s.push(new ASTreeNode(tkn_to_eat));
        next_state = REDUCE_TABLE;
    }
    else{
        throw ParseError("", "expect a table")
    }
    return next_state;
}

SLRstate Generator::reduce_table::Accept(TokenStream & token_stream, stack<ASTreeNode*> & s){
    SLRstate next_state = REDUCE_TABLE_SET;
    ASTreeNode tableNode = reduceTable(s);
    s.push(tableNode);
    return next_state；
}

SLRstate Generator::reduce_table_set(TokenStream & token_stream, stack<ASTreeNode*> & s){
    SLRstate next_state = WAIT_TABLE_AGAIN;
    const Token & lookahead = token_stream.top();
    
}