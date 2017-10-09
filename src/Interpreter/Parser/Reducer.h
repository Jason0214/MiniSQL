#ifndef __REDUCER_H__
#define __REDUCER_H__

#include <stack>

#include "ASTree.h"

ASTreeNode* reduceAttrId(std::stack<ASTreeNode*> & s);
ASTreeNode* reduceAttrIdWithTableId(std::stack<ASTreeNode*> & s);


ASTreeNode* reduceAttr(std::stack<ASTreeNode*> & s);


ASTreeNode* reduceAttrWithAlias(std::stack<ASTreeNode*> & s);

ASTreeNode* reduceAttrSet(std::stack<ASTreeNode*> & s);

ASTreeNode* reduceTableID(std::stack<ASTreeNode*> & s);

ASTreeNode* reduceTableWithoutAlias(std::stack<ASTreeNode*> & s);

ASTreeNode* reduceTableWithAlias(std::stack<ASTreeNode*> & s);

ASTreeNode* reduceTableSet(std::stack<ASTreeNode*> & s);

ASTreeNode* reduceCondition(std::stack<ASTreeNode*> & s);

ASTreeNode* reduceConditionSet(std::stack<ASTreeNode*> & s);

ASTreeNode* reduceQuery(std::stack<ASTreeNode*> & s);



#endif