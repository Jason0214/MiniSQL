#include "Executor.h"
#include <sstream>

using namespace std;
using namespace ParserSymbol;

Comparison Executor::syntax2CmpSingleAttr(Action equality, ASTreeNode* attr, ASTreeNode* constant){
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
    comp.Comparand1.Content = attr->getContent();

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
    return comp;
}

bool Executor::checkEquality(Action equality, ASTreeNode* left_node, ASTreeNode* right_node) {
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