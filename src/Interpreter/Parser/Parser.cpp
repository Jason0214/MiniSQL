#include "Parser.h"

#include <string>
#include <stack>


#include <iostream>

#include "ASTreeNodeStack.h"
#include "../../EXCEPTION.h"

using namespace std;

void Parser::loadGenerator(){
    this->query_generators_[0] = NULL;
    this->query_generators_[1] = new Generator::wait_select();
    this->query_generators_[2] = new Generator::wait_attr_id();
    this->query_generators_[3] = new Generator::reduce_attr_id();
    this->query_generators_[4] = new Generator::wait_addr_dot_right();
    this->query_generators_[5] = new Generator::reduce_attr_id_with_table_id();
    this->query_generators_[6] = new Generator::reduce_attr();
    this->query_generators_[7] = new Generator::wait_attr_alias();
    this->query_generators_[8] = new Generator::reduce_attr_with_alias();
    this->query_generators_[9] = new Generator::reduce_attr_set();
    this->query_generators_[10] = new Generator::wait_from();
    this->query_generators_[11] = new Generator::wait_table_id();
    this->query_generators_[12] = new Generator::reduce_table_id();
    this->query_generators_[13] = new Generator::reduce_table();
    this->query_generators_[14] = new Generator::wait_table_alias();
    this->query_generators_[15] = new Generator::reduce_table_with_alias();
    this->query_generators_[16] = new Generator::reduce_table_set();
    this->query_generators_[17] = new Generator::wait_where();
    this->query_generators_[18] = new Generator::reduce_query_without_condition();
    this->query_generators_[19] = new Generator::wait_condition();
    this->query_generators_[20] = new Generator::wait_num_or_str();
    this->query_generators_[21] = new Generator::wait_equality();
    this->query_generators_[22] = new Generator::reduce_condition();
    this->query_generators_[23] = new Generator::reduce_condition_set();
    this->query_generators_[24] = new Generator::reduce_query_with_condition();

    this->delete_generators_[0] = NULL;
    this->delete_generators_[1] = new Generator::wait_from_in_delete();
    this->delete_generators_[2] = new Generator::wait_table_id_in_delete();
    this->delete_generators_[3] = new Generator::wait_where_in_delete();
    this->delete_generators_[4] = new Generator::wait_condition_in_delete();
    this->delete_generators_[5] = new Generator::wait_attr_in_delete();
    this->delete_generators_[6] = new Generator::wait_num_or_str_in_delete();
    this->delete_generators_[7] = new Generator::wait_equality_in_delete();
    this->delete_generators_[8] = new Generator::reduce_condition_in_delete();
    this->delete_generators_[9] = new Generator::reduce_condition_set_in_delete();
    this->delete_generators_[10] = new Generator::reduce_delete();

    this->insert_generators_[0] = NULL;
    this->insert_generators_[1] = new Generator::wait_insert();
    this->insert_generators_[2] = new Generator::wait_into();
    this->insert_generators_[3] = new Generator::wait_table_in_insert();
    this->insert_generators_[5] = new Generator::wait_value_set();
    this->insert_generators_[5] = new Generator::begin_of_value_set();
    this->insert_generators_[6] = new Generator::wait_single_value();
    this->insert_generators_[7] = new Generator::reduce_value_set();
}

void Parser::deleteGenerator(){
    for(int i = 0; i < QUERY_STATE_CNT; i++){
        delete this->query_generators_[i];
        this->query_generators_[i] = NULL;
    }
}

void Parser::parseSentence(TokenStream & token_stream){
    const Token & t = token_stream.front();
    if(t.type == Token::KEYWORD){
        if(t.content == "select"){
            parseSelectSentence(token_stream);
        }
        else if(t.content == "insert"){
            parseInsertSentence(token_stream);
        }
        else if(t.content == "delete"){
            parseDeleteSentence(token_stream);
        }
        else if(t.content == "drop"){
            parseDropSentence(token_stream);
        }
        else if(t.content == "update"){
            parseUpdateSentence(token_stream);
        }
        else if(t.content == "create"){
            parseCreateSentence(token_stream);
        }
    }
    else{
        throw ParseError(t.content, "expect select / insert / delete / drop / create");
    }        
}

