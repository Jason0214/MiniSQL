
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
		catch (const IOFailure &e)
		{
			goto _QUIT;
		}
		catch (const InvalidCommand &e)
		{
			cout << "Invalid Command: " + e.ErrorCommand << endl;
			Flush();
			goto _QUIT;
		}
		catch (const DatabaseNotFound &e) {
			cout << "Database Not Found: " << e.Message << endl;
			Flush();
		}
		catch (const DatabaseNotSelected &e) {
			cout << "Database Not Seleted" << endl;
			Flush();
		}
		catch (const DuplicatedTableName &e)
		{
			cout << "Duplicated Table Name: " << e.Message << endl;
			Flush();
		}
		catch (const DuplicatedIndexName &e) {
			cout << "Duplicated Index Name: " << e.Message << endl;
			Flush();
		}
		catch (const TableNotFound &e) {
			cout << "Table Not Found: " << e.Message << endl;
			Flush();
		}
		catch (const IndexNotFound &e) {
			cout << "Index Not Found: " << e.Message << endl;
			Flush();
		}
		catch (const AttributeNotFound &e) {
			cout << "Attribute Not Found: " << e.Message << endl;
			Flush();
		}
		catch (const TableAliasNotFound &e) {
			cout << "Table Alias Not Found: " << e.Message << endl;
			Flush();
		}
		catch (const Exception &e)
		{
			cout << "Customized Exception: " << e.Message << endl;
			cout << "end_result" << endl;
			Flush();
		}
		catch (const exception &e)
		{
			cout << "C++ Standard Exception: " << e.what() << endl;
			cout << "end_result" << endl;
			Flush();
		}
		catch (...)
		{
			cout << "Fatal Error in API:" << endl;
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