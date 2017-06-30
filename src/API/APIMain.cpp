
#include "APICommands.h"
#include "../EXCEPTION.h"
#include "IO.h"
#include <string>
#include <iostream>

using namespace std;

static string Command;

int main()
{
	while (1)
	{
		try
		{
			Command = GetString();

			if (Command == "quit") goto _QUIT;
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
		catch (IOFailure)
		{
			goto _QUIT;
		}
		catch (InvalidCommand e)
		{
			cout << "Invalid Command: " + e.ErrorCommand << endl;
			Flush();
			goto _QUIT;
		}
		catch (Exception e)
		{
			cout << "Error Msg : " << e.Message << endl;
			cout << "end_result" << endl;
			Flush();
		}
		catch (...)
		{
			cout << "Fatal Error in API" << endl;
			Flush();
			goto _QUIT;
		}
	}

_QUIT:
	OnQuit();
	cout << "API Quit" << endl;
	Flush();
	return 0;
}