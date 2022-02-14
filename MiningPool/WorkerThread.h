#pragma once


#include <vector>
#include <string>
#include <thread>


#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "json.hpp"

using namespace std;

using json = nlohmann::json;

class WorkerThread
{

	int socketError;
	int authDone;
	int lastBlockHeightSent;
	int difficulty;

public:
	void clientWorker(int clientSocket);
	void blockUpdateThread(int clientSocket);

};

