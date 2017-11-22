#include "DeleteExecutor.h"

#include <string>
#include "Executor.h"
#include "../Parser/ParserSymbol.h"

#include "../../API/APIFunctions.h"
#include "../../EXCEPTION.h"

using namespace std;
using namespace ParserSymbol;

static void parseCondition(ComparisonVector & cmpVec, const ASTreeNode* condition_node){
    if(condition_node->getChild(0)->getTag() == attrID && condition_node->getChild(1)->getTag() == attrID){
        throw TODO("compare between two attributes currently not supported");
    }
    else if(condition_node->getChild(0)->getTag() == attrID){
        cmpVec.emplace_back(Executor::syntax2CmpSingleAttr(condition_node->getAction(),
                               condition_node->getChild(0)->getChild(0), condition_node->getChild(1)));
    }
    else if(condition_node->getChild(1)->getTag() == attrID){
        cmpVec.emplace_back(Executor::syntax2CmpSingleAttr(condition_node->getAction(),
                               condition_node->getChild(1)->getChild(0), condition_node->getChild(0)));
    }
    else{
        if(!Executor::checkEquality(condition_node->getAction(),
                          condition_node->getChild(0), condition_node->getChild(1))){
            throw FalseCondition();
        }
    }
}

static void parseConditionSet(ComparisonVector & cmpVec, const ASTreeNode* condition_set_node){
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

void DeleteExecutor::run(const ASTreeNode* node){
    const string & table_name = node->getChild(1)->getContent();
    ComparisonVector cmpVec;
    try{
        parseConditionSet(cmpVec, node->getChild(0));
    }
    catch(FalseCondition &){
        return ;
    }
    ExeDelete(table_name, cmpVec);
}