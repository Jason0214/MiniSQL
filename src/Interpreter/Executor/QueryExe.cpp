#include "QueryExe.h"

#include <string>

#include "../Parser/ParserSymbol.h"

using namespace std;
using namespace ParserSymbol;

string QueryExe::run(const ASTreeNode* node){
    // TODO: get estimate table size, single rotate AST to increase join efficiency.

    ExeTree* exe_Tree = this->parseTableSet(t->getChild(1));
    this->descendProjection(exe_Tree, t->getChild(0));
    this->descendSelection(exe_Tree, t->getChild(2));
    string ret = this->joinTable(exe_Tree);
    ExeTreeFree(exe_Tree);
    return ret;
}

string QueryExe::mapTableAlias(ASTreeNode* table_node){
    if(table_node.getChild(0).getTag == query){
        // nested query
        string table_name = this->run(table_node->getChild(0));
        if(table_node.childrenCount() == 2){
            string table_alias = table_node->getChild(1)->getContent();
            this->mapAppendMatch(table_alias, table_name);
        }
        else{
            throw MissAlias();
        }
    }
    else{
        if(table_node.childrenCount() == 1){
            string alias_and_name = table_node->getChild(0)->getContent();
            this->mapAppendMatch(alias_and_name, alias_and_name);
        }
        else{
            string table_alias = table_node->getChild(0)->getContent();
            string table_name = table_node->getChild(1)->getContent()
            this->mapAppendMatch(table_alias, table_name);
        }
    }

}

ExeTree* QueryExe::parseTableSet(ASTreeNode* node){
    if(node->getChild(1)->getTag() == table_set){
        ExeTree* t_right = this->parseTableSet(node->getChild(1));
        ExeTree* t_left = new ExeTree(mapTableAlias(node->getChild(0)));
        return new ExeTree(t_left, t_right, node->getAction());
    }
    else{
        ExeTree* t_left = new ExeTree(mapTableAlias(node->getChild(0)));
        ExeTree* t_right = new ExeTree(mapTableAlias(node->getChild(1)));
        return new ExeTree(t_left, t_right, node->getAction());
    }
}