#include "Reducer.h"

#include "ParserSymbol.h"
#include "ASTree.h"

ASTreeNode* reduceAttrId(stack<ASTreeNode*> & s){
    ASTreeNode* node_with_addrid = new ASTreeNode(attrID, single, s.pop());
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
    ASTreeNode* node_with_attr = new ASTreeNode(attr, single, s.pop());
    return node_with_attr;
}


ASTreeNode* reduceAttrWithAlias(stack<ASTreeNode*> & s){
    ASTreeNode* node_with_alias = s.pop();
    ASTreeNode* node_with_attrid = s.pop();
    ASTreeNode* node_with_attr = new ASTreeNode(attr, as, node_with_alias, node_with_attrid);
    return node_with_attr;
}

ASTreeNode* reduceAttrSetSingle(stack<ASTreeNode*> & s){
    ASTreeNode* node_with_attrset = new ASTreeNode(attr_set, single, s.pop());
    return  node_with_attrset;
}


ASTreeNode* reduceAttrSetMulti(stack<ASTreeNode*> & s){
    ASTreeNode* node_with_attrset = s.pop();
    ASTreeNode* node_with_attr = s.pop();
    return new ASTreeNode(attr_set, dot, node_with_attrset, node_with_attr);
}

ASTreeNode* reduceTable(stack<ASTreeNode> & s){
    ASTreeNode* node_with_tableid = s.pop();
    return new ASTreeNode(table, single, node_with_tableid);
}

