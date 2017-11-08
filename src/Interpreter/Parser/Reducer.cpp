#include "Reducer.h"

#include "ParserSymbol.h"
#include "ASTree.h"

using namespace std;
using namespace ParserSymbol;

ASTreeNode* reduceAttrId(ASTNodeStack & s){
    ASTreeNode* node_with_attr = s.pop();
    ASTreeNode* node_with_attrid = new ASTreeNode(attrID, parallel, node_with_attr) ;
    return node_with_attrid;
}

ASTreeNode* reduceAttrIdWithTableId(ASTNodeStack & s){
    ASTreeNode* node_with_attr_id = s.pop();
    ASTreeNode* node_with_table_id = s.pop();
    ASTreeNode* node_table_dot_attr = 
            new ASTreeNode(attrID, dot, node_with_table_id, node_with_attr_id);

    return node_table_dot_attr;
}


ASTreeNode* reduceAttrWithoutAlias(ASTNodeStack & s){
    ASTreeNode* node_with_attr = new ASTreeNode(attr, parallel, s.pop());
    return node_with_attr;
}


ASTreeNode* reduceAttrWithAlias(ASTNodeStack & s){
    ASTreeNode* node_with_alias = s.pop();
    ASTreeNode* node_with_attrid = s.pop();
    ASTreeNode* node_with_attr = new ASTreeNode(attr, as, node_with_alias, node_with_attrid);
    return node_with_attr;
}

ASTreeNode* reduceAttrSet(ASTNodeStack & s){
    ASTreeNode* node_with_attr_set = new ASTreeNode(attr_set, parallel);
    while(!s.empty() && s.top()->getTag() == attr){
        node_with_attr_set->appendChild(s.pop());
    }
    return  node_with_attr_set;
}

ASTreeNode* reduceTableID(ASTNodeStack & s){
    ASTreeNode* node_with_id = s.pop();
    return new ASTreeNode(tableID, parallel, node_with_id);
}

ASTreeNode* reduceTableWithoutAlias(ASTNodeStack & s){
    ASTreeNode* node_with_table = new ASTreeNode(table, parallel, s.pop());
    return node_with_table;
}

ASTreeNode* reduceTableWithAlias(ASTNodeStack & s){
    ASTreeNode* node_with_alias = s.pop();
    ASTreeNode* node_with_tableid = s.pop();
    ASTreeNode* node_with_table = new ASTreeNode(table, as, node_with_tableid, node_with_alias);
    return node_with_table;
}

ASTreeNode* reduceTableSet(ASTNodeStack & s){
    ASTNodeStack tmp_stack;
    tmp_stack.push(s.pop());
    while(s.top()->getTag() == table_set){
        tmp_stack.push(s.pop());
        tmp_stack.push(s.pop());
    }
    ASTreeNode* right_child = tmp_stack.pop();
    while(!tmp_stack.empty()){
        ASTreeNode* table_set_node = tmp_stack.pop();
        ASTreeNode* left_child = tmp_stack.pop();
        table_set_node->appendChild(left_child);
        table_set_node->appendChild(right_child);
        right_child = table_set_node;
    }
    if(right_child->getTag() == table){
    // only one table in table_set
        right_child = new ASTreeNode(table_set, join, right_child);
    }
    return right_child;
}

ASTreeNode* reduceCondition(ASTNodeStack & s){
    ASTreeNode* right_child = s.pop();
    ASTreeNode* parent = s.pop();
    ASTreeNode* left_child = s.pop();
    parent->appendChild(left_child);
    parent->appendChild(right_child);
    return parent;
}

ASTreeNode* reduceConditionSet(ASTNodeStack & s){
    ASTNodeStack tmp_stack;
    tmp_stack.push(s.pop());
    while(s.top()->getTag() == condition_set){
        tmp_stack.push(s.pop());
        tmp_stack.push(s.pop());
    }
    ASTreeNode* right_child = tmp_stack.pop();
    while(!tmp_stack.empty()){
        ASTreeNode* table_set_node = tmp_stack.pop();
        ASTreeNode* left_child = tmp_stack.pop();
        table_set_node->appendChild(left_child);
        table_set_node->appendChild(right_child);
        right_child = table_set_node;
    }
    if(right_child->getTag() == condition){
        // only one table in table_set
        right_child = new ASTreeNode(condition_set, and_, right_child);
    }
    return right_child;
}

ASTreeNode* reduceQueryWithCondition(ASTNodeStack & s){
    ASTreeNode* condition_set_node = s.pop();
    ASTreeNode* table_set_node = s.pop();
    ASTreeNode* attr_set_node = s.pop();
    ASTreeNode* query_node = new ASTreeNode(query, parallel);
    query_node->appendChild(attr_set_node);
    query_node->appendChild(table_set_node);
    query_node->appendChild(condition_set_node);
    return query_node;
}

ASTreeNode* reduceQueryWithoutCondition(ASTNodeStack & s){
    ASTreeNode* table_set_node = s.pop();
    ASTreeNode* attr_set_node = s.pop();
    ASTreeNode* query_node = new ASTreeNode(query, parallel);
    query_node->appendChild(attr_set_node);
    query_node->appendChild(table_set_node);
    return query_node;
}