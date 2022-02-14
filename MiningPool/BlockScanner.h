#pragma once

#include <thread>

#include "Settings.h"
#include "RPC.h"
#include "Global.h"

using namespace std;

class BlockScanner
{
public:
	mutex lockJob;

	void scan(Global* global, RPC* rpc);
	

};

