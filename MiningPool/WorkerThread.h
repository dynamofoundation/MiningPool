#pragma once


#include <vector>
#include <string>
#include <thread>


#ifdef _WIN32
#include <windows.h>
#endif

#include "json.hpp"

#include "Global.h"

using namespace std;

using json = nlohmann::json;

class WorkerThread
{

	int socketError;
	int authDone;
	int lastBlockHeightSent;
	int difficulty;
	int extraNonce;

	void sendDifficulty();
	void sendCurrentBlock(int clientSocket, Global* global);

public:
	void clientWorker(int clientSocket, Global* global);
	void blockUpdateThread(int clientSocket, Global* global);

};

