#include "Reducer.h"

#include <stack>

#include "ParserSymbol.h"
#include "ASTree.h"

using namespace std;
using namespace ParserSymbol;

ASTreeNode* reduceAttrId(stack<ASTreeNode*> & s){
    ASTreeNode* node_with_addrid = new ASTreeNode(attrID, parallel, s.pop());
    return node_with_addrid;
}

ASTreeNode* reduceAttrIdWithTableId(stack<ASTreeNode*> & s){
    ASTreeNode* node_with_attr_id = s.pop();
    ASTreeNode* node_with_table_id = s.pop();
    ASTreeNode* node_table_dot_attr = 
            new ASTreeNode(attrID, dot, node_with_table_id, node_with_attr_id);

    return node_table_dot_attr;
}


ASTreeNode* reduceAttr(stack<ASTreeNode*> & s){
    ASTreeNode* node_with_attr = new ASTreeNode(attr, parallel, s.pop());
    return node_with_attr;
}


ASTreeNode* reduceAttrWithAlias(stack<ASTreeNode*> & s){
    ASTreeNode* node_with_alias = s.pop();
    ASTreeNode* node_with_attrid = s.pop();
    ASTreeNode* node_with_attr = new ASTreeNode(attr, as, node_with_alias, node_with_attrid);
    return node_with_attr;
}

ASTreeNode* reduceAttrSet(stack<ASTreeNode*> & s){
    ASTreeNode* node_with_attr_set = new ASTreeNode(attr_set, parallel);
    while(!s.empty()){
        node_with_attr_set->appendChild(new ASTreeNode(s.pop()));
    }
    return  node_with_attr_set;
}

ASTreeNode* reduceTableID(stack<ASTreeNode*> & s){
    ASTreeNode* node_with_id = s.pop();
    return new ASTreeNode(tableID, parallel, node_with_id);
}

ASTreeNode* reduceTableWithoutAlias(stack<ASTreeNode*> & s){
    ASTreeNode* node_with_table = new ASTreeNode(table, parallel, s.pop());
    return node_with_table;
}

ASTreeNode* reduceTableWithAlias(stack<ASTreeNode*> & s){
    ASTreeNode* node_with_alias = s.pop();
    ASTreeNode* node_with_tableid = s.pop();
    ASTreeNode* node_with_table = new ASTreeNode(table, as, s.pop(), s.pop());
}

ASTreeNode* reduceTableSet(stack<ASTreeNode*> & s){
    stack<ASTreeNode*> tmp_stack;
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
    return right_child;
}

ASTreeNode* reduceCondition(stack<ASTreeNode*> & s){
    ASTreeNode* right_child = s.pop();
    ASTreeNode* parent = s.pop();
    ASTreeNode* left_child = s.pop();
    parent.appendChild(left_child);
    parent.appendChild(right_child);
    return parent;
}

ASTreeNode* reduceConditionSet(stack<ASTreeNode*> & s){
    stack<ASTreeNode*> tmp_stack;
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
    return right_child;
}

ASTreeNode* reduceQuery(stack<ASTreeNode*> & s){
    ASTreeNode* condition_set_node = s.pop();
    ASTreeNode* table_set_node = s.pop();
    ASTreeNode* attr_set_node = s.pop();
    ASTreeNode* query_node = new ASTreeNode(query, parallel);
    query_node->appendChild(attr_set_node);
    query_node->appendChild(table_set_node);
    query_node->appendChild(condition_set_node);
    return query_node;
}