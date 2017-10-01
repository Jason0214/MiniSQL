#include <list>

class ParseTreeNode{
public:
    
};

class ProjectNode:ParseTreeNode{
public:
    ProjectNode(bool no_project):no_project_(no_project){

    }
private:
    std::list<Attribute> attr_list_;
    bool no_project_;
}