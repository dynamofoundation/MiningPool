#include "BlockScanner.h"


void BlockScanner::scan(Global* global, RPC* rpc) {
	Log::log("Starting block scanner on RPC %s", global->settings->rpcURL.c_str());

	while (true) {

		json jResult = rpc->execRPC("{ \"id\": 0, \"method\" : \"getblockcount\" }", global->settings);
		int blockHeight = jResult["result"];
		if (blockHeight != global->currentBlockHeight) {
			jResult = rpc->execRPC("{ \"id\": 0, \"method\" : \"getblocktemplate\", \"params\" : [{ \"rules\": [\"segwit\"] }] }", global->settings);
			global->lockBlockData.lock();
			json block;
			block["command"] = "block_data";
			block["data"] = jResult;
			global->currentBlock = block;
			global->lockBlockData.unlock();
		}

		this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

