
#include "IO.h"
#include <string>
#include <iostream>

using namespace std;

static void CheckConnection()
{
	if (cin.fail())
	{
		// If parent process fails, exit child process
		throw IOFailure();
	}
}

template <typename T> T GetObject()
{
	CheckConnection();
	T result;
	cin >> result;
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

void Flush()
{
	// Don't forget to flush or the parent process will get nothing!
	cout.flush();
}
