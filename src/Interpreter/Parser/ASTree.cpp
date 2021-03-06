#include "ASTree.h"

#include <cassert>
#include <list>


using namespace std;
using namespace ParserSymbol;


#define VALIDATE_ASTREE

#ifdef VALIDATE_ASTREE

    #include <iostream>

#endif

ASTreeNode::ASTreeNode(const Token & token):content_(token.content){
    switch(token.type){
        case Token::IDENTIFIER:
            this->tag_ = identifier;
            break;
        case Token::INTS:
            this->tag_ = int_;
            break;
        case Token::FLOATS:
            this->tag_ = float_;
            break;
        case Token::STR:
            this->tag_ = str_;
            break;
        default:
            assert(0 == 1);
            break;
    }
    this->action_ = (ParserSymbol::Action)0;
}


ASTreeNode::ASTreeNode(ParserSymbol::Tag tag, ParserSymbol::Action action)
:tag_(tag),
action_(action)
{}

ASTreeNode::ASTreeNode(ParserSymbol::Tag tag, ParserSymbol::Action action, ASTreeNode* child)
:tag_(tag),
action_(action)
{
    this->children_.push_back(child);   
}

ASTreeNode::ASTreeNode(ParserSymbol::Tag tag, ParserSymbol::Action action,
                            ASTreeNode* child1, ASTreeNode* child2)
:tag_(tag),
action_(action)
{
    this->children_.push_back(child1);
    this->children_.push_back(child2);
}


#ifdef VALIDATE_ASTREE

void ASTree::print()const{
    list<ASTreeNode*> queue;
    queue.push_back(this->root_);
    while(!queue.empty()){
        ASTreeNode* tmp = queue.front();
        queue.pop_front();
        for(int i = 0; i < tmp->childrenCount(); i++){
            queue.push_back(tmp->getChild(i));
        }
        switch(tmp->getTag()){
            case none: cout << "none "; break;
            case identifier: cout << "identifier "; break;
            case str_: cout << "str "; break;
            case float_: cout << "float "; break;
            case int_: cout << "int "; break;
            case query_: cout << "query ";break;
            case attr_set: cout << "attr_set "; break;
            case attrID: cout << "attrID "; break;
            case attr: cout << "attr "; break;
            case table_set: cout << "table_set "; break;
            case condition_set: cout << "condition_set "; break;
            case condition: cout << "condition "; break;
            case table: cout << "table "; break;
            case tableID: cout << "tableID "; break;
            case delete_:cout << "delete "; break;
            case insert_:cout << "insert "; break;
            case update_:cout << "update "; break;
            case create_table_:cout << "create_table "; break;
            case drop_table:cout << "drop_table "; break;
            case drop_index:cout << "drop_index "; break;
            case create_index:cout << "create_index "; break;
            case type: cout << "type "; break;
            case constrain: cout << "constrain "; break;
            case meta: cout << "meta "; break;
            case assign: cout << "assign "; break;
            case meta_set:cout << "meta_set "; break;
            case assign_set:cout << "assign_set "; break;
            case value_set: cout << "value_set "; break;
        }
    }
    cout << endl;
}

#endif