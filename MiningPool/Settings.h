#pragma once

#include <string>

#include "json.hpp"

#include "Log.h"

using json = nlohmann::json;
using namespace std;

class Settings
{
public:

	void readSettings();

	int clientListenPort;

	string rpcURL;
	string rpcUser;
	string rpcPassword;
	string rpcWallet;

	string payoutWallet;
	string miningWallet;
	uint64_t miningFeePercent;
	int payoutPeriod;
	uint64_t minPayout;
	uint64_t payoutFee;
	int startDifficulty;

	int httpPort;
};

