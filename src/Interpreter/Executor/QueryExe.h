#ifndef __QUERY_EXE_H__
#define __QUERY_EXE_H__

#include <string>

class QueryExe{
public:
    std::string run(const ASTreeNode* node);
    void outputTable(std::string & table_name);

    vector table_to_join;
    map table_alias_map;
    map attr_alias_map;
};

#endif