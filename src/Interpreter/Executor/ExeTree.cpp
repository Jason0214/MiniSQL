#include "ExeTree.h"

void freeExeTree(ExeTree* t){
    if(t == NULL){
        return;
    }
    else{
        freeExeTree(t->left);
        freeExeTree(t->right);
        t->left = NULL;
        t->right = NULL;
        delete t;
        return ;
    }
}
