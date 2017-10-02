#include "Lexer.h"
#include <iostream>
#include <string>
#include <list>

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
    for(list<Token>::iterator i = lexer.result.begin(); i != lexer.result.end(); i++){
        cout << i->type << " " << i->content << endl; 
    }
    return 0;
}