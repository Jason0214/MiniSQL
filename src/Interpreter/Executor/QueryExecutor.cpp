#include "QueryExecutor.h"

#include <string>
#include <sstream>

#include <iostream>

#include "Executor.h"
#include "../../EXCEPTION.h"
#include "../Parser/ParserSymbol.h"
#include "../../API/APIStructures.h"
#include "../../API/APIFunctions.h"

using namespace std;
using namespace ParserSymbol;

void QueryExecutor::run(const ASTreeNode* node){
    BeginQuery();
    string final_table;
    try{
        final_table = this->query(node);
    }
    catch(FalseCondition & ){
        EndQuery();
        this->clear();
        return ;
    }

    // ExeOutputTable(this->table_alias_map_, final_table);
    cout << "output table " << final_table << endl;
    EndQuery();
    this->clear();
}

std::string QueryExecutor::query(const ASTreeNode* node){
    // TODO: get estimate table size, single rotate AST to increase join efficiency
    ExeTree* exe_tree = NULL;
    try{
        exe_tree = this->parseTableSet(node->getChild(1));
        this->parseProjection(exe_tree, node->getChild(0));

        if(node->childrenCount() > 2){
            this->parseConditionSet(exe_tree, node->getChild(2)); //may do descend selection
        }
        this->joinTable(exe_tree);
        string ret = exe_tree->table_name;
        return ret;
    }
    catch(FalseCondition & e){
        freeExeTree(exe_tree);
        EndQuery();
        throw e;
    }
    catch (Exception & e){
        freeExeTree(exe_tree);
        throw e;
    }
}


void QueryExecutor::appendTableAlias(const string & alias, const string & name){
    TableAliasMap::iterator iter = this->table_alias_map_.find(alias);
    if(iter == this->table_alias_map_.end()){
        this->table_alias_map_[alias] = name;
    }
    else{
        if(iter->second != name){
            throw ExecuteError(alias, "duplicated table alias");
        }
    }
}


string QueryExecutor::parseTable(ASTreeNode* table_node){
    string table_alias;
    string table_name;
    ASTreeNode* table_id_node = table_node->getChild(0);
    // check if nested query
    if(table_id_node->getChild(0)->getTag() == query_){
        table_name = this->query(table_id_node->getChild(0));
    }
    else{
        table_name = table_id_node->getChild(0)->getContent();
    }
    // check if has alias
    if(table_node->childrenCount() == 1){
        table_alias = table_name;
    }
    else{
        table_alias = table_node->getChild(1)->getContent();
    }
    this->appendTableAlias(table_alias, table_name);
    return table_alias;
}

ExeTree* QueryExecutor::parseTableSet(ASTreeNode* node){
    ExeTree *t_right, *t_left;
    if(node->childrenCount() == 1){
        return new ExeTree(parseTable(node->getChild(0)));
    }
    if(node->getChild(0)->getTag() == table_set && node->getChild(1)->getTag() == table_set){
        t_right = this->parseTableSet(node->getChild(1));
        t_left = this->parseTableSet(node->getChild(0));
    }
    else if(node->getChild(0)->getTag() == table_set){
        t_right = new ExeTree(parseTable(node->getChild(1)));
        t_left = this->parseTableSet(node->getChild(0));
    }
    else if(node->getChild(1)->getTag() == table_set){
        t_right = this->parseTableSet(node->getChild(1));
        t_left = new ExeTree(parseTable(node->getChild(0)));
    }
    else{
        t_left = new ExeTree(parseTable(node->getChild(0)));
        t_right = new ExeTree(parseTable(node->getChild(1)));
    }
    return new ExeTree(t_left, t_right, node->getAction());
}

void QueryExecutor::parseConditionSet(ExeTree* t, ASTreeNode* condition_set_node){
    if(condition_set_node->childrenCount() == 1){
        parseCondition(t, condition_set_node->getChild(0));
        return;
    }
    if(condition_set_node->getAction() == and_){
        if(condition_set_node->getChild(0)->getTag() == condition_set
           && condition_set_node->getChild(1)->getTag() == condition_set){
            parseConditionSet(t, condition_set_node->getChild(0));
            parseConditionSet(t, condition_set_node->getChild(1));
        }
        else if(condition_set_node->getChild(0)->getTag() == condition_set){
            parseCondition(t, condition_set_node->getChild(1));
            parseConditionSet(t, condition_set_node->getChild(0));
        }
        else if(condition_set_node->getChild(1)->getTag() == condition_set){
            parseCondition(t, condition_set_node->getChild(0));
            parseConditionSet(t, condition_set_node->getChild(1));
        }
        else{
            parseCondition(t, condition_set_node->getChild(0));
            parseCondition(t, condition_set_node->getChild(1));
        }
    }
    else{
        throw TODO("keyword 'or' currently not supported");
    }
}

