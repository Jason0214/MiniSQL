#ifndef MINISQL_EXECUTOR_H
#define MINISQL_EXECUTOR_H

#include "../Parser/ASTree.h"
#include "../../API/APIStructures.h"
#include "QueryExecutor.h"
#include "DeleteExecutor.h"

class Executor{
public:
    Executor(){}
    ~Executor(){}

    void run(const ASTreeNode* root);

    static Comparison syntax2CmpSingleAttr(ParserSymbol::Action equality, ASTreeNode* attr, ASTreeNode* constant);
    static bool checkEquality(ParserSymbol::Action equality, ASTreeNode* left_node, ASTreeNode* right_node);
private:
    void runInsertExecutor(const ASTreeNode* root);
    void runUpdateExecutor(const ASTreeNode* root);
    void runCreateTableExecutor(const ASTreeNode* root);
    void runCreateIndexExecutor(const ASTreeNode* root);
    void runDropTableExecutor(const ASTreeNode* root);
    void runDropIndexExecutor(const ASTreeNode* root);

    QueryExecutor query_executor_;
    DeleteExecutor delete_executor_;
};

#endif //MINISQL_EXECUTOR_H
