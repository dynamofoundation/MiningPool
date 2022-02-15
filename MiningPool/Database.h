#pragma once

#include <stdio.h>
#include <string>
#include <time.h>
#include <vector>

#include "sqlite3.h"

#include "Log.h"


using namespace std;

struct sShareSummary {
	string wallet;
	uint64_t shareCount;
};

struct sPendingPayout {
	string wallet;
	uint64_t amount;
};


class Database
{
public:
	static int databaseExists();
	static void createDatabase();
	static void addShare(string wallet, string hash);
	static vector<sShareSummary> countShares(time_t cutoffTime);
	static void updateSharesProcessed(time_t cutoffTime);
	static void savePayout(string address, uint64_t amount);
	static void savePendingPayout(string address, uint64_t amount);
	static bool pendingWalletExists(sqlite3* db, string address);
	static vector<sPendingPayout> getPendingPayout();
	static void deletePendingPayout(string address);
};