void QueryExecutor::parseCondition(ExeTree* t, ASTreeNode* condition_node){
    if(condition_node->getChild(0)->getTag() == attrID && condition_node->getChild(1)->getTag() == attrID){
        throw TODO("conditionally join currently not supported");
    }
    else if(condition_node->getChild(0)->getTag() == attrID){
        // attrID => ID . ID  (table.attr)
        // or attrID => ID  (attr)
        if(condition_node->getChild(0)->childrenCount() == 2){
            bool result = descendSelection(t, condition_node->getAction(),
                             condition_node->getChild(0), condition_node->getChild(1));
            if(!result){
                throw ExecuteError(condition_node->getChild(0)->getContent(), "table name not found");
            }
        }
        else{
            // attrID = constant
            t->select_args.emplace_back(
                    Executor::syntax2CmpSingleAttr(condition_node->getAction(),
                            condition_node->getChild(0)->getChild(0), condition_node->getChild(1))
            );
        }
    }
    else if(condition_node->getChild(1)->getTag() == attrID){
        if(condition_node->getChild(1)->childrenCount() == 2){
            bool result = descendSelection(t, condition_node->getAction(),
                             condition_node->getChild(1), condition_node->getChild(0));
            if(!result){
                throw ExecuteError(condition_node->getChild(1)->getContent(), "table name not found");
            }
        }
        else{
            // constant = attrID
            t->select_args.emplace_back(
                    Executor::syntax2CmpSingleAttr(condition_node->getAction(),
                             condition_node->getChild(1)->getChild(0), condition_node->getChild(0))
            );
        }
    }
    else{
        if(!Executor::checkEquality(condition_node->getAction(),
                         condition_node->getChild(0), condition_node->getChild(1))){
            throw FalseCondition();
        }
    }
}

bool QueryExecutor::descendSelection(ExeTree* t, Action equality, ASTreeNode* attr, ASTreeNode* constant){
    if(t == NULL){
        return false;
    }
    else{
        string table_alias = attr->getChild(0)->getContent();
        bool res1 = descendSelection(t->right, equality, attr, constant);
        bool res2 = descendSelection(t->left, equality, attr, constant);
        if(this->table_alias_map_[t->table_name] == table_alias){
            t->select_args.emplace_back(Executor::syntax2CmpSingleAttr(equality, attr->getChild(1), constant));
            return true;
        }
        else{
            return res1 || res2;
        }
    }
}

void QueryExecutor::parseProjection(ExeTree* root, ASTreeNode* attr_set_node){
    if(attr_set_node->getTag() == star){
        return;
    }
    for(int i = 0; i < attr_set_node->childrenCount(); i++){
        AttrNameAlias args;
        ASTreeNode* attr_node = attr_set_node->getChild(i);
        if(attr_node->getAction() == as){
            args.AttrAlias = attr_node->getChild(1)->getContent();
            if(attr_node->getAction() == dot){
                throw TODO("projection attributes specified table ID currently not supported");
            }
            else{
                args.AttrName = attr_node->getChild(0)->getChild(0)->getContent();
            }
        }
        else{
            if(attr_node->getAction() == dot){
                throw TODO("projection attributes specified table ID currently not supported");
            }
            else{
                args.AttrAlias = attr_node->getChild(0)->getChild(0)->getContent();
                args.AttrName = attr_node->getChild(0)->getChild(0)->getContent();
            }
        }
        root->project_args.push_back(args);
    }
}

string QueryExecutor::getTmpTableName() {
    string ret = to_string(this->tmp_table_cnt_) + "TmpTable";
    this->tmp_table_cnt_++;
    return ret;
}

void QueryExecutor::joinTable(ExeTree* t){
    if(!(t->left == NULL && t->right == NULL)){
        joinTable(t->left);
        joinTable(t->right);
        string dst_table_name = this->getTmpTableName();
        cout << "join two tables: " << t->left->table_name << ","<<t->right->table_name<<" ;dst table" << dst_table_name <<endl;
        t->table_name = dst_table_name;
    }
    if(!t->select_args.empty()){
        string dst_table_name = this->getTmpTableName();
        cout << "do selection on " << "source table" << t->table_name << "; dst table" << dst_table_name << endl;
        for(int i = 0; i < t->select_args.size(); i++){
            cout << "left: " <<  t->select_args[i].Comparand1.TypeName;
            cout << "right: " << t->select_args[i].Comparand2.TypeName;
            cout << endl;
        }
        t->table_name = dst_table_name;
    }
    if(!t->project_args.empty()){
        string dst_table_name = this->getTmpTableName();
        cout << "do projection on " << "source table" << t->table_name << "; dst table" << dst_table_name << endl;
        cout << "attributes: ";
        for(int i = 0; i < t->project_args.size(); i++){
            cout << t->project_args[i].AttrName;
        }
        cout << endl;
        t->table_name = dst_table_name;
    }
}

