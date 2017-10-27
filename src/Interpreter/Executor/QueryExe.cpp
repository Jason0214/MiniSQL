#include "QueryExe.h"

void QueryExe::run(const ASTreeNode* node){
    this->mapTableAlias(t->getChild(1));
    this->descendProjection(t->getChild(0));
    this->descendSelection(t->getChild(2));
    this->joinTable();
}