#ifndef __QUERY_EXE_H__
#define __QUERY_EXE_H__

#include <map>
#include <string>
#include "ExeTree.h"
#include "../Parser/ASTree.h"
#include "../../API/APIStructures.h"

class QueryExecutor{
public:
    QueryExecutor(){
        this->tmp_table_cnt_ = 0;
    }
    ~QueryExecutor(){}
    void clear(){
        this->table_alias_map_.clear();
        this->tmp_table_cnt_  = 0;
    }
    std::string run(const ASTreeNode* node);
    void outputTable(std::string & table_name){};
private:
    std::string parseTable(ASTreeNode* table_node);
    ExeTree* parseTableSet(ASTreeNode* node);
    void parseConditionSet(ExeTree* t, ASTreeNode* condition_set_node);
    void parseCondition(ExeTree* t, ASTreeNode* condition_node);
    bool checkEquality(ParserSymbol::Action equality, ASTreeNode* left_node, ASTreeNode* right_node);
    void appendSelectArgs(ExeTree* root, ParserSymbol::Action equality, ASTreeNode* attr, ASTreeNode* constant);
    bool descendSelection(ExeTree* t, ParserSymbol::Action equality, ASTreeNode* attr, ASTreeNode* constant);
    void parseProjection(ExeTree* root, ASTreeNode* attr_set_node);
    void joinTable(ExeTree* t);

    void appendTableAlias(const std::string & alias, const std::string & name);
    std::string getTmpTableName();

    TableAliasMap table_alias_map_;
    int tmp_table_cnt_;
};

#endif