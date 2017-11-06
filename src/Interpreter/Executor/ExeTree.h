#ifndef __EXE_TREE_H__
#define __EXE_TREE_H__

#include <vector>
#include <string>
#include "../../API/APIStructures.h"
#include "../Parser/ParserSymbol.h"

struct ExeTree{
    ExeTree(std::string table_name){
        this->table_name = table_name;
        this->join_type = (ParserSymbol::Action)0;
        this->left = NULL;
        this->right = NULL;
    }
    ExeTree(ExeTree* left, ExeTree* right, ParserSymbol::Action type){
        this->left = left;
        this->right = right;
        this->join_type = type;
    }
    ExeTree* left;
    ExeTree* right;
    std::vector<Comparison> select_args;
    std::vector<AttrNameAlias> project_args;

    std::string table_name;
    ParserSymbol::Action join_type;
};

void freeExeTree(ExeTree* t);

#endif