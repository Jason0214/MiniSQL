#ifndef MINISQL_DELETEEXECUTOR_H
#define MINISQL_DELETEEXECUTOR_H

#include "../Parser/ASTree.h"

class DeleteExecutor{
public:
    DeleteExecutor(){}
    ~DeleteExecutor(){}
    void run(const ASTreeNode* node);
};

#endif //MINISQL_DELETEEXECUTOR_H
