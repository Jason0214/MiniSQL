
#ifndef _IO_H_
#define _IO_H_

#include "string"

class IOFailure {};

std::string GetString();

int GetInt();

float GetFloat();

void Flush();

#endif