void Parser::parseCreateSentence(TokenStream & token_stream) {
    token_stream.pop_front();
    Token t = token_stream.pop_front();
    if(t.content == "index"){
        parseCreateIndexSentence(token_stream);
    }
    else if(t.content == "table"){
        parseCreateTableSentence(token_stream);
    }
    else{
        throw ParseError(t.content, "expect index / table after create");
    }
}

void Parser::parseDropSentence(TokenStream & token_stream) {
    token_stream.pop_front();
    Token t = token_stream.pop_front();
    if(t.content == "index"){
        parseDropIndexSentence(token_stream);
    }
    else if(t.content == "table"){
        parseDropTableSentence(token_stream);
    }
    else{
        throw ParseError(t.content, "expect index / table after drop");
    }
}

void Parser::parseSelectSentence(TokenStream & token_stream){
    ASTNodeStack s; 
    ParserSymbol::QueryState state = ParserSymbol::WAIT_SELECT;
    try{
        while(state != ParserSymbol::FINISH_QUERY){
            state = this->getQueryGenerator(state)->Accept(token_stream, s);
        }
        this->astree_ = ASTree(s.pop());
    }
    catch(const Exception & e){
        s.clear();
        this->astree_.destroy();
        throw e;
    }

    s.clear();
}

void Parser::parseDeleteSentence(TokenStream& token_stream){
    ASTNodeStack s;
    ParserSymbol::DeleteState state = ParserSymbol::WAIT_FROM_IN_DELETE;
    try{
        while(state != ParserSymbol::FINISH_DELETE){
            state = this->getDeleteGenerator(state)->Accept(token_stream, s);
        }
        this->astree_ = ASTree(s.pop());
    }
    catch(const Exception & e){
        s.clear();
        this->astree_.destroy();
        throw e;
    }

    s.clear();
}

void Parser::parseInsertSentence(TokenStream & token_stream) {
    ASTNodeStack s;
    ParserSymbol::InsertState state = ParserSymbol::WAIT_INSERT;
    try{
        while(state != ParserSymbol::FINISH_INSERT){
            state = this->getInsertGenerator(state)->Accept(token_stream, s);
        }
        this->astree_ = ASTree(s.pop());
    }
    catch(const Exception & e){
        s.clear();
        this->astree_.destroy();
        throw e;
    }

    s.clear();
}

void Parser::parseDropTableSentence(TokenStream & token_stream){
    Token tkn = token_stream.pop_front();
    const Token & lookahead = token_stream.front();
    if(tkn.type == Token::IDENTIFIER && lookahead.type ==Token::NONE){
        ASTreeNode* root = new ASTreeNode(ParserSymbol::drop_table,ParserSymbol::parallel);
        root->appendChild(new ASTreeNode(tkn));
        this->astree_ = ASTree(root);
    }
    else{
        throw ParseError(tkn.content, "expect table name");
    }
}

void Parser::parseDropIndexSentence(TokenStream & token_stream){
    Token tkn = token_stream.pop_front();
    const Token & lookahead = token_stream.front();
    if(tkn.type == Token::IDENTIFIER && lookahead.type ==Token::NONE){
        ASTreeNode* root = new ASTreeNode(ParserSymbol::drop_index,ParserSymbol::parallel);
        root->appendChild(new ASTreeNode(tkn));
        this->astree_ = ASTree(root);
    }
    else{
        throw ParseError(tkn.content, "expect table name");
    }
}

void Parser::parseCreateIndexSentence(TokenStream & token_stream){
    Token index_id_token = token_stream.pop_front();
    if(index_id_token.type != Token::IDENTIFIER){
        throw ParseError(index_id_token.content, "expect index name");
    }

    Token keyword_on_token = token_stream.pop_front();
    if(keyword_on_token.type == Token::KEYWORD && keyword_on_token.content == "on");
    else{
        throw ParseError(keyword_on_token.content, "expect on");
    }

    Token table_id_token = token_stream.pop_front();
    if(table_id_token.type != Token::IDENTIFIER){
        throw ParseError(table_id_token.content, "expect table name");
    }

    const Token & lookahead = token_stream.front();
    if(lookahead.type == Token::NONE){
        ASTreeNode* root = new ASTreeNode(ParserSymbol::create_index,ParserSymbol::parallel);
        root->appendChild(new ASTreeNode(index_id_token));
        root->appendChild(new ASTreeNode(table_id_token));
        this->astree_ = ASTree(root);
    }
    else{
        throw ParseError(lookahead.content, "end of create index");
    }
}
