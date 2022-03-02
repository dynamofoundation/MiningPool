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

#ifdef __linux__
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h> 
#include <unistd.h>
#endif

#include "json.hpp"

#include "Global.h"
#include "WebPack.h"
#include "Payout.h"

class HTTPThread
{
public:
	void clientWorker(int clientSocket, Global* global);
	void processAPICall(string URL, int clientSocket, Global* global);
};

