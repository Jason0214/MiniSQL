#ifndef MINISQL_EXECUTOR_H
#define MINISQL_EXECUTOR_H

#include "../Parser/ASTree.h"
#include "../../API/APIStructures.h"
#include "QueryExecutor.h"

class Executor{
public:
    Executor(){}
    ~Executor(){}

    void run(const ASTree & T);

private:
    void runInsertExecutor(const ASTreeNode* root);
    void runUpdateExecutor(const ASTreeNode* root);
    void runCreateTableExecutor(const ASTreeNode* root);
    void runCreateIndexExecutor(const ASTreeNode* root);
    void runDropTableExecutor(const ASTreeNode* root);
    void runDropIndexExecutor(const ASTreeNode* root);
    void runDeleteExecutor(const ASTreeNode* node);

    QueryExecutor query_executor_;
};

#endif //MINISQL_EXECUTOR_H
