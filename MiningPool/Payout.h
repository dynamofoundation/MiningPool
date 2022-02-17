#pragma once

#include <thread>

#include "json.hpp"

#include "Settings.h"
#include "Database.h"
#include "Global.h"

using json = nlohmann::json;

class Payout
{
public:
	void payoutJob( Global *global );
	void sendMoney(string address, uint64_t amount, Global *global);	
};

