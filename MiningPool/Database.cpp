#include "Database.h"


int Database::databaseExists() {
	if (FILE* test = fopen("pool.db", "r")) {
		fclose(test);
		return true;
	}
	else
		return false;

}

void Database::createDatabase() {

	sqlite3* db;
	int rc;
	char* errorMsg;

	rc = sqlite3_open("pool.db", &db);

	if (rc == 0) {
		const char *sql_share = "create table share (  "  \
			"share_wallet text not null, "  \
			"share_hash text not null, "  \
			"share_difficulty int not null, " \
			"share_timestamp int not null, " \
			"share_processed int not null ); "  \
			;

		if ( sqlite3_exec(db, sql_share, NULL, 0, &errorMsg) !=  SQLITE_OK) 
			Log::fatalError(errorMsg);
		
		const char* sql_payout = "create table payout (  "  \
			"payout_wallet text not null, "  \
			"payout_amount int not null, "  \
			"payout_timestamp int not null );"  \
			;

		if (sqlite3_exec(db, sql_payout, NULL, 0, &errorMsg) != SQLITE_OK)
			Log::fatalError(errorMsg);


		const char* sql_pending_payout = "create table pending_payout (  "  \
			"pending_payout_wallet text not null, "  \
			"pending_payout_amount int not null, "  \
			"pending_payout_timestamp int not null ); ";

		const char* sql_block_submit = "create table block_submit (  "  \
			"block_submit_hash text not null, "  \
			"block_submit_timestamp int not null, "  \
			"block_submit_result text not null ); ";

		if (sqlite3_exec(db, sql_block_submit, NULL, 0, &errorMsg) != SQLITE_OK)
			Log::fatalError(errorMsg);

	}
	else 
		Log::fatalError(sqlite3_errmsg(db));

	sqlite3_close(db);

}


void Database::addShare(string wallet, string hash, int difficulty) {

	dbLock.lock();

	sqlite3* db;
	int rc;

	rc = sqlite3_open("pool.db", &db);

	if (rc == 0) {
		time_t now;
		time(&now);

		const char* sql = "insert into share ( share_wallet, share_hash, share_timestamp, share_difficulty, share_processed) values (@wallet, @hash, @timestamp, @difficulty, @processed)";

		sqlite3_stmt* stmt = NULL;
		sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

		sqlite3_bind_text(stmt, 1, wallet.c_str(), -1, NULL);
		sqlite3_bind_text(stmt, 2, hash.c_str(), -1, NULL);
		sqlite3_bind_int64(stmt, 3, now);
		sqlite3_bind_int64(stmt, 4, difficulty);
		sqlite3_bind_int64(stmt, 5, 0);

		sqlite3_step(stmt);
	
		if (sqlite3_finalize(stmt) != SQLITE_OK) 
			Log::fatalError(sqlite3_errmsg(db));		

	}
	else
		Log::fatalError(sqlite3_errmsg(db));

	sqlite3_close(db);

	dbLock.unlock();

}


vector<sShareSummary> Database::countShares(time_t cutoffTime) {

	dbLock.lock();
		
	sqlite3* db;
	int rc;

	vector<sShareSummary> result;

	rc = sqlite3_open("pool.db", &db);

	if (rc == 0) {
		time_t now;
		time(&now);

		const char* sql = "select substr(share_wallet,1,42), sum(share_difficulty) from share where share_timestamp <= @endTime and share_processed = 0 group by substr(share_wallet,1,42);";

		sqlite3_stmt* stmt = NULL;
		sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

		sqlite3_bind_int64(stmt, 1, cutoffTime);

		while (sqlite3_step(stmt) == SQLITE_ROW) {
			sShareSummary share;
			share.wallet = string((char*)sqlite3_column_text(stmt, 0));
			share.shareCount = sqlite3_column_int(stmt, 1);
			result.push_back(share);
		}

		sqlite3_finalize(stmt);

	}
	else
		Log::fatalError(sqlite3_errmsg(db));

	sqlite3_close(db);

	dbLock.unlock();

	return result;

}


void Database::updateSharesProcessed(time_t cutoffTime) {
	sqlite3* db;
	int rc;

	dbLock.lock();

	rc = sqlite3_open("pool.db", &db);

	if (rc == 0) {
		time_t now;
		time(&now);

		const char* sql = "update share set share_processed = @now where share_timestamp <= @endTime and share_processed = 0;";

		sqlite3_stmt* stmt = NULL;
		sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

		sqlite3_bind_int64(stmt, 1, now);
		sqlite3_bind_int64(stmt, 2, cutoffTime);

		sqlite3_step(stmt);

		sqlite3_finalize(stmt);

	}
	else
		Log::fatalError(sqlite3_errmsg(db));

	sqlite3_close(db);
	dbLock.unlock();
	
}

