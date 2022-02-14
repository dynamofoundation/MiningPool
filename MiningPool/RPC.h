#pragma once
#include <mutex>
#include <string>

#include "json.hpp"

#ifdef __linux__
#include "curl/curl.h"
#endif

#ifdef _WIN32
#include <windows.h>
#include <curl\curl.h>
#endif

#include "struct.h"
#include "Settings.h"

using json = nlohmann::json;
using namespace std;

class RPC
{
	mutex execLock;

	CURL* curl;
	CURLcode res;
	struct MemoryStruct chunk;

public:
	void init();
	json execRPC(string data, Settings *settings);
};

