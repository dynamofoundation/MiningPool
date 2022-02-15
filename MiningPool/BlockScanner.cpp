#include "BlockScanner.h"





void BlockScanner::scan(Global* global) {
	Log::log("Starting block scanner on RPC %s", global->settings->rpcURL.c_str());

	while (true) {

		json jResult = global->rpc->execRPC("{ \"id\": 0, \"method\" : \"getblockcount\" }", global->settings);
		int blockHeight = jResult["result"];
		if (blockHeight != global->currentBlockHeight) {
			Log::log("Switching to block %d", blockHeight);
			global->lockBlockData.lock();

			jResult = global->rpc->execRPC("{ \"id\": 0, \"method\" : \"gethashfunction\", \"params\" : [] }", global->settings);
			string strProgram = jResult["result"][0]["program"];

			jResult = global->rpc->execRPC("{ \"id\": 0, \"method\" : \"getblocktemplate\", \"params\" : [{ \"rules\": [\"segwit\"] }] }", global->settings);
			jResult["program"] = strProgram;

			json block;
			block["command"] = "block_data";
			block["data"] = jResult;

			global->currentBlock = block;

			global->strNativeTarget = jResult["result"]["target"];
			hex2bin((unsigned char*)&(global->iNativeTarget), global->strNativeTarget.c_str(), 32);
			memcpy(&(global->nativeTarget), &(global->iNativeTarget), 32);


			global->lockBlockData.unlock();
			global->currentBlockHeight = blockHeight;
		}

		this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

