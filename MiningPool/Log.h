#pragma once

#include "stdio.h"
#include <stdlib.h>
#include <cstdarg>

class Log
{
public:
	static void fatalError(const char* message);

	static void log(const char* format, ...);

};

