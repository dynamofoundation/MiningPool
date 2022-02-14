#pragma once

#include <stdio.h>

#include "sqlite3.h"

#include "Log.h"

class Database
{
public:
	static int databaseExists();
	static void createDatabase();


};

