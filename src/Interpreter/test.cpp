#include <iostream>
#include <string>
#include <list>

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"


using namespace std;

int main(){
    string sql_sentence;
    string buf;
    while(true){
        getline(cin, buf);
        if(buf[buf.size()-1] == ';'){
            sql_sentence += buf.substr(0, buf.size()-1);
            break;
        }
        else{
            sql_sentence += buf;
        }
    }
    Lexer lexer;
    lexer.loadText(sql_sentence);
     for(list<Token>::iterator i = lexer.result.token_list_.begin(); i != lexer.result.token_list_.end(); i++){
         cout << i->type << " " << i->content <<"|" << endl;
     }

    Parser parse;
    parse.parseSentence(lexer.result);
    parse.getASTree().print();

    return 0;
}