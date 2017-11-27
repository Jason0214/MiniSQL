#include "Generator.h"

#include "../../EXCEPTION.h"
#include "Reducer.h"

using namespace std;
using namespace ParserSymbol;


static ASTreeNode* categoryEqualityToken(const Token & equality_token){
    if(equality_token.content == "="){
        return new ASTreeNode(condition, equal_);
    }
    else if(equality_token.content == "<="){
        return new ASTreeNode(condition, less_equal_);
    }
    else if(equality_token.content == ">="){
        return new ASTreeNode(condition, larger_equal_);
    }
    else if(equality_token.content == "<"){
        return new ASTreeNode(condition, less_);
    }
    else if(equality_token.content == ">"){
        return new ASTreeNode(condition, larger_);
    }
    else if(equality_token.content == "<>"){
        return new ASTreeNode(condition, not_equal_);
    }
}

//
// select parser generators
//

QueryState Generator::QueryGenerator::wait_select(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::KEYWORD && tkn_to_eat.content == "select"){
        return WAIT_ATTR_ID;
    }
    throw ParseError(tkn_to_eat.content, "expect select");
}


QueryState Generator::QueryGenerator::wait_attr_id(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    QueryState next_state;
    if(tkn_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(tkn_to_eat));
        next_state = REDUCE_ATTR_ID;
    }
    else if(tkn_to_eat.type == Token::SYMBOL && tkn_to_eat.content == "*"){
        s.push(new ASTreeNode(attr_set, star));
        next_state = WAIT_FROM;
    }
    else{
        throw ParseError(tkn_to_eat.content, "expect attr id");
    }
    return next_state;
}

QueryState Generator::QueryGenerator::reduce_attr_id(TokenStream & token_stream, ASTNodeStack & s){
    QueryState next_state;
    const Token & lookahead = token_stream.front();
    //SLR
    if(lookahead.type == Token::SYMBOL && lookahead.content == "."){
        token_stream.pop_front();
        next_state = WAIT_ATTR_DOT_RIGHT;
    }
    else{
        ASTreeNode* attrIdNode = reduceAttrId(s);
        if(s.empty() || s.top()->getTag() == ParserSymbol::attr
                      || s.top()->getTag() == ParserSymbol::attr_set){
            // select attrID
            next_state = REDUCE_ATTR; //6
        }
        else if(s.top()->getTag() == condition){
            next_state = REDUCE_CONDITION;                
        }
        else{
            next_state = WAIT_EQUALITY;
        }
        s.push(attrIdNode);
    }
    return next_state;
}

