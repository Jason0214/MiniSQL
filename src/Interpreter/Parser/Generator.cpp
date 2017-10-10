#include "Generator.h"

#include "../../EXCEPTION.h"
#include "Reducer.h"

using namespace std;
using namespace ParserSymbol;

SLRstate Generator::wait_select::Accept(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::KEYWORD && tkn_to_eat.content == "select"){
        return WAIT_ATTR_ID;
    }
    else{
        throw ParseError(tkn_to_eat.content, "expect select");
    }
}


SLRstate Generator::wait_attr_id::Accept(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    SLRstate next_state;
    if(tkn_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(tkn_to_eat));
        next_state = REDUCE_ATTR_ID;
    }
    else{
        throw ParseError(tkn_to_eat.content, "expect attr id");
    }
    return next_state;
}

SLRstate Generator::reduce_attr_id::Accept(TokenStream & token_stream, ASTNodeStack & s){
    SLRstate next_state;
    const Token & lookahead = token_stream.front();
    //SLR
    if(lookahead.type == Token::SYMBOL && lookahead.content == "."){
        token_stream.pop_front();
        next_state = WAIT_ATTR_DOT_RIGHT;
    }
    else{
        ASTreeNode* attrIdNode = reduceAttrId(s);
        if(s.empty() || s.top()->getTag() == ParserSymbol::attr){
            // select attrID
            next_state = REDUCE_ATTR; //6
        }
        else if(s.top()->getTag() == condition){
            next_state = REDUCE_CONDITION;                
        }
        else{
            next_state = WAIT_EUQALITY;
        }
        s.push(attrIdNode);
    }
    return next_state;
}

SLRstate Generator::wait_addr_dot_right::Accept(TokenStream & token_stream, ASTNodeStack & s){
    SLRstate next_state;
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(tkn_to_eat));
        next_state = REDUCE_ADDR_ID_WITH_TABLE_ID;
    }
    else{
        throw ParseError(tkn_to_eat.content, "wait attr dot right value error");
    }
    return next_state;
}

SLRstate Generator::reduce_attr_id_with_table_id::Accept(TokenStream & token_stream, ASTNodeStack & s){
    SLRstate next_state;
    ASTreeNode* attrIdWithTableID = reduceAttrIdWithTableId(s); 
    if(s.empty() || s.top()->getTag() == ParserSymbol::attr){
        // select attrID
        next_state = REDUCE_ATTR; //6
    }
    else if(s.top()->getTag() == condition){
        next_state = REDUCE_CONDITION;                
    }
    else{
        next_state = WAIT_EUQALITY;
    }
    s.push(attrIdWithTableID);
    return next_state;
}

SLRstate Generator::reduce_attr::Accept(TokenStream & token_stream, ASTNodeStack & s){
    SLRstate next_state;
    const Token & lookahead = token_stream.front();
    if(lookahead.type == Token::SYMBOL && lookahead.content == "as"){
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

SLRstate Generator::wait_attr_alias::Accept(TokenStream & token_stream, ASTNodeStack & s){
    SLRstate next_state;
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(tkn_to_eat));
        next_state = REDUCE_ATTR_WITH_ALIAS;
    }
    else{
        throw ParseError(tkn_to_eat.content, "wait attr alias error");
    }
    return next_state;
}


SLRstate Generator::reduce_attr_with_alias::Accept(TokenStream & token_stream, ASTNodeStack & s){
    ASTreeNode* attr_node_with_alias = reduceAttrWithAlias(s);
    s.push(attr_node_with_alias);
    return REDUCE_ATTR_SET;
}

SLRstate Generator::reduce_attr_set::Accept(TokenStream & token_stream, ASTNodeStack & s){
    SLRstate next_state;
    const Token & lookahead = token_stream.front();
    if(lookahead.type == Token::SYMBOL && lookahead.content == ","){
        token_stream.pop_front();
        next_state = WAIT_ATTR_ID;
    }
    else{
        ASTreeNode* attrSetNode = reduceAttrSet(s);
        next_state = WAIT_FROM;
        s.push(attrSetNode);
    }
    return next_state;
}


SLRstate Generator::wait_from::Accept(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::KEYWORD && tkn_to_eat.content == "from"){
        return WAIT_TABLE_ID;
    }
    else{
        throw ParseError("", "expect from");
    }
}


SLRstate Generator::wait_table_id::Accept(TokenStream & token_stream, ASTNodeStack & s){
    SLRstate next_state;
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::SYMBOL && tkn_to_eat.content == "("){
        next_state = WAIT_SELECT;
    }
    else if(tkn_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(tkn_to_eat));
        next_state = REDUCE_TABLE_ID;
    }
    else{
        throw ParseError(tkn_to_eat.content, "expect a table id");
    }
    return next_state;
}

SLRstate Generator::reduce_table_id::Accept(TokenStream & token_stream, ASTNodeStack & s){
    s.push(reduceTableID(s));
    return RECUDE_TABLE;
}

