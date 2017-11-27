#include <iostream>
#include <string>
#include <list>
#include <vector>

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Executor/Executor.h"
#include "../EXCEPTION.h"


using namespace std;

void grammarTestCase(vector<string> & cases){
    cases.push_back("select * from a   ");
    cases.push_back("select * from a where 1=c");
    cases.push_back("select * from a, b where b=2");
    cases.push_back("select * from a join b where b=c");
    cases.push_back("select * from a naturaljoin b where b=c");
    cases.push_back("select * from a, b, c where b=c");
    cases.push_back("select * from a where b=c and b = c");
    cases.push_back("select * from a where b=c or b = c");
    cases.push_back("select * from a where 1=3.2");
    cases.push_back("select * from a where \"aaa\"=c");
    cases.push_back("select * from a  1=2.2 and c = 1");
    cases.push_back("select a from b");
    cases.push_back("select a from b where c = d");
    cases.push_back("select * from (select a from b) where b=c");
    cases.push_back("select * from (select a from b where b = c) where b=c");
    cases.push_back("select asdf form asf");
    cases.push_back("drop index a");
    cases.push_back("create index a on b(c)");
    cases.push_back("create index a on b(c,d)");
    cases.push_back("drop table b");
    cases.push_back("update a set b = 3, c = '1' where a = b");
    cases.push_back("update a set b = 3, c = '1' where 1 = 2");
    cases.push_back("delete from b where c = 1");
    cases.push_back("create table c(a int primary key, b char(32))");
}

void executeTestCase(vector<string> & cases){
    cases.push_back("drop index TABLE_TEST");
    cases.push_back("drop table TABLE_TEST");

    cases.push_back("create index INDEX_TEST on TABLE_TEST(ATTR_TEST_1)");
    cases.push_back("update TABLE_TEST set ATTR_TEST_1 = 3 where 1 = 1");
    cases.push_back("update TABLE_TEST set ATTR_TEST_1 = 3 where 1 = 2");

    cases.push_back("delete from TABLE_TEST where ATTR_TEST_1 = 1");
    cases.push_back("delete from TABLE_TEST where 1 = 1");
    cases.push_back("delete from TABLE_TEST where 2 = 1");
    cases.push_back("create table TABLE_TEST(ATTR_TEST_1 int primary key, ATTR_TEST_2 char(32))");


    cases.push_back("select * from TABLE_TEST");
    cases.push_back("select * from TABLE_TEST where 1 = ATTR_TEST_1");
    cases.push_back("select * from TABLE_TEST_1, TABLE_TEST_2 where ATTR_TEST_1=2");
    cases.push_back("select * from TABLE_TEST_1 join TABLE_TEST_2 where ATTR_TEST_1=ATTR_TEST_2");
}

void printToken(TokenStream& ss){
    for(list<Token>::iterator iter = ss.token_list_.begin(); iter != ss.token_list_.end(); iter++){
        cout << iter->type << " " << iter->content <<"|" << endl;
    }
}

//int main(){
//    Lexer lexer;
//    Parser parser;
//    Executor executor;
//
//    vector<string> test_cases;
//    grammarTestCase(test_cases);
//    for(unsigned int i = 0; i < test_cases.size(); i++){
//        try{
//            lexer.loadText(test_cases[i]);
//            parser.parseSentence(lexer.result);
//            parser.getASTree().print();
//        }
//        catch (Exception & e){
//            cout << e.err << endl;
//        }
//        parser.clear();
//        lexer.clear();
//    }
//
//    test_cases.clear();
//    executeTestCase(test_cases);
//    for(unsigned int i = 0; i < test_cases.size(); i++){
//        try{
//            lexer.loadText(test_cases[i]);
//            parser.parseSentence(lexer.result);
//            executor.run(parser.getASTree().getRoot());
//            cout << endl;
//        }
//        catch (Exception & e){
//            cout << e.err << endl;
//        }
//        parser.clear();
//        lexer.clear();
//    }
//    return 0;
//}