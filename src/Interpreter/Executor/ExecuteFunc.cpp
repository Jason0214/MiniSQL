#include "ExecuteFunc.h"

#include <sstream>
#include "../../EXCEPTION.h"

using namespace std;
using namespace ParserSymbol;

void ExecuteFunc::parseCondition(ComparisonVector & cmpVec, const ASTreeNode* condition_node){
    if(condition_node->getChild(0)->getTag() == identifier && condition_node->getChild(1)->getTag() == identifier){
        throw TODO("compare between two attributes currently not supported");
    }
    else if(condition_node->getChild(0)->getTag() == identifier){
        cmpVec.emplace_back(ExecuteFunc::syntax2CmpSingleAttr(condition_node->getAction(),
                                                           condition_node->getChild(0), condition_node->getChild(1)));
    }
    else if(condition_node->getChild(1)->getTag() == identifier){
        cmpVec.emplace_back(ExecuteFunc::syntax2CmpSingleAttr(condition_node->getAction(),
                                                           condition_node->getChild(1), condition_node->getChild(0)));
    }
    else{
        if(!ExecuteFunc::checkEquality(condition_node->getAction(),
                                    condition_node->getChild(0), condition_node->getChild(1))){
            throw FalseCondition();
        }
    }
}

void ExecuteFunc::parseConditionSet(ComparisonVector & cmpVec, const ASTreeNode* condition_set_node){
    if(condition_set_node->childrenCount() == 1){
        parseCondition(cmpVec, condition_set_node->getChild(0));
        return;
    }
    if(condition_set_node->getAction() == and_){
        if(condition_set_node->getChild(0)->getTag() == condition_set
           && condition_set_node->getChild(1)->getTag() == condition_set){
            parseConditionSet(cmpVec, condition_set_node->getChild(0));
            parseConditionSet(cmpVec, condition_set_node->getChild(1));
        }
        else if(condition_set_node->getChild(0)->getTag() == condition_set){
            parseCondition(cmpVec, condition_set_node->getChild(1));
            parseConditionSet(cmpVec, condition_set_node->getChild(0));
        }
        else if(condition_set_node->getChild(1)->getTag() == condition_set){
            parseCondition(cmpVec, condition_set_node->getChild(0));
            parseConditionSet(cmpVec, condition_set_node->getChild(1));
        }
        else{
            parseCondition(cmpVec, condition_set_node->getChild(0));
            parseCondition(cmpVec, condition_set_node->getChild(1));
        }
    }
    else{
        throw TODO("keyword 'or' currently not supported");
    }
}

Comparison ExecuteFunc::syntax2CmpSingleAttr(Action equality, ASTreeNode* attr, ASTreeNode* constant){
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

bool ExecuteFunc::checkEquality(Action equality, ASTreeNode* left_node, ASTreeNode* right_node) {
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