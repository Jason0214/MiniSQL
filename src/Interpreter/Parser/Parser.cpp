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
    stack<ASTNode> s; 
    SLRstate state = WAIT_SELECT;
    Token tmp;
    while(!this->error){
        swtich(state){
            case WAIT_SELECT: state = WAIT_ATTR_SET; break;
            case WAIT_ATTR_SET: 
                tmp = token_stream.pop_front();
                if(tmp->type == Token::IDENTIFIER){
                    s.push(ASTNode(tmp));
                    state = REDUCE_ATTR;
                }
                else{
                    this->setErrorInfo(tmp.content, "not recognized");
                }
                break;
            case REDUCE_ATTR:
                tmp = token_stream.front();
                //SLR
                if(tmp->type == Token::KEYWORD && tmp->content == "as"){
                    token_stream.pop_front();
                    state = WAIT_ATTR_ALIAS;
                }
                else{
                    reduceAttr(ASTree, s);
                    state = REDUCE_ATTR_SET;
                }
                break;
            case WAIT_ATTR_ALIAS:
                tmp = token_stream.pop_front();
                if(tmp->type == Token::IDENTIFIER){
                    s.push(ASTNode(tmp));
                    state = REDUCE_ATTR_WITH_ALIAS;
                }
            case REDUCE_ATTR_WITH_ALIAS:
                reduceAttrwithAlias(ASTree, s);
                state = REDUCE_ATTR_SET;
                break;
            case REDUCE_ATTR_SET:
                tmp = token_stream.front();
                //SLR
                if(tmp->type == Token::SYMBOL && tmp->content == ","){
                    token_stream.pop_front();
                    state = WAIT_ATTR_SET_AGAIN;
                }
                else{
                    reduceAttrSet(ASTree, s);
                    state = WAIT_FROM;
                }
                break;
            case WAIT_ATTR_SET_AGAIN:
                tmp = token_stream.pop_front();
                if(tmp.type == Token::IDENTIFIER){
                    s.push(ASTree(tmp));
                    state = REDUCE_ATTR;
                }

        }
    }
}