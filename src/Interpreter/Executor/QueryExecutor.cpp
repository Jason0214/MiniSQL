#include "QueryExecutor.h"

#include <string>
#include <sstream>

#include <iostream>

#include "../../EXCEPTION.h"
#include "../Parser/ParserSymbol.h"

using namespace std;
using namespace ParserSymbol;

string QueryExecutor::run(const ASTreeNode* node){
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
        freeExeTree(exe_tree);
        return ret;
    }
    catch(FalseCondition & e){
        freeExeTree(exe_tree);
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
    if(table_id_node->getChild(0)->getTag() == query){
        table_name = this->run(table_id_node->getChild(0));
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
        if(condition_node->getChild(0)->getAction() == dot){
            bool result = descendSelection(t, condition_node->getAction(),
                             condition_node->getChild(0), condition_node->getChild(1));
            if(!result){
                throw ExecuteError(condition_node->getChild(0)->getContent(), "table name not found");
            }
        }
        else{
            appendSelectArgs(t, condition_node->getAction(),
                             condition_node->getChild(0), condition_node->getChild(1));
        }
    }
    else if(condition_node->getChild(1)->getTag() == attrID){
        if(condition_node->getChild(1)->getAction() == dot){
            bool result = descendSelection(t, condition_node->getAction(),
                             condition_node->getChild(1), condition_node->getChild(0));
            if(!result){
                throw ExecuteError(condition_node->getChild(1)->getContent(), "table name not found");
            }
        }
        else{
            appendSelectArgs(t, condition_node->getAction(),
                            condition_node->getChild(1), condition_node->getChild(0));
        }
    }
    else{
        if(!checkEquality(condition_node->getAction(),
                         condition_node->getChild(0), condition_node->getChild(1))){
            throw FalseCondition();
        }
    }
}

bool QueryExecutor::checkEquality(Action equality, ASTreeNode* left_node, ASTreeNode* right_node){
    if(left_node->getTag() != right_node->getTag()){
        return false;
    }
    if(left_node->getTag() == float_){
        stringstream ltmp(left_node->getContent());
        stringstream rtmp(right_node->getContent());
        float lv,rv;
        ltmp >> lv;
        rtmp >> rv;
        switch(equality){
            case less_: return lv < rv;
            case less_equal_: return lv <= rv;
            case larger_: return lv > rv;
            case larger_equal_: return lv >= rv;
            case equal_: return lv == rv;
            case not_equal_: return lv != rv;
        }
    }
    else if(left_node->getTag() == int_){
        stringstream ltmp(left_node->getContent());
        stringstream rtmp(right_node->getContent());
        int lv,rv;
        ltmp >> lv;
        rtmp >> rv;
        switch(equality){
            case less_: return lv < rv;
            case less_equal_: return lv <= rv;
            case larger_: return lv > rv;
            case larger_equal_: return lv >= rv;
            case equal_: return lv == rv;
            case not_equal_: return lv != rv;
        }
    }
    else{
        switch(equality){
            case less_: return false;
            case less_equal_: return false;
            case larger_: return false;
            case larger_equal_: return false;
            case equal_: return left_node->getContent() == right_node->getContent();
            case not_equal_: return left_node->getContent() != right_node->getContent();
        }
    }
}

void QueryExecutor::appendSelectArgs(ExeTree* root, Action equality, ASTreeNode* attr, ASTreeNode* constant){
    Comparison comp;
    switch(equality){
        case less_: comp.Operation = "<"; break;
        case less_equal_: comp.Operation = "<="; break;
        case larger_: comp.Operation = ">"; break;
        case larger_equal_: comp.Operation = ">="; break;
        case equal_: comp.Operation = "="; break;
        case not_equal_: comp.Operation = "<>"; break;
    }
    comp.Comparand1.TypeName = "Attribute";
    comp.Comparand1.Content = attr->getChild(0)->getContent();

    switch(constant->getTag()){
        case int_:
            comp.Comparand2.TypeName = "int";
            break;
        case float_:
            comp.Comparand2.TypeName = "float";
            break;
        case str_:
            comp.Comparand2.TypeName = "string";
            break;
    }
    comp.Comparand2.Content = constant->getContent();

    root->select_args.push_back(comp);
}

bool QueryExecutor::descendSelection(ExeTree* t, Action equality, ASTreeNode* attr, ASTreeNode* constant){
    if(t == NULL){
        return false;
    }
    else{
        string id = attr->getChild(0)->getContent();
        bool res1 = descendSelection(t->right, equality, attr, constant);
        bool res2 = descendSelection(t->left, equality, attr, constant);
        if(this->table_alias_map_[t->table_name] == id){
            appendSelectArgs(t, equality, attr, constant);
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
        t->table_name = dst_table_name;
    }
    if(!t->project_args.empty()){
        string dst_table_name = this->getTmpTableName();
        cout << "do projection on " << "source table" << t->table_name << "; dst table" << dst_table_name << endl;
        t->table_name = dst_table_name;
    }
}

