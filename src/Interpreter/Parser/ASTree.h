#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <stack>
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
    ~ASTreeNode(){}

    void appendChild(ASTreeNode* new_child){
        this->children_.push_back(new_child);
    }

    int childrenCount() const{
        return (int)(this->children_.size());
    }

    ParserSymbol::Tag getTag() const{
        return this->tag_;
    }
    ParserSymbol::Action getAction() const{
        return this->action_;
    }
    const std::string & getContent() const{
        return this->content_;
    }
    ASTreeNode* getChild(int i) const{
        return this->children_[i];
    }

    void free(){
        for(int i = 0; i < this->children_.size(); i++){
            this->children_[i]->free();
            delete this->children_[i];
        }
    }
private:
    ParserSymbol::Tag tag_;
    ParserSymbol::Action action_;
    std::string content_;

    std::vector<ASTreeNode*> children_;
};

class ASTree{
public:
    ASTree():root_(NULL){};
    ASTree(ASTreeNode* tree_root):root_(tree_root){}
    ~ASTree(){
 //       this->destroy();
    }

    ParserSymbol::Tag getType() const{
        if(this->root_){
            return this->root_->getTag();
        }
        else{
            return ParserSymbol::none;
        }
    }
    const ASTreeNode * getRoot() const{
        return this->root_;
    }
    void print() const;

    void destroy(){
        if(this->root_ != NULL){
            this->root_->free();
            delete this->root_;
            this->root_ = NULL;
        }
    }
private:
    ASTreeNode* root_;
};

#endif