SLRstate Generator::reduce_table::Accept(TokenStream & token_stream, ASTNodeStack & s){
    const Token & lookahead = token_stream.front();
    SLRstate next_state;
    if(lookahead.type == Token::KEYWORD && lookahead.content == "as"){
        next_state = WAIT_TABLE_ALIAS;
        token_stream.pop_front();
    }
    else{
        next_state = REDUCE_TABLE_SET;
        ASTreeNode* tableNode = reduceTableWithoutAlias(s);
        s.push(tableNode);
    }
    return next_state;
}

SLRstate Generator::wait_table_alias::Accept(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(tkn_to_eat));
    }
    else{
        throw ParseError(tkn_to_eat.content, "expect a table alias");
    }
    return REDUCE_TABLE_WITH_ALIAS;
}

SLRstate Generator::reduce_table_with_alias::Accept(TokenStream & token_stream, ASTNodeStack & s){
    ASTreeNode* node_with_aliased_table = reduceTableWithAlias(s);
    s.push(node_with_aliased_table);
    return REDUCE_TABLE_SET;
}

//19
SLRstate Generator::reduce_table_set::Accept(TokenStream & token_stream, ASTNodeStack & s){
    const Token & lookahead = token_stream.front();
    SLRstate next_state;
    if(lookahead.type == Token::KEYWORD && lookahead.content == "join"){
        next_state = WAIT_TABLE_ID;
        token_stream.pop_front();
        s.push(new ASTreeNode(table_set, join));
    }
    else if(lookahead.type == Token::KEYWORD && lookahead.content == "naturaljoin"){
        next_state = WAIT_TABLE_ID;
        token_stream.pop_front();
        s.push(new ASTreeNode(table_set, naturaljoin));
    }
    else if(lookahead.type == Token::SYMBOL && lookahead.content == ","){
        next_state = WAIT_TABLE_ID;
        token_stream.pop_front();
        s.push(new ASTreeNode(table_set, join));
    }
    else{
        ASTreeNode* table_set_node = reduceTableSet(s);
        next_state = WAIT_WHERE;
    }
    return next_state;
}

SLRstate Generator::wait_where::Accept(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::KEYWORD && tkn_to_eat.content == "where"){
        return WAIT_CONDITION;
    }
    else{
        throw ParseError(tkn_to_eat.content, "expect where");
    }
}

SLRstate Generator::wait_condition::Accept(TokenStream & token_stream, ASTNodeStack & s){
    const Token & lookahead = token_stream.front();
    SLRstate next_state;
    if(lookahead.type == Token::IDENTIFIER){
        next_state = WAIT_ATTR_ID;
    }
    else{
        next_state = WAIT_NUM_OR_STR;
    }
    return next_state;
}

SLRstate Generator::wait_num_or_str::Accept(TokenStream & token_stream, ASTNodeStack & s){
    const Token & tkn_to_eat = token_stream.pop_front();
    SLRstate next_state;
    if(tkn_to_eat.type == Token::INTS || tkn_to_eat.type == Token::FLOATS || tkn_to_eat.type == Token::STR){
        if(s.top()->getTag() == equality){
            next_state = REDUCE_CONDITION;
        }
        else{
            next_state = WAIT_EUQALITY;            
        }
        s.push(new ASTreeNode(tkn_to_eat));
        return next_state;
    }
    else{
        throw ParseError(tkn_to_eat.content, "expect int or float or string");
    }
    return next_state;
}

SLRstate Generator::wait_equality::Accept(TokenStream & token_stream, ASTNodeStack & s){
    const Token & tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::EQUALITY){
        s.push(new ASTreeNode(tkn_to_eat));
        return WAIT_CONDITION;
    }
    else{
        throw ParseError(tkn_to_eat.content, "expect = or <> or < or > or <= or >=");
    }    
}

SLRstate Generator::reduce_condition::Accept(TokenStream & token_stream, ASTNodeStack & s){
    ASTreeNode* node_with_condition = reduceCondition(s);
    return REDUCE_CONDITION_SET;
}

SLRstate Generator::reduce_condition_set::Accept(TokenStream & token_stream, ASTNodeStack & s){
    const Token & lookahead = token_stream.front();
    SLRstate next_state;
    if(lookahead.type == Token::KEYWORD && lookahead.content == "and"){
        next_state = WAIT_CONDITION;
        s.push(new ASTreeNode(condition_set, and_));
    }
    else if(lookahead.type == Token::KEYWORD && lookahead.content == "or"){
        next_state = WAIT_CONDITION;
        s.push(new ASTreeNode(condition_set, or_));        
    }
    else{
        ASTreeNode* node_with_condition_set = reduceConditionSet(s);
        s.push(node_with_condition_set);
        return REDUCE_QUERY;
    }
}

SLRstate Generator::reduce_query::Accept(TokenStream & token_stream, ASTNodeStack & s){
    s.push(reduceQuery(s));
    const Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::SYMBOL && tkn_to_eat.content == ")"){
        return REDUCE_TABLE_ID;
    }   
    else if(tkn_to_eat.type == Token::NONE){
        return FINISH;
    }
    else{
        throw ParseError(tkn_to_eat.content, "end of query.");
    }
}
