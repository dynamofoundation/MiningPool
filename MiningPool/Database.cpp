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
		const char *sql_share = "create table share ( share_id int primary key not null, "  \
			"share_wallet text not null, "  \
			"share_hash text not null, "  \
			"share_timestamp int not null );"  \
			;

		if ( sqlite3_exec(db, sql_share, NULL, 0, &errorMsg) !=  SQLITE_OK) 
			Log::fatalError(errorMsg);
		
		const char* sql_payout = "create table payout ( payout_id int primary key not null, "  \
			"payout_wallet text not null, "  \
			"payout_amount int not null, "  \
			"payout_timestamp int not null );"  \
			;

		if (sqlite3_exec(db, sql_payout, NULL, 0, &errorMsg) != SQLITE_OK)
			Log::fatalError(errorMsg);


		const char* sql_pending_payout = "create table pending_payout ( pending_payout_id int primary key not null, "  \
			"pending_payout_wallet text not null, "  \
			"pending_payout_amount int not null, "  \
			"pending_payout_timestamp int not null );"  \
			;

		if (sqlite3_exec(db, sql_pending_payout, NULL, 0, &errorMsg) != SQLITE_OK)
			Log::fatalError(errorMsg);

	}
	else 
		Log::fatalError(sqlite3_errmsg(db));

	sqlite3_close(db);

}



/*
	sqlite3* db;
	int rc;

	rc = sqlite3_open("test.db", &db);

	if (rc == 0) {

	}
	else
		Log::fatalError(sqlite3_errmsg(db));

	sqlite3_close(db);


*/