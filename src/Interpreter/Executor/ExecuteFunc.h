#ifndef MINISQL_EXECUTEFUNC_H
#define MINISQL_EXECUTEFUNC_H

#include "../../API/APIStructures.h"
#include "../Parser/ParserSymbol.h"
#include "../Parser/ASTree.h"

namespace ExecuteFunc{
    Comparison syntax2CmpSingleAttr(ParserSymbol::Action equality, ASTreeNode* attr, ASTreeNode* constant);
    bool checkEquality(ParserSymbol::Action equality, ASTreeNode* left_node, ASTreeNode* right_node);
    void parseCondition(ComparisonVector & cmpVec, const ASTreeNode* condition_node);
    void parseConditionSet(ComparisonVector & cmpVec, const ASTreeNode* condition_set_node);
}

#endif //MINISQL_EXECUTEFUNC_H
