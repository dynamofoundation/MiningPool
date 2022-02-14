#pragma once


#include <vector>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "json.hpp"

using namespace std;

using json = nlohmann::json;

class WorkerThread
{
public:
	void clientWorker(int clientSocket);
};

