#include <string>


#include "ParserSymbol.h"

class ASTreeNode{
public:
    ASTreeNode(const Token & token);
    ASTreeNode(ParserSymbol::Tag tag, ParserSymbol::Action action, ASTreeNode* child);
    ASTreeNode(ParserSymbol::Tag tag, ParserSymbol::Action action,
                ASTreeNode* left_child, ASTreeNode* right_child);
    ~ASTreeNode();

    ParserSymbol::Tag getTag() const{
        return this->tag_;
    }
private:
    ParserSymbol::Tag tag_;
    ParserSymbol::Action action_;
    std::string content_;

    ASTreeNode* left_child_;
    ASTreeNode* right_child_;
};

class ASTree{
public:
    ASTree():root_(NULL){};
    ASTree(ASTreeNode* tree_root):(tree_root){}

private:
    ASTreeNode* root_
};