QueryState Generator::QueryGenerator::wait_addr_dot_right(TokenStream & token_stream, ASTNodeStack & s){
    QueryState next_state;
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

QueryState Generator::QueryGenerator::reduce_attr_id_with_table_id(TokenStream & token_stream, ASTNodeStack & s){
    QueryState next_state;
    ASTreeNode* attrIdWithTableID = reduceAttrIdWithTableId(s); 
    if(s.empty() || s.top()->getTag() == ParserSymbol::attr){
        // select attrID
        next_state = REDUCE_ATTR; //6
    }
    else if(s.top()->getTag() == condition){
        next_state = REDUCE_CONDITION;                
    }
    else{
        next_state = WAIT_EQUALITY;
    }
    s.push(attrIdWithTableID);
    return next_state;
}

QueryState Generator::QueryGenerator::reduce_attr(TokenStream & token_stream, ASTNodeStack & s){
    QueryState next_state;
    const Token & lookahead = token_stream.front();
    if(lookahead.type == Token::KEYWORD && lookahead.content == "as"){
        token_stream.pop_front();
        next_state = WAIT_ATTR_ALIAS; //7
    }
    else{
        ASTreeNode* attr_node = reduceAttrWithoutAlias(s);
        s.push(attr_node);
        next_state = REDUCE_ATTR_SET;
    }
    return next_state;
}

QueryState Generator::QueryGenerator::wait_attr_alias(TokenStream & token_stream, ASTNodeStack & s){
    QueryState next_state;
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


QueryState Generator::QueryGenerator::reduce_attr_with_alias(TokenStream & token_stream, ASTNodeStack & s){
    ASTreeNode* attr_node_with_alias = reduceAttrWithAlias(s);
    s.push(attr_node_with_alias);
    return REDUCE_ATTR_SET;
}

QueryState Generator::QueryGenerator::reduce_attr_set(TokenStream & token_stream, ASTNodeStack & s){
    QueryState next_state;
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


QueryState Generator::QueryGenerator::wait_from(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::KEYWORD && tkn_to_eat.content == "from"){
        return WAIT_TABLE_ID;
    }
    throw ParseError("", "expect from");
}


QueryState Generator::QueryGenerator::wait_table_id(TokenStream & token_stream, ASTNodeStack & s){
    QueryState next_state;
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

QueryState Generator::QueryGenerator::reduce_table_id(TokenStream & token_stream, ASTNodeStack & s){
    s.push(reduceTableID(s));
    return REDUCE_TABLE;
}

QueryState Generator::QueryGenerator::reduce_table(TokenStream & token_stream, ASTNodeStack & s){
    const Token & lookahead = token_stream.front();
    QueryState next_state;
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

QueryState Generator::QueryGenerator::wait_table_alias(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(tkn_to_eat));
    }
    else{
        throw ParseError(tkn_to_eat.content, "expect a table alias");
    }
    return REDUCE_TABLE_WITH_ALIAS;
}

QueryState Generator::QueryGenerator::reduce_table_with_alias(TokenStream & token_stream, ASTNodeStack & s){
    ASTreeNode* node_with_aliased_table = reduceTableWithAlias(s);
    s.push(node_with_aliased_table);
    return REDUCE_TABLE_SET;
}

//19
QueryState Generator::QueryGenerator::reduce_table_set(TokenStream & token_stream, ASTNodeStack & s){
    const Token & lookahead = token_stream.front();
    QueryState next_state;
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
        s.push(table_set_node);
    }
    return next_state;
}

QueryState Generator::QueryGenerator::wait_where(TokenStream & token_stream, ASTNodeStack & s){
    const Token & lookahead = token_stream.front();
    if(lookahead.type == Token::KEYWORD && lookahead.content == "where"){
        token_stream.pop_front();
        return WAIT_CONDITION;
    }
    else{
        return REDUCE_QUERY_WITHOUT_CONDITION;
    }
}

QueryState Generator::QueryGenerator::reduce_query_without_condition(TokenStream & token_stream, ASTNodeStack & s){
    s.push(reduceQueryWithoutCondition(s));
    const Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::SYMBOL && tkn_to_eat.content == ")"){
        return REDUCE_TABLE_ID;
    }
    if(tkn_to_eat.type == Token::NONE){
        return FINISH_QUERY;
    }

    throw ParseError(tkn_to_eat.content, "expect 'where'");
}

QueryState Generator::QueryGenerator::wait_condition(TokenStream & token_stream, ASTNodeStack & s){
    const Token & lookahead = token_stream.front();
    QueryState next_state;
    if(lookahead.type == Token::IDENTIFIER){
        next_state = WAIT_ATTR_ID;
    }
    else{
        next_state = WAIT_NUM_OR_STR;
    }
    return next_state;
}


QueryState Generator::QueryGenerator::wait_num_or_str(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    QueryState next_state;
    if(tkn_to_eat.type == Token::INTS || tkn_to_eat.type == Token::FLOATS || tkn_to_eat.type == Token::STR){
        if(s.top()->getTag() == condition){
            next_state = REDUCE_CONDITION;
        }
        else{
            next_state = WAIT_EQUALITY;
        }
        s.push(new ASTreeNode(tkn_to_eat));
        return next_state;
    }
    throw ParseError(tkn_to_eat.content, "expect int or float or string");
}

QueryState Generator::QueryGenerator::wait_equality(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::EQUALITY){
        s.push(categoryEqualityToken(tkn_to_eat));
        return WAIT_CONDITION;
    }

    throw ParseError(tkn_to_eat.content, "expect = or <> or < or > or <= or >=");
}

QueryState Generator::QueryGenerator::reduce_condition(TokenStream & token_stream, ASTNodeStack & s){
    ASTreeNode* node_with_condition = reduceCondition(s);
    s.push(node_with_condition);
    return REDUCE_CONDITION_SET;
}

