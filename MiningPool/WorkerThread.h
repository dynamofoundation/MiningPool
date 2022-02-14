#pragma once


#include <vector>
#include <string>
#include <thread>


#ifdef _WIN32
#include <windows.h>
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

