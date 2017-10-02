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
    while(!this->error){
        swtich(state){
            case WAIT_SELECT: //1
                token_stream.pop_front();
                state = WAIT_ATTR_SET; 
                break; 

            case WAIT_ATTR_SET:  //2
                Token to_eat = token_stream.pop_front();
                if(to_eat->type == Token::IDENTIFIER){
                    s.push(new ASTreeNode(to_eat));
                    state = REDUCE_ATTR;
                }
                else{
                    this->setErrorInfo(to_eat.content, "wait attr set error");
                }
                break;

            case REDUCE_ATTR_ID://3
                const Token & lookahead = token_stream.front();
                //SLR
                if(lookahead->type == Token::SYMBOL && lookahead->content == "."){
                    token_stream.pop_front();
                    state = WAIT_ATTR_DOT_RIGHT;
                }
                else{
                    ASTreeNode attrIdNode = reduceAttrId(s);
                    if(s.empty() || s.top()->getTag() == ParserSymbol::attr){
                        // select attrID
                        state = WAIT_ATTR_AS; //6
                    }
                    else if(s.top()->getTag() == attr_set || s.top()->getTag() == condition){
                        state = WAIT_EUQALITY; //27
                    }
                    else if(s.top()->getTag() == attrID){
                        state = WAIT_EUQALITY_RIGHT;
                    }
                    else{
                        this->setErrorInfo("","reduce attr ID error");
                    }
                    s.push(attrIdNode);
                }
                break;

            case WAIT_ATTR_DOT_RIGHT: //4
                Token to_eat = token_stream.pop_front();
                if(to_eat->type == Token::IDENTIFIER){
                    s.push(new ASTreeNode(to_eat));
                    state = REDUCE_ADDR_ID_WITH_TABLE_ID;
                }
                else{
                    this->setErrorInfo(to_eat->content, "wait attr dot right value error");
                }
                break;

            case REDUCE_ADDR_ID_WITH_TABLE_ID: //5
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
                    this->setErrorInfo("","reduce attr ID with table ID error");
                }
                s.push(attrIdWithTableID);
                break;

            case REDUCE_ATTR: //6
                const Token & lookahead = token_stream.front();
                if(lookahead->type == Token::SYMBOL && lookahead->content == "as"){
                    token_stream.pop_front();
                    state = WAIT_ATTR_ALIAS; //7
                }
                else{
                    ASTreeNode* attr_node = reduceAttr(s);
                    s.push(attr_node);
                    state = REDUCE_ATTR_SET;
                }
                break;

            case WAIT_ATTR_ALIAS: //7
                Token to_eat = token_stream.pop_front();
                if(to_eat->type == Token::IDENTIFIER){
                    s.push(new ASTreeNode(to_eat));
                    state = REDUCE_ATTR_WITH_ALIAS;
                }
                else{
                    this->setErrorInfo(to_eat->content, "wait attr alias error")
                }
                break;

            case REDUCE_ATTR_WITH_ALIAS: //8
                ASTreeNode* attrNode_with_alias = reduceAttrWithAlias(s);
                s.push(attr_node);
                state = REDUCE_ATTR_SET;
                break;

            case REDUCE_ATTR_SET: //9
                const Token & lookahead = token_stream.front();
                if(lookahead->type == Token::SYMBOL && lookahead->content = ","){
                    token_stream.pop_front();
                    state = WAIT_ATTR_SET_AGAIN;
                }
                else{
                    ASTreeNode* attrSetNode = reduceAttrSetSingle(s);
                    if(s.empty()){
                        state = WAIT_FROM;
                    }
                    else if(s.top()->getTag() == ParserSymbol::attr){
                        state = REDUCE_ATTR_SET_AGAIN;
                    }
                    else{
                        this->setErrorInfo("", "reduce atrr set error"); 
                    }
                }
                break;

            case WAIT_ATTR_SET_AGAIN:
            /* redundant, same of state 2 */
                Token to_eat = token_stream.pop_front();
                if(to_eat.type == Token::IDENTIFIER){
                    s.push(new ASTreeNode(to_eat));
                    state = REDUCE_ATTR;
                }
                else{
                    this->setErrorInfo(to_eat->content, "reduce attr again error");
                }
                break;

            case REDUCE_ATTR_SET_AGAIN:
                ASTreeNode* attrSetNode = reduceAttrSetMultiple(s);
                if(s.top()->getTag() == ParserSymbol:attr){
                    state = REDUCE_ATTR_SET_AGAIN;
                }
                else{
                    state = WAIT_FROM;
                }
                s.push(attrSetNode);
                break;


            case WAIT_FROM:
            break;
        }
    }
}