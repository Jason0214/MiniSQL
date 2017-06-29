
#include "IO.h"
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

static ofstream logg("log.txt");

void Flush()
{
	// Don't forget to flush or the parent process will get nothing!
	cout.flush();
}

static void CheckConnection()
{
	if (cin.fail())
	{
		// If parent process fails, exit child process
		logg << "IO Failed" << endl;
		logg.flush();
		throw IOFailure();
	}
}

template <typename T> T GetObject()
{
	CheckConnection();
	T result;
	cin >> result;
	logg << result << endl;
	logg.flush();
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
