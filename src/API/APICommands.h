
#ifndef _API_COMMANDS_H_
#define _API_COMMANDS_H_

#include <string>

class InvalidCommand 
{
public:
	std::string ErrorCommand;
	InvalidCommand(std::string errorCommand) { ErrorCommand = errorCommand; }
};

void OnQuit();

void AcceptQuery();

void AcceptInsert();

void AcceptUpdate();

void AcceptDelete();

void AcceptCreateTable();

void AcceptDropTable();

void AcceptCreateIndex();

void AcceptDropIndex();

#endif