QueryState Generator::QueryGenerator::reduce_condition_set(TokenStream & token_stream, ASTNodeStack & s){
    const Token & lookahead = token_stream.front();
    QueryState next_state;
    if(lookahead.type == Token::KEYWORD && lookahead.content == "and"){
        next_state = WAIT_CONDITION;
        token_stream.pop_front();
        s.push(new ASTreeNode(condition_set, and_));
    }
    else if(lookahead.type == Token::KEYWORD && lookahead.content == "or"){
        next_state = WAIT_CONDITION;
        token_stream.pop_front();
        s.push(new ASTreeNode(condition_set, or_));        
    }
    else{
        ASTreeNode* node_with_condition_set = reduceConditionSet(s);
        s.push(node_with_condition_set);
        next_state = REDUCE_QUERY_WITH_CONDITION;
    }
    return next_state;
}

QueryState Generator::QueryGenerator::reduce_query_with_condition(TokenStream & token_stream, ASTNodeStack & s){
    s.push(reduceQueryWithCondition(s));
    const Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::SYMBOL && tkn_to_eat.content == ")"){
        return REDUCE_TABLE_ID;
    }   
    if(tkn_to_eat.type == Token::NONE){
        return FINISH_QUERY;
    }

    throw ParseError(tkn_to_eat.content, "end of query.");
}

//
// delete parser generators
//
DeleteState Generator::DeleteGenerator::wait_delete(TokenStream & token_stream, ASTNodeStack & s){
    Token token = token_stream.pop_front();
    if(token.type == Token::KEYWORD && token.content == "delete"){
        return WAIT_FROM_IN_DELETE;
    }

    throw ParseError(token.content, "expect delete");
}

DeleteState Generator::DeleteGenerator::wait_from_in_delete(TokenStream & token_stream, ASTNodeStack & s){
    Token token = token_stream.pop_front();
    if(token.type == Token::KEYWORD && token.content == "from"){
        return WAIT_TABLE_ID_IN_DELETE;
    }

    throw ParseError(token.content, "expect from");
}

DeleteState Generator::DeleteGenerator::wait_table_id_in_delete(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(tkn_to_eat));
        return WAIT_WHERE_IN_DELETE;
    }

    throw ParseError(tkn_to_eat.content, "expect table ID");
}

DeleteState Generator::DeleteGenerator::wait_where_in_delete(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::KEYWORD && tkn_to_eat.content == "where"){
        return WAIT_CONDITION_IN_DELETE;
    }

    throw ParseError(tkn_to_eat.content, "expect where");
}

DeleteState Generator::DeleteGenerator::wait_condition_in_delete(TokenStream & token_stream, ASTNodeStack & s){
    const Token & lookahead = token_stream.front();
    DeleteState next_state;
    if(lookahead.type == Token::IDENTIFIER){
        next_state = WAIT_ATTR_IN_DELETE;
    }
    else{
        next_state = WAIT_NUM_OR_STR_IN_DELETE;
    }
    return next_state;
}

DeleteState Generator::DeleteGenerator::wait_attr_in_delete(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    s.push(new ASTreeNode(tkn_to_eat));
    const Token & lookahead = token_stream.front();
    if(lookahead.type == Token::EQUALITY){
        return WAIT_EQUALITY_IN_DELETE;
    }
    else{
        return REDUCE_CONDITION_IN_DELETE;
    }
}

DeleteState Generator::DeleteGenerator::wait_num_or_str_in_delete(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    DeleteState next_state;
    if(tkn_to_eat.type == Token::INTS || tkn_to_eat.type == Token::FLOATS || tkn_to_eat.type == Token::STR){
        if(s.top()->getTag() == condition){
            next_state = REDUCE_CONDITION_IN_DELETE;
        }
        else{
            next_state = WAIT_EQUALITY_IN_DELETE;
        }
        s.push(new ASTreeNode(tkn_to_eat));
        return next_state;
    }
    throw ParseError(tkn_to_eat.content, "expect int or float or string");
}