void Database::savePayout(string address, uint64_t amount) {

	sqlite3* db;
	int rc;

	dbLock.lock();
	rc = sqlite3_open("pool.db", &db);

	if (rc == 0) {
		time_t now;
		time(&now);

		const char* sql = "insert into payout ( payout_wallet, payout_amount, payout_timestamp) values (@wallet, @amount, @timestamp)";

		sqlite3_stmt* stmt = NULL;
		sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

		sqlite3_bind_text(stmt, 1, address.c_str(), -1, NULL);
		sqlite3_bind_int64(stmt, 2, amount);
		sqlite3_bind_int64(stmt, 3, now);

		sqlite3_step(stmt);

		if (sqlite3_finalize(stmt) != SQLITE_OK)
			Log::fatalError(sqlite3_errmsg(db));

	}
	else
		Log::fatalError(sqlite3_errmsg(db));

	sqlite3_close(db);
	dbLock.unlock();

}


bool Database::pendingWalletExists(sqlite3* db, string address) {
	const char* sql = "select count(1) from pending_payout where pending_payout_wallet = @wallet;";

	sqlite3_stmt* stmt = NULL;
	sqlite3_bind_text(stmt, 1, address.c_str(), -1, NULL);

	
	sqlite3_step(stmt);
	int result  = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);

	return (result > 0);
}


void Database::savePendingPayout(string address, uint64_t amount) {

	sqlite3* db;
	int rc;

	dbLock.lock();
	rc = sqlite3_open("pool.db", &db);

	if (rc == 0) {
		if (pendingWalletExists(db, address)) {
			time_t now;
			time(&now);

			const char* sql = "update pending_payout set pending_payout_amount = pending_payout_amount + @amount where pending_payout_wallet = @wallet;";

			sqlite3_stmt* stmt = NULL;
			sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

			sqlite3_bind_text(stmt, 1, address.c_str(), -1, NULL);
			sqlite3_bind_int64(stmt, 2, amount);
			sqlite3_bind_int64(stmt, 3, now);
			sqlite3_bind_int64(stmt, 4, 0);

			sqlite3_step(stmt);

			if (sqlite3_finalize(stmt) != SQLITE_OK)
				Log::fatalError(sqlite3_errmsg(db));

		}
		else {
			time_t now;
			time(&now);

			const char* sql = "insert into pending_payout ( pending_payout_wallet, pending_payout_amount, pending_payout_timestamp) values (@wallet, @amount, @timestamp)";

			sqlite3_stmt* stmt = NULL;
			sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

			sqlite3_bind_text(stmt, 1, address.c_str(), -1, NULL);
			sqlite3_bind_int64(stmt, 2, amount);
			sqlite3_bind_int64(stmt, 3, now);

			sqlite3_step(stmt);

			if (sqlite3_finalize(stmt) != SQLITE_OK)
				Log::fatalError(sqlite3_errmsg(db));
		}
	}
	else
		Log::fatalError(sqlite3_errmsg(db));

	sqlite3_close(db);

	dbLock.unlock();

}

vector<sPendingPayout> Database::getPendingPayout() {

	sqlite3* db;
	int rc;

	vector<sPendingPayout> result;

	dbLock.lock();
	rc = sqlite3_open("pool.db", &db);

	if (rc == 0) {
		time_t now;
		time(&now);

		const char* sql = "select pending_payout_wallet, pending_payout_amount from pending_payout;";

		sqlite3_stmt* stmt = NULL;
		sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

		while (sqlite3_step(stmt) == SQLITE_ROW) {
			sPendingPayout payout;
			payout.wallet = string((char*)sqlite3_column_text(stmt, 0));
			payout.amount = sqlite3_column_int(stmt, 1);
			result.push_back(payout);
		}

		sqlite3_finalize(stmt);

	}
	else
		Log::fatalError(sqlite3_errmsg(db));

	sqlite3_close(db);
	dbLock.unlock();

	return result;
}


void Database::deletePendingPayout(string address) {

	sqlite3* db;
	int rc;

	dbLock.lock();
	rc = sqlite3_open("pool.db", &db);

	if (rc == 0) {
		const char* sql = "delete from pending_payout where pending_payout_wallet = @wallet;";

		sqlite3_stmt* stmt = NULL;
		sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

		sqlite3_bind_text(stmt, 1, address.c_str(), -1, NULL);

		sqlite3_step(stmt);

		if (sqlite3_finalize(stmt) != SQLITE_OK)
			Log::fatalError(sqlite3_errmsg(db));
	}
	else
		Log::fatalError(sqlite3_errmsg(db));

	sqlite3_close(db);
	dbLock.unlock();

}


void Database::addBlockSubmit(string hash, string result) {

	dbLock.lock();

	sqlite3* db;
	int rc;

	rc = sqlite3_open("pool.db", &db);

	if (rc == 0) {
		time_t now;
		time(&now);

		const char* sql = "insert into block_submit ( block_submit_hash, block_submit_timestamp, block_submit_result) values (@hash, @timestamp, @result)";

		sqlite3_stmt* stmt = NULL;
		sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

		sqlite3_bind_text(stmt, 1, hash.c_str(), -1, NULL);
		sqlite3_bind_int64(stmt, 2, now);
		sqlite3_bind_text(stmt, 3, result.c_str(), -1, NULL);

		sqlite3_step(stmt);

		if (sqlite3_finalize(stmt) != SQLITE_OK)
			Log::fatalError(sqlite3_errmsg(db));

	}
	else
		Log::fatalError(sqlite3_errmsg(db));

	sqlite3_close(db);

	dbLock.unlock();

}