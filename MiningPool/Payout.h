#pragma once

#include <thread>

#include "json.hpp"

#include "Settings.h"
#include "Database.h"


using json = nlohmann::json;

class Global;

class Payout
{
public:
	void payoutJob( Global *global );
	void sendMoney(string address, uint64_t amount, Global *global);	

	time_t lastPayout;


};

