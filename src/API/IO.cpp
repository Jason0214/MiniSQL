
#include "IO.h"
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

//#define _LOG_

#ifdef _LOG_
static ofstream logg("log.txt");
#endif

void Flush()
{
	// Don't forget to flush or the parent process will get nothing!
	cout.flush();
}

static void CheckConnection()
{
	if (cin.fail())
	{
#ifdef _LOG_
		// If parent process fails, exit child process
		logg << "IO Failed" << endl;
		logg.flush();
#endif
		throw IOFailure();
	}
}

template <typename T> T GetObject()
{
	CheckConnection();
	T result;
	cin >> result;
#ifdef _LOG_
	logg << result << endl;
	logg.flush();
#endif
	return result;
}

string GetString()
{
	return GetObject<string>();
}

int GetInt()
{
	return GetObject<int>();
}

float GetFloat()
{
	return GetObject<float>();
}
