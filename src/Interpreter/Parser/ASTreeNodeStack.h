#ifndef MINISQL_ASTREENODESTACK_H
#define MINISQL_ASTREENODESTACK_H

#include "ASTree.h"

class ASTNodeStack{
public:
    ASTNodeStack(){}
    ~ASTNodeStack(){}
    void push(ASTreeNode* n){
        this->stack_.push(n);
    }
    ASTreeNode* pop(){
        ASTreeNode* ret = this->stack_.top();
        this->stack_.pop();
        return ret;
    }
    ASTreeNode* top() const{
        return this->stack_.top();
    }
    bool empty() const{
        return this->stack_.empty();
    }
    int size() const{
        return (int)(this->stack_.size());
    }
    void clear() {
        while(!this->stack_.empty()){
            this->stack_.top()->free();
            delete this->stack_.top();
            this->stack_.pop();
        }
    }
private:
    std::stack<ASTreeNode*> stack_;
};



#endif //MINISQL_ASTREENODESTACK_H
