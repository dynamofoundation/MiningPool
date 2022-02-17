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
#include "WebPack.h"
#include "sha256.h"

class Global
{

public:
	Global();

	Settings* settings;
	RPC* rpc;
	Database* db;
	WebPack* webpack;

	int currentBlockHeight;
	atomic<uint32_t> extraNonce;

	mutex lockBlockData;
	json currentBlock;	
	std::string strNativeTarget;
	uint32_t iNativeTarget[8];
	unsigned char nativeTarget[32];

	unsigned char pk_script[25] = { 0 };
	int pk_script_size;


	uint32_t getExtraNonce();

	static string convertAtomToDecimal(uint64_t amount);

};

