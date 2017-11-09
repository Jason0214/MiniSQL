#include "Parser.h"

#include <string>
#include <stack>


#include <iostream>

#include "../../EXCEPTION.h"

using namespace std;

Generator::QueryGenerator* Parser::getGenerator(ParserSymbol::QueryState state){
    return this->query_generators_[(unsigned int)state];
}

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

        }
        else if(t.content == "delete"){
            parseDeleteSentence(token_stream);
        }
        else if(t.content == "drop"){
            
        }
        else if(t.content == "update"){

        }
    }
    else{
        throw ParseError(t.content, "expect select / insert / delete / drop");
    }        
}

void Parser::parseSelectSentence(TokenStream & token_stream){
    ASTNodeStack s; 
    ParserSymbol::QueryState state = ParserSymbol::WAIT_SELECT;
    try{
        while(state != ParserSymbol::FINISH){
            state = this->getGenerator(state)->Accept(token_stream, s);
        }
        this->astree_ = ASTree(s.pop(), "select");
    }
    catch(const Exception & e){
        while(!s.empty()){
            s.top()->free();
            delete s.top();
            s.pop();
        }
        this->astree_.destroy();
        throw e;
    }

    // free
    while(!s.empty()){
        s.top()->free();
        delete s.top();
        s.pop();
    }
}

void Parser::parseDeleteSentence(TokenStream& token_stream){

}
