#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>

#include "../Lexer/Token.h"
#include "ParserSymbol.h"

class ASTreeNode{
public:
    ASTreeNode(const Token & token);
    ASTreeNode(ParserSymbol::Tag tag, ParserSymbol::Action action);
    ASTreeNode(ParserSymbol::Tag tag, ParserSymbol::Action action, ASTreeNode* child);
    ASTreeNode(ParserSymbol::Tag tag, ParserSymbol::Action action,
                ASTreeNode* child1, ASTreeNode* child2);
    ~ASTreeNode();

    void appendChild(ASTreeNode* new_child){

    }
    ParserSymbol::Tag getTag() const{
        return this->tag_;
    }
private:
    ParserSymbol::Tag tag_;
    ParserSymbol::Action action_;
    std::string content_;

    std::vector<ASTreeNode*> childrean_;
};

class ASTree{
public:
    ASTree():root_(NULL){};
    ASTree(ASTreeNode* tree_root):root_(tree_root){}

private:
    ASTreeNode* root_;
};


#endif