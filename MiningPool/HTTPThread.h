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
#include "WebPack.h"

class HTTPThread
{
public:
	void clientWorker(int clientSocket, Global* global);
	void processAPICall(string URL, int clientSocket, Global* global);
};

