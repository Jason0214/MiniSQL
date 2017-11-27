#include <csignal>
#include <iostream>
#include "./Interpreter/Interpreter.h"
#include "./API/APIFunctions.h"

using namespace std;

void handle_SIGINT(int a){
    ExeExit();
    exit(0);
}

int main(){
    struct sigaction handler;
    handler.sa_handler = handle_SIGINT;
    sigaction(SIGINT, &handler, NULL);
    sigaction(SIGTERM, &handler, NULL);

    Interpreter interpreter;
    while(1){
        std::string sentence;
        std::string buf;
        while(sentence.empty() || sentence[sentence.size()-1] != ';'){
            getline(std::cin, buf);
            sentence += buf;
        }
        if(sentence == "exit;"){
            ExeExit();
            break;
        }
        interpreter.run(sentence.substr(0, sentence.size()-1));
    }
    return 0;
}