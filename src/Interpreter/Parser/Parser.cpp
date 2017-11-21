#include "Parser.h"

#include <string>
#include <stack>


#include <iostream>

#include "ASTreeNodeStack.h"
#include "../../EXCEPTION.h"

using namespace std;

// branch decision
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


// LR(1) parsing
void Parser::parseSelectSentence(TokenStream & token_stream){
    ASTNodeStack s; 
    try{
        this->query_generator_.Accept(token_stream, s);
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
    try{
        this->delete_generator_.Accept(token_stream, s);
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
    try{
        this->insert_generator_.Accept(token_stream, s);
        this->astree_ = ASTree(s.pop());
    }
    catch(const Exception & e){
        s.clear();
        this->astree_.destroy();
        throw e;
    }

    s.clear();
}

void Parser::parseCreateTableSentence(TokenStream &token_stream) {
    ASTNodeStack s;
    try{
        this->create_table_generator_.Accept(token_stream, s);
        this->astree_ = ASTree(s.pop());
    }
    catch(const Exception & e){
        s.clear();
        this->astree_.destroy();
        throw e;
    }

    s.clear();
}

void Parser::parseUpdateSentence(TokenStream &token_stream) {
    ASTNodeStack s;
    try{
        this->update_generator_.Accept(token_stream, s);
        this->astree_ = ASTree(s.pop());
    }
    catch(const Exception & e){
        s.clear();
        this->astree_.destroy();
        throw e;
    }

    s.clear();
}

// simple parsing
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
