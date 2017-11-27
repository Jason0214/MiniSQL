#include "Reducer.h"

#include "ParserSymbol.h"
#include "ASTree.h"

using namespace std;
using namespace ParserSymbol;

ASTreeNode* reduceAttrId(ASTNodeStack & s){
    ASTreeNode* node_with_attr = s.pop();
    ASTreeNode* node_with_attrid = new ASTreeNode(attrID, parallel, node_with_attr) ;
    return node_with_attrid;
    // child 0: attribute name
}

ASTreeNode* reduceAttrIdWithTableId(ASTNodeStack & s){
    ASTreeNode* node_with_attr_id = s.pop();
    ASTreeNode* node_with_table_id = s.pop();
    ASTreeNode* node_table_dot_attr = 
            new ASTreeNode(attrID, dot, node_with_table_id, node_with_attr_id);

    return node_table_dot_attr;
    //child 0: table name
    //child 1: attribute name
}


ASTreeNode* reduceAttrWithoutAlias(ASTNodeStack & s){
    ASTreeNode* node_with_attr = new ASTreeNode(attr, parallel, s.pop());
    return node_with_attr;
    // child 0: attribute(either T.A or A)
}


ASTreeNode* reduceAttrWithAlias(ASTNodeStack & s){
    ASTreeNode* node_with_alias = s.pop();
    ASTreeNode* node_with_attrid = s.pop();
    ASTreeNode* node_with_attr = new ASTreeNode(attr, as, node_with_attrid,  node_with_alias);
    return node_with_attr;
    // child 0: attribute
    // child 1: attribute alias
}

ASTreeNode* reduceAttrSet(ASTNodeStack & s){
    ASTreeNode* node_with_attr_set = new ASTreeNode(attr_set, parallel);
    while(!s.empty() && s.top()->getTag() == attr){
        node_with_attr_set->appendChild(s.pop());
    }
    return  node_with_attr_set;
    // children: a vector of attributes (either 'A as A' or 'A')
}

ASTreeNode* reduceTableID(ASTNodeStack & s){
    ASTreeNode* node_with_id = s.pop();
    return new ASTreeNode(tableID, parallel, node_with_id);
    //child 0: table name or a nested query
}

ASTreeNode* reduceTableWithoutAlias(ASTNodeStack & s){
    ASTreeNode* node_with_table = new ASTreeNode(table, parallel, s.pop());
    return node_with_table;
    //child 0: table
}

ASTreeNode* reduceTableWithAlias(ASTNodeStack & s){
    ASTreeNode* node_with_alias = s.pop();
    ASTreeNode* node_with_tableid = s.pop();
    ASTreeNode* node_with_table = new ASTreeNode(table, as, node_with_tableid, node_with_alias);
    return node_with_table;
    // child 0: table
    // child 1: table alias
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
    // children:: a tree of table
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
    ASTreeNode* query_node = new ASTreeNode(query_, parallel);
    query_node->appendChild(attr_set_node);
    query_node->appendChild(table_set_node);
    query_node->appendChild(condition_set_node);
    return query_node;
}

ASTreeNode* reduceQueryWithoutCondition(ASTNodeStack & s){
    ASTreeNode* table_set_node = s.pop();
    ASTreeNode* attr_set_node = s.pop();
    ASTreeNode* query_node = new ASTreeNode(query_, parallel);
    query_node->appendChild(attr_set_node);
    query_node->appendChild(table_set_node);
    return query_node;
}

ASTreeNode* reduceDelete(ASTNodeStack & s){
    ASTreeNode* table_id_node = s.pop();
    ASTreeNode* condition_set_node = s.pop();
    ASTreeNode* delete_node = new ASTreeNode(delete_, parallel);
    delete_node->appendChild(table_id_node);
    delete_node->appendChild(condition_set_node);
    return delete_node;
}

ASTreeNode* reduceValueSet(ASTNodeStack & s){
    ASTreeNode* value_set_node = new ASTreeNode(value_set, parallel);
    while(s.size() > 1){
        value_set_node->appendChild(s.pop());
    }
    return value_set_node;
}

ASTreeNode* reduceInsert(ASTNodeStack & s){
    ASTreeNode* insert_node = new ASTreeNode(insert_, parallel);
    insert_node->appendChild(s.pop());
    insert_node->appendChild(s.pop());
    return insert_node;
}

ASTreeNode* reduceType(ASTNodeStack & s){
    ASTreeNode* type_node = NULL;
    if(s.top()->getTag() != type){
        ASTreeNode* param_node = s.pop();
        type_node = s.pop();
        type_node->appendChild(param_node);
    }
    else{
        type_node = s.pop();
    }
    return type_node;
}

ASTreeNode* reduceMeta(ASTNodeStack & s){
    ASTreeNode* meta_node = new ASTreeNode(meta, parallel);
    while(s.top()->getTag() != identifier){
        meta_node->appendChild(s.pop());
    }
    meta_node->appendChild(s.pop());
    return meta_node;
}

ASTreeNode* reduceMetaSet(ASTNodeStack & s){
    ASTreeNode* meta_set_node = new ASTreeNode(meta_set, parallel);
    while(s.top()->getTag() == meta){
        meta_set_node->appendChild(s.pop());
    }
    return meta_set_node;
}

ASTreeNode* reduceCreateTable(ASTNodeStack & s){
    ASTreeNode* create_table_node = new ASTreeNode(create_table_, parallel);
    create_table_node->appendChild(s.pop());
    create_table_node->appendChild(s.pop());
    return create_table_node;
}

ASTreeNode* reduceAssign(ASTNodeStack & s){
    ASTreeNode* assign_node = new ASTreeNode(assign, parallel);
    assign_node->appendChild(s.pop());
    assign_node->appendChild(s.pop());
    return assign_node;
}

ASTreeNode* reduceAssignSet(ASTNodeStack & s){
    ASTreeNode* assign_set_node = new ASTreeNode(assign_set, parallel);
    while(s.top()->getTag() == assign){
        assign_set_node->appendChild(s.pop());
    }
    return assign_set_node;
}

ASTreeNode* reduceUpdate(ASTNodeStack & s){
    ASTreeNode* update_node = new ASTreeNode(update_, parallel);
    update_node->appendChild(s.pop());
    update_node->appendChild(s.pop());
    update_node->appendChild(s.pop());
    return update_node;
}

ASTreeNode* reduceColumnSet(ASTNodeStack & s){
    ASTreeNode* column_set_node = new ASTreeNode(attr_set, parallel);
    while(s.size() > 2){
        column_set_node->appendChild(s.pop());
    }
    return column_set_node;
}

ASTreeNode* reduceCreateIndex(ASTNodeStack & s){
    ASTreeNode* create_index_node = new ASTreeNode(create_index, parallel);
    while(!s.empty()){
        create_index_node->appendChild(s.pop());
    }
    return  create_index_node;
}
