#ifndef __REDUCER_H__
#define __REDUCER_H__

#include "ASTree.h"
#include "ASTreeNodeStack.h"

ASTreeNode* reduceAttrId(ASTNodeStack & s);

ASTreeNode* reduceAttrIdWithTableId(ASTNodeStack & s);

ASTreeNode* reduceAttrWithoutAlias(ASTNodeStack & s);

ASTreeNode* reduceAttrWithAlias(ASTNodeStack & s);

ASTreeNode* reduceAttrSet(ASTNodeStack & s);

ASTreeNode* reduceTableID(ASTNodeStack & s);

ASTreeNode* reduceTableWithoutAlias(ASTNodeStack & s);

ASTreeNode* reduceTableWithAlias(ASTNodeStack & s);

ASTreeNode* reduceTableSet(ASTNodeStack & s);

ASTreeNode* reduceCondition(ASTNodeStack & s);

ASTreeNode* reduceConditionSet(ASTNodeStack & s);

ASTreeNode* reduceQueryWithCondition(ASTNodeStack & s);

ASTreeNode* reduceQueryWithoutCondition(ASTNodeStack & s);

ASTreeNode* reduceDelete(ASTNodeStack & s);

#endif