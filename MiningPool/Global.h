#pragma once

#include <string>
#include <mutex>

#include "json.hpp"

using json = nlohmann::json;
using namespace std;


#include "Settings.h"
#include "RPC.h"

class Global
{

public:
	Global();

	Settings* settings;
	RPC* rpc;
	int currentBlockHeight;
	atomic<uint32_t> extraNonce;

	mutex lockBlockData;
	json currentBlock;	
	std::string strNativeTarget;
	uint32_t iNativeTarget[8];
	unsigned char nativeTarget[32];


	uint32_t getExtraNonce();

};

