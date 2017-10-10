#include "ASTree.h"

#include <cassert>

using namespace ParserSymbol;

ASTreeNode::ASTreeNode(const Token & token):content_(token.content){
    switch(token.type){
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