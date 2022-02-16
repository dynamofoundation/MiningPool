#pragma once

#include <stdio.h>
#include <string>
#include <time.h>
#include <vector>
#include <mutex>

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

	mutex dbLock;

public:
	int databaseExists();
	void createDatabase();
	void addShare(string wallet, string hash, int difficulty);
	vector<sShareSummary> countShares(time_t cutoffTime);
	void updateSharesProcessed(time_t cutoffTime);
	void savePayout(string address, uint64_t amount);
	void savePendingPayout(string address, uint64_t amount);
	bool pendingWalletExists(sqlite3* db, string address);
	vector<sPendingPayout> getPendingPayout();
	void deletePendingPayout(string address);
};

