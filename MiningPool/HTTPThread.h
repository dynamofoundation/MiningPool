#pragma once

#include <vector>
#include <string>
#include <thread>
#include <sstream>
#include <algorithm>
#include <iterator>

#ifdef _WIN32
#include <windows.h>
#endif

#include "json.hpp"

#include "Global.h"

class HTTPThread
{
public:
	void clientWorker(int clientSocket, Global* global);
};

