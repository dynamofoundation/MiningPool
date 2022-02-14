#include "BlockScanner.h"


void BlockScanner::scan(Global* global, RPC* rpc) {
	Log::log("Starting block scanner on RPC %s", global->settings->rpcURL.c_str());

	while (true) {

		json jResult = rpc->execRPC("{ \"id\": 0, \"method\" : \"getblockcount\" }", global->settings);
		global->currentBlockHeight = jResult["result"];

		this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}