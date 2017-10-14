#include <iostream>
#include <string>
#include <list>
#include <vector>

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"


using namespace std;

void generateCase(vector<string> & cases){
    cases.push_back("select * from a   ");
    cases.push_back("select * from a where b=c");
    cases.push_back("select * from a, b where b=c");
    cases.push_back("select * from a join b where b=c");
    cases.push_back("select * from a naturaljoin b where b=c");
    cases.push_back("select * from a, b, c where b=c");
    cases.push_back("select * from a where b=c and b = c");
    cases.push_back("select * from a where b=c or b = c");
    cases.push_back("select * from a where 1=3.2");
    cases.push_back("select * from a where \"aaa\"=c");
    cases.push_back("select * from a where 1=2.2 and c = 1");
    cases.push_back("select a from b");
    cases.push_back("select a from b where c = d");
    cases.push_back("select * from (select a from b) where b=c");
    cases.push_back("select * from (select a from b where b = c) where b=c");
}

void printToken(TokenStream& ss){
    for(list<Token>::iterator iter = ss.token_list_.begin(); iter != ss.token_list_.end(); iter++){
        cout << iter->type << " " << iter->content <<"|" << endl;
    }
}

int main(){
    Lexer lexer;
    Parser parser;

    vector<string> test_cases;
    generateCase(test_cases);
    for(unsigned int i = 0; i < test_cases.size(); i++){
        lexer.loadText(test_cases[i]);
        parser.parseSentence(lexer.result);
        parser.getASTree().print();
        parser.clear();
        lexer.clear();
    }

    return 0;
}