DeleteState Generator::DeleteGenerator::wait_equality_in_delete(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::EQUALITY){
        s.push(categoryEqualityToken(tkn_to_eat));
        return WAIT_CONDITION_IN_DELETE;
    }

    throw ParseError(tkn_to_eat.content, "expect = or <> or < or > or <= or >=");
}

DeleteState Generator::DeleteGenerator::reduce_condition_in_delete(TokenStream & token_stream, ASTNodeStack & s){
    ASTreeNode* node_with_condition = reduceCondition(s);
    s.push(node_with_condition);
    return REDUCE_CONDITION_SET_IN_DELETE;
}

DeleteState Generator::DeleteGenerator::reduce_condition_set_in_delete(TokenStream & token_stream, ASTNodeStack & s){
    const Token & lookahead = token_stream.front();
    DeleteState next_state;
    if(lookahead.type == Token::KEYWORD && lookahead.content == "and"){
        next_state = WAIT_CONDITION_IN_DELETE;
        token_stream.pop_front();
        s.push(new ASTreeNode(condition_set, and_));
    }
    else if(lookahead.type == Token::KEYWORD && lookahead.content == "or"){
        next_state = WAIT_CONDITION_IN_DELETE;
        token_stream.pop_front();
        s.push(new ASTreeNode(condition_set, or_));
    }
    else{
        ASTreeNode* node_with_condition_set = reduceConditionSet(s);
        s.push(node_with_condition_set);
        next_state = REDUCE_DELETE;
    }
    return next_state;
}

DeleteState Generator::DeleteGenerator::reduce_delete(TokenStream & token_stream, ASTNodeStack & s){
    s.push(reduceDelete(s));
    const Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::NONE){
        return FINISH_DELETE;
    }

    throw ParseError(tkn_to_eat.content, "end of query.");
}


//
// Insert generators
//

InsertState Generator::InsertGenerator::wait_insert(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::KEYWORD && token_to_eat.content == "insert"){
        return WAIT_INTO;
    }

    throw ParseError(token_to_eat.content, "expect 'insert'");
};

InsertState Generator::InsertGenerator::wait_into(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::KEYWORD && token_to_eat.content == "into"){
        return WAIT_TABLE_IN_INSERT;
    }

    throw ParseError(token_to_eat.content, "expect 'into'");
};

InsertState Generator::InsertGenerator::wait_table_in_insert(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(token_to_eat));
        return WAIT_VALUE_SET;
    }

    throw ParseError(token_to_eat.content, "expect table name");
}

InsertState Generator::InsertGenerator::wait_value_set(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::KEYWORD && token_to_eat.content == "values"){
        return BEGIN_OF_VALUE_SET;
    }

    throw ParseError(token_to_eat.content, "expect 'values'");
};

InsertState Generator::InsertGenerator::begin_of_value_set(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::SYMBOL && token_to_eat.content == "("){
        return WAIT_SINGLE_VALUE;
    }
    throw ParseError(token_to_eat.content, "expect '('");
};

InsertState Generator::InsertGenerator::wait_single_value(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    const Token & lookahead = token_stream.front();
    if(token_to_eat.type == Token::FLOATS || token_to_eat.type == Token::INTS || token_to_eat.type == Token::STR){
        s.push(new ASTreeNode(token_to_eat));
        if(lookahead.type == Token::SYMBOL && lookahead.content == ","){
            token_stream.pop_front();
            return WAIT_SINGLE_VALUE;
        }
        else{
            return REDUCE_VALUE_SET;
        }
    }

    throw ParseError(token_to_eat.content, "expect a value");
}

InsertState Generator::InsertGenerator::reduce_value_set(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::SYMBOL && token_to_eat.content == ")"){
        ASTreeNode* value_set_node = reduceValueSet(s);
        s.push(value_set_node);
        return REDUCE_INSERT;
    }

    throw ParseError(token_to_eat.content, "expect ')'");
}

InsertState Generator::InsertGenerator::reduce_insert(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::NONE){
        s.push(reduceInsert(s));
        return FINISH_INSERT;
    }

    throw ParseError(token_to_eat.content, "should be end of insert");
}

//
// Create table generators
//

CreateTableState Generator::CreateTableGenerator::wait_table_in_create_table(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(token_to_eat));
        return BEGIN_OF_META_SET;
    }
    throw ParseError(token_to_eat.content, "expect table name");
}

CreateTableState Generator::CreateTableGenerator::begin_of_meta_set(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::SYMBOL && token_to_eat.content == "("){
        return WAIT_META;
    }
    throw ParseError(token_to_eat.content, "expect '('");
}

CreateTableState Generator::CreateTableGenerator::wait_meta(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(token_to_eat));
        return WAIT_TYPE;
    }
    throw ParseError(token_to_eat.content, "expect attribute name");
}

CreateTableState Generator::CreateTableGenerator::wait_type(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::KEYWORD && token_to_eat.content == "int"){
        s.push(new ASTreeNode(type, type_int));
        return WAIT_CONSTRAIN;
    }
    if(token_to_eat.type == Token::KEYWORD && token_to_eat.content == "float"){
        s.push(new ASTreeNode(type, type_float));
        return WAIT_CONSTRAIN;
    }
    if(token_to_eat.type == Token::KEYWORD && token_to_eat.content == "char"){
        s.push(new ASTreeNode(type, type_char));
        return WAIT_TYPE_PARAM;
    }
    if(token_to_eat.type == Token::KEYWORD && token_to_eat.content == "varchar"){
        s.push(new ASTreeNode(type, type_varchar));
        return WAIT_TYPE_PARAM;
    }

    throw ParseError(token_to_eat.content, "expect int/float/char");
}

CreateTableState Generator::CreateTableGenerator::wait_type_param(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::SYMBOL && token_to_eat.content == "("){
        token_to_eat = token_stream.pop_front();
        if(token_to_eat.type == Token::INTS){
            s.push(new ASTreeNode(token_to_eat));
            token_to_eat = token_stream.pop_front();
            if(token_to_eat.type == Token::SYMBOL && token_to_eat.content == ")"){
                return REDUCE_TYPE;
            }
            throw ParseError(token_to_eat.content, "expect ')'");
        }
        throw ParseError(token_to_eat.content, "expect char parameters");
    }
    throw ParseError(token_to_eat.content, "expect '('");
}

CreateTableState Generator::CreateTableGenerator::reduce_type(TokenStream & token_stream, ASTNodeStack & s){
    s.push(reduceType(s));
    return WAIT_CONSTRAIN;
}

CreateTableState Generator::CreateTableGenerator::wait_constrain(TokenStream & token_stream, ASTNodeStack & s){
    const Token & lookahead = token_stream.front();
    if(lookahead.type == Token::KEYWORD && lookahead.content == "primary"){
        token_stream.pop_front();
        Token token_to_eat = token_stream.pop_front();
        if(token_to_eat.type == Token::KEYWORD && token_to_eat.content == "key"){
            s.push(new ASTreeNode(constrain, primary_key));
            return WAIT_CONSTRAIN;
        }
        throw ParseError(token_to_eat.content, "expect 'key'");
    }
    if(lookahead.type == Token::KEYWORD && lookahead.content == "not"){
        token_stream.pop_front();
        Token token_to_eat = token_stream.pop_front();
        if(token_to_eat.type == Token::KEYWORD && token_to_eat.content == "null"){
            s.push(new ASTreeNode(constrain, not_null));
            return WAIT_CONSTRAIN;
        }
        throw ParseError(token_to_eat.content, "expect 'null'");
    }
    if(lookahead.type == Token::KEYWORD && lookahead.content == "unique"){
        token_stream.pop_front();
        s.push(new ASTreeNode(constrain, unique_));
        return WAIT_CONSTRAIN;
    }
    return REDUCE_META;
}

CreateTableState Generator::CreateTableGenerator::reduce_meta(TokenStream & token_stream, ASTNodeStack & s){
    s.push(reduceMeta(s));
    const Token & lookahead = token_stream.front();
    if(lookahead.type == Token::SYMBOL && lookahead.content == ","){
        token_stream.pop_front();
        return WAIT_META;
    }
    else{
        return REDUCE_META_SET;
    }
}

CreateTableState Generator::CreateTableGenerator::reduce_meta_set(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::SYMBOL && token_to_eat.content == ")"){
        s.push(reduceMetaSet(s));
        return REDUCE_CREATE_TABLE;
    }
    throw ParseError(token_to_eat.content, "expect ')'");
}

CreateTableState Generator::CreateTableGenerator::reduce_create_table(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::NONE){
        s.push(reduceCreateTable(s));
        return FINISH_CREATE_TABLE;
    }
    throw ParseError(token_to_eat.content, "should be end of sentence");
}

//
// update generators
//

UpdateState Generator::UpdateGenerator::wait_update(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::KEYWORD && token_to_eat.content == "update"){
        return WAIT_TABLE_IN_UPDATE;
    }

    throw ParseError(token_to_eat.content, "expect 'update'");
}

UpdateState Generator::UpdateGenerator::wait_table_in_update(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(token_to_eat));
        return BEGIN_OF_ASSIGN_SET;
    }
    
    throw ParseError(token_to_eat.content, "expect table name");
}

UpdateState Generator::UpdateGenerator::begin_of_assign_set(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::KEYWORD && token_to_eat.content == "set"){
        return WAIT_ATTR_IN_ASSIGN;
    }
    throw ParseError(token_to_eat.content, "expect 'set'");
}

UpdateState Generator::UpdateGenerator::wait_attr_in_assign(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(token_to_eat));
        return WAIT_EQUAL_IN_ASSIGN;
    }
    throw ParseError(token_to_eat.content, "expect attribute name");
}

UpdateState Generator::UpdateGenerator::wait_equal_in_assign(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::EQUALITY && token_to_eat.content == "="){
       return WAIT_ASSIGN_VALUE;
    } 
    throw ParseError(token_to_eat.content, "expect '='");
}

UpdateState Generator::UpdateGenerator::wait_assign_value(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::INTS || token_to_eat.type == Token::FLOATS || token_to_eat.type == Token::STR){
        s.push(new ASTreeNode(token_to_eat));
        return REDUCE_ASSIGN;
    }
    throw ParseError(token_to_eat.content, "expect assignment value");
}

UpdateState Generator::UpdateGenerator::reduce_assign(TokenStream & token_stream, ASTNodeStack & s){
    s.push(reduceAssign(s));
    const Token & lookahead = token_stream.front();
    if(lookahead.type == Token::SYMBOL && lookahead.content == ","){
        token_stream.pop_front();
        return WAIT_ATTR_IN_ASSIGN;
    }
    else{
        return REDUCE_ASSIGN_SET;
    }
}

UpdateState Generator::UpdateGenerator::reduce_assign_set(TokenStream & token_stream, ASTNodeStack & s){
    s.push(reduceAssignSet(s));
    return WAIT_WHERE_IN_UPDATE;
}

UpdateState Generator::UpdateGenerator::wait_where_in_update(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::KEYWORD && tkn_to_eat.content == "where"){
        return WAIT_CONDITION_IN_UPDATE;
    }

    throw ParseError(tkn_to_eat.content, "expect where");
}

UpdateState Generator::UpdateGenerator::wait_condition_in_update(TokenStream & token_stream, ASTNodeStack & s){
    const Token & lookahead = token_stream.front();
    UpdateState next_state;
    if(lookahead.type == Token::IDENTIFIER){
        next_state = WAIT_ATTR_IN_UPDATE;
    }
    else{
        next_state = WAIT_NUM_OR_STR_IN_UPDATE;
    }
    return next_state;
}

UpdateState Generator::UpdateGenerator::wait_attr_in_update(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    s.push(new ASTreeNode(tkn_to_eat));
    const Token & lookahead = token_stream.front();
    if(lookahead.type == Token::EQUALITY){
        return WAIT_EQUALITY_IN_UPDATE;
    }
    else{
        return REDUCE_CONDITION_IN_UPDATE;
    }
}

UpdateState Generator::UpdateGenerator::wait_num_or_str_in_update(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    UpdateState next_state;
    if(tkn_to_eat.type == Token::INTS || tkn_to_eat.type == Token::FLOATS || tkn_to_eat.type == Token::STR){
        if(s.top()->getTag() == condition){
            next_state = REDUCE_CONDITION_IN_UPDATE;
        }
        else{
            next_state = WAIT_EQUALITY_IN_UPDATE;
        }
        s.push(new ASTreeNode(tkn_to_eat));
        return next_state;
    }
    throw ParseError(tkn_to_eat.content, "expect int or float or string");
}

