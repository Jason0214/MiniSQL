#include "ASTree.h"

#include <cassert>
#include <list>

#include <iostream>

using namespace std;
using namespace ParserSymbol;

ASTreeNode::ASTreeNode(const Token & token):content_(token.content){
    switch(token.type){
        case Token::NONE:
            this->tag_ = none;
            break;
        case Token::IDENTIFIER:
            this->tag_ = identifier;
            break;
        case Token::INTS:
            this->tag_ = int_;
            break;
        case Token::FLOATS:
            this->tag_ = float_;
            break;
        case Token::STR:
            this->tag_ = str;
            break;
        default:
            assert(0 == 1);
            break;
    }
    this->action_ = (ParserSymbol::Action)0;
}


ASTreeNode::ASTreeNode(ParserSymbol::Tag tag, ParserSymbol::Action action)
:tag_(tag),
action_(action)
{}

ASTreeNode::ASTreeNode(ParserSymbol::Tag tag, ParserSymbol::Action action, ASTreeNode* child)
:tag_(tag),
action_(action)
{
    this->children_.push_back(child);   
}

ASTreeNode::ASTreeNode(ParserSymbol::Tag tag, ParserSymbol::Action action,
                            ASTreeNode* child1, ASTreeNode* child2)
:tag_(tag),
action_(action)
{
    this->children_.push_back(child1);
    this->children_.push_back(child2);
}


void ASTree::printTree()const{
    list<ASTreeNode*> queue;
    queue.push_back(this->root_);
    while(!queue.empty()){
        ASTreeNode* tmp = queue.front();
        queue.pop_front();
        switch(tmp->getTag){
            case none: cout << "\n"; delete tmp; break;
            case identifier: cout << "identifier "; break;
            case str: cout << "str "; break;
            case float_: cout << "float "; break;
            case int_: cout << "int "; break;
            case query: cout << "query ";break;
            case attr_set: cout << "attr_set "; break;
            case attrID: cout << "attrID"; break;
            case attr: cout << "attr "; break;
            case table_set: cout << "table_set "; break;
            case condition_set: cout << "condition_set "; break;
            case condition: cout << "condition "; break;
            case table: cout << "table"; break;
            case tableID: cout << "tableID"; break;
        }
        for(int i = 0; i < tmp->childrenCount(); i++){
            queue.push_back(tmp->getChild(i));
        }
        queue.push_back(new ASTreeNode(Token()));
    }
}