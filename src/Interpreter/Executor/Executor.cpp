#include "Executor.h"

#include "../../API/APIStructures.h"
#include "../../API/APIFunctions.h"
#include "ExecuteFunc.h"
#include "../../EXCEPTION.h"

#include <iostream>

using namespace std;
using namespace ParserSymbol;

void Executor::run(const ASTree & T) {
    const ASTreeNode* root = T.getRoot();
    switch(root->getTag()){
        case query_:
            this->query_executor_.run(root);
            break;
        case delete_:
            runDeleteExecutor(root);
            break;
        case insert_:
            runInsertExecutor(root);
            break;
        case create_index:
            runCreateIndexExecutor(root);
            break;
        case create_table_:
            runCreateTableExecutor(root);
            break;
        case drop_table:
            runDropTableExecutor(root);
            break;
        case drop_index:
            runDropIndexExecutor(root);
            break;
        case update_:
            runUpdateExecutor(root);
            break;
    }
}

void Executor::runCreateTableExecutor(const ASTreeNode* root){
    AttrDefinitionVector defVec;
    const string & table_name = root->getChild(1)->getContent();
    ASTreeNode* meta_set_node = root->getChild(0);
    for(int i = 0; i < meta_set_node->childrenCount(); i++){
        AttrDefinition def;
        def.beUnique = false;
        def.bePrimaryKey = false;
        def.beNotNull = false;
        ASTreeNode* meta_node = meta_set_node->getChild(i);
        int j = 0;
        while(meta_node->getChild(j)->getTag() == constrain){
            const string & constrain_type = meta_node->getChild(j)->getContent();
            if(constrain_type == "primary-key"){
                def.bePrimaryKey = true;
            }
            else if(constrain_type == "unique"){
                def.beUnique = true;
            }
            else if(constrain_type == "not-null"){
                def.beNotNull = true;
            }
            j++;
        }
        ASTreeNode* type_node = meta_node->getChild(j);
        switch(type_node->getAction()){
            case type_char:
                def.TypeName = "char";
                def.TypeParam = atoi(type_node->getChild(0)->getContent().c_str());
                break;
            case type_varchar:
                def.TypeName = "varchar";
                def.TypeParam = atoi(type_node->getChild(0)->getContent().c_str());
                break;
            case type_int:
                def.TypeName = "int";
                break;
            case type_float:
                def.TypeName = "float";
                break;
        }
        j++;
        def.AttrName = meta_node->getChild(j)->getContent();
        defVec.emplace_back(def);
    }
    ExeCreateTable(table_name, defVec);
}

void Executor::runInsertExecutor(const ASTreeNode* root){
    InsertValueVector valueVec;
    const string & table_name = root->getChild(1)->getContent();
    ASTreeNode* tmp = root->getChild(0);
    for(int i = tmp->childrenCount()-1; i >= 0; i--){
        valueVec.emplace_back(tmp->getChild(i)->getContent());
    }
     ExeInsert(table_name, valueVec);
//    cout << "Insert into " << table_name <<endl;
//    for(int i = 0; i < valueVec.size(); i++){
//        cout << valueVec[i] << endl;
//    }
}

void Executor::runUpdateExecutor(const ASTreeNode* root){
    const string & table_name = root->getChild(2)->getContent();
    // exe conditions
    ComparisonVector cmpVec;
    try{
        ExecuteFunc::parseConditionSet(cmpVec, root->getChild(0));
    }
    catch(FalseCondition &){
        return ;
    }
    ASTreeNode* assign_set_node = root->getChild(1);
    if(assign_set_node->childrenCount() > 1){
        throw TODO("multiple value update currently not support");
    }
    const string & attr_name = assign_set_node->getChild(0)->getChild(1)->getContent();
    const string & value = assign_set_node->getChild(0)->getChild(0)->getContent();
    ExeUpdate(table_name, attr_name, value, cmpVec);
}

void Executor::runCreateIndexExecutor(const ASTreeNode* root){
    const string & table_name = root->getChild(1)->getContent();
    const string & index_name = root->getChild(2)->getContent();
    ASTreeNode* column_set_node = root->getChild(0);
    if(column_set_node -> childrenCount() > 1){
        throw TODO("create index on multiple attributes currently not supported");
    }
    const string & attr_name = column_set_node->getChild(0)->getContent();
    ExeCreateIndex(table_name, attr_name, index_name);
//    cout << "create index: " << index_name << endl;
//    cout << "attribute name: " << attr_name << endl;
//    cout << "table name: " << table_name << endl;
}

void Executor::runDropTableExecutor(const ASTreeNode* root){
    const string & table_name = root->getChild(0)->getContent();
    ExeDropTable(table_name);
//    cout << "drop table: " << table_name << endl;
}
void Executor::runDropIndexExecutor(const ASTreeNode* root){
    const string & index_name = root->getChild(0)->getContent();
    ExeDropIndex(index_name);
//    cout << "drop index: " << index_name << endl;
}

void Executor::runDeleteExecutor(const ASTreeNode* root){
    const string & table_name = root->getChild(1)->getContent();
    ComparisonVector cmpVec;
    try{
        ExecuteFunc::parseConditionSet(cmpVec, root->getChild(0));
    }
    catch(FalseCondition &){
        return ;
    }
//    cout << "delete table " << table_name << endl;
//    for(auto i = cmpVec.begin(); i != cmpVec.end(); i++){
//        cout << "left: " <<  i->Comparand1.TypeName <<" " << i->Comparand1.Content << " ";
//        cout << "op: " << i->Operation << " ";
//        cout << "right: " <<  i->Comparand2.TypeName <<" " << i->Comparand2.Content << " ";
//        cout << endl;
//    }
    ExeDelete(table_name, cmpVec);
}