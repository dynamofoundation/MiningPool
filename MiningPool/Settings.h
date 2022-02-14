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

	string payoutWallet;
	int miningFeePercent;
	int payoutPeriod;
	int minPayout;
	int payoutFee;
};

