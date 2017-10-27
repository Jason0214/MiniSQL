#include "Executor.h"

void Executor::ExecuteAstree(const ASTree & t) const{
    if(t.getType() == "select"){
        std::string result_table = this->QueryExe.run(t.root_);
        this->QueryExe.outputTable(result_table);
        this->QueryExe.clear();
    }
    else if(t.getType() == "insert"){
        this->InsertExe->run(t->root_);
    }
    else if(t.getType() == "update"){
        this->UpdateExe->run(t->root_);
    }
    else if(t.getType() == "delete"){
        this->DeleteExe->run(t->root_);
    }
    else{
        // drop
        this->executeDrop(t);
    }
}