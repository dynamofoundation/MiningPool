#pragma once

#include <string>
#include <mutex>
#include <atomic>

#include "json.hpp"

using json = nlohmann::json;
using namespace std;


#include "Settings.h"
#include "RPC.h"
#include "Database.h"

class Global
{

public:
	Global();

	Settings* settings;
	RPC* rpc;
	Database* db;

	int currentBlockHeight;
	atomic<uint32_t> extraNonce;

	mutex lockBlockData;
	json currentBlock;	
	std::string strNativeTarget;
	uint32_t iNativeTarget[8];
	unsigned char nativeTarget[32];


	uint32_t getExtraNonce();

};

