#pragma once

#include <string>
#include <mutex>

#include "json.hpp"

using json = nlohmann::json;
using namespace std;


#include "Settings.h"

class Global
{

public:
	Global();

	Settings* settings;
	int currentBlockHeight;
	atomic<uint32_t> extraNonce;

	mutex lockBlockData;
	json currentBlock;	

	uint32_t getExtraNonce();

};

