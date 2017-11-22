#ifndef MINISQL_EXECUTOR_H
#define MINISQL_EXECUTOR_H

#include "../Parser/ASTree.h"
#include "../../API/APIStructures.h"

class Executor{
public:
    static Comparison syntax2CmpSingleAttr(ParserSymbol::Action equality, ASTreeNode* attr, ASTreeNode* constant);
    static bool checkEquality(ParserSymbol::Action equality, ASTreeNode* left_node, ASTreeNode* right_node);
};

#endif //MINISQL_EXECUTOR_H
