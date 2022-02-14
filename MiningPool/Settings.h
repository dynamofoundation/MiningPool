#pragma once

#include <string>

using namespace std;

class Settings
{
public:
	int clientListenPort;

	string rpcServer;
	int rpcPort;
	string rpcUser;
	string rpcPassword;

	string payoutWallet;
	int miningFeePercent;
	int payoutPeriod;
	int minPayout;
	int payoutFee;
};

