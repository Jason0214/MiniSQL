
#include "APICommands.h"
#include "IO.h"
#include <string>
#include <iostream>

using namespace std;

static string Command;

int main_api()
{
	try
	{
		while (1)
		{
			Command = GetString();

			if (Command == "quit") return 0;
			else if (Command == "begin_query") AcceptQuery();
			else if (Command == "begin_insert") AcceptInsert();
			else if (Command == "begin_update") AcceptUpdate();
			else if (Command == "begin_delete") AcceptDelete();
			else if (Command == "begin_drop_index") AcceptDropIndex();
			else if (Command == "begin_drop_table") AcceptDropTable();
			else if (Command == "begin_create_index") AcceptCreateIndex();
			else if (Command == "begin_create_table") AcceptCreateTable();
			else throw InvalidCommand(Command);

			Flush();
		}
	}
	catch (IOFailure)
	{
		cout << "IO Failed" << endl;
		Flush();
	}
	catch (InvalidCommand e)
	{
		cout << "Invalid Command: " + e.ErrorCommand << endl;
		Flush();
	}
	catch (...)
	{
		cout << "Fatal Error in API" << endl;
		Flush();
	}

	return 0;
}