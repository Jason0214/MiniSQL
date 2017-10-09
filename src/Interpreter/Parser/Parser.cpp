#include "Parser.h"

#include <string>
#include <stack>

using namespace std;

void Parser::generateParseTree(const std::vector<Token> & token_stream){
    parse_stack_.push_back(parseSentence(token_stream, 0));
}

void Parser::parseSentence(TokenStream & token_stream){
    Token t = token_stream.front()
    if(t.type == Token::KEYWORD){
        if(t.content == "select"){
            tree = parseSelectSentence(token_stream);
 
        }
        else if(t.content == "insert"){

        }
        else if(t.content == "delete"){

        }
        else if(t.content == "drop"){
            
        }
    }
    else{
        this->setErrorInfo(t.content, "expect select / insert/ delete/ drop");
    }        
}

void Parser::parseSelectSentence(TokenStream & token_stream){
    stack<ASTreeNode*> s; 
    SLRstate state = WAIT_SELECT;
    try{
        while(state != FINISH){
            state = this->getGenerator(state)->Accept(token_stream, s);
        }
        this->astree_ =  ASTree(s.pop());
    }
    catch(const ParseError & e){

    }


}