UpdateState Generator::UpdateGenerator::wait_equality_in_update(TokenStream & token_stream, ASTNodeStack & s){
    Token tkn_to_eat = token_stream.pop_front();
    if(tkn_to_eat.type == Token::EQUALITY){
        s.push(categoryEqualityToken(tkn_to_eat));
        return WAIT_CONDITION_IN_UPDATE;
    }

    throw ParseError(tkn_to_eat.content, "expect = or <> or < or > or <= or >=");
}

UpdateState Generator::UpdateGenerator::reduce_condition_in_update(TokenStream & token_stream, ASTNodeStack & s){
    ASTreeNode* node_with_condition = reduceCondition(s);
    s.push(node_with_condition);
    return REDUCE_CONDITION_SET_IN_UPDATE;
}

UpdateState Generator::UpdateGenerator::reduce_condition_set_in_update(TokenStream & token_stream, ASTNodeStack & s){
    const Token & lookahead = token_stream.front();
    UpdateState next_state;
    if(lookahead.type == Token::KEYWORD && lookahead.content == "and"){
        next_state = WAIT_CONDITION_IN_UPDATE;
        token_stream.pop_front();
        s.push(new ASTreeNode(condition_set, and_));
    }
    else if(lookahead.type == Token::KEYWORD && lookahead.content == "or"){
        next_state = WAIT_CONDITION_IN_UPDATE;
        token_stream.pop_front();
        s.push(new ASTreeNode(condition_set, or_));
    }
    else{
        ASTreeNode* node_with_condition_set = reduceConditionSet(s);
        s.push(node_with_condition_set);
        next_state = REDUCE_UPDATE;
    }
    return next_state;
}


UpdateState Generator::UpdateGenerator::reduce_update(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::NONE){
        s.push(reduceUpdate(s));
        return FINISH_UPDATE;
    }
    throw ParseError(token_to_eat.content, "should be end of update sentence");
}

//
// create index generators
//

CreateIndexState Generator::CreateIndexGenerator::wait_index_name(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(token_to_eat));
        return WAIT_ON;
    }
    throw ParseError(token_to_eat.content, "expect index name");
}
CreateIndexState Generator::CreateIndexGenerator::wait_on(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::KEYWORD && token_to_eat.content == "on"){
        return WAIT_TABLE_IN_CREATE_INDEX;
    }
    throw ParseError(token_to_eat.content, "expect 'on'");
}
CreateIndexState Generator::CreateIndexGenerator::wait_table_in_create_index(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(token_to_eat));
        return BEGIN_OF_COLUMN_SET;
    }
    throw ParseError(token_to_eat.content, "expect table name");
}
CreateIndexState Generator::CreateIndexGenerator::begin_of_column_set(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::SYMBOL && token_to_eat.content == "("){
        return WAIT_COLUMN_SET;
    }
    throw ParseError(token_to_eat.content, "expect '('");
}
CreateIndexState Generator::CreateIndexGenerator::wait_column_set(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::IDENTIFIER){
        s.push(new ASTreeNode(token_to_eat));
        return REDUCE_COLUMN_SET;
    }
    throw ParseError(token_to_eat.content, "expect attribute name");
}

CreateIndexState Generator::CreateIndexGenerator::reduce_column_set(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::SYMBOL && token_to_eat.content == ","){
        return WAIT_COLUMN_SET;
    }
    if(token_to_eat.type == Token::SYMBOL && token_to_eat.content == ")"){
        s.push(reduceColumnSet(s));
        return REDUCE_CREATE_INDEX;
    }
    throw ParseError(token_to_eat.content, "expect ',' or ')'");
}
CreateIndexState Generator::CreateIndexGenerator::reduce_create_index(TokenStream & token_stream, ASTNodeStack & s){
    Token token_to_eat = token_stream.pop_front();
    if(token_to_eat.type == Token::NONE){
        s.push(reduceCreateIndex(s));
        return FINISH_CREATE_INDEX;
    }
    throw ParseError(token_to_eat.content, "should be end of create index sentence");
}
