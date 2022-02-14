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


}

void Database::readSettings() {

	if (FILE* settings = fopen("settings.txt", "r")) {

		fclose(settings);
	}
	else
		Log::fatalError("Cannot open settings.txt");

	/*
	sqlite3* db;
	int rc = sqlite3_open("pool.db", &db);
	if (rc == 0) {

	}
	else

		sqlite3_close(db);
		*/
}