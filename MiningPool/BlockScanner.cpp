#include "BlockScanner.h"


void BlockScanner::scan(Settings* settings) {
	Log::log("Starting block scanner on RPC %s:%d", settings->rpcServer, settings->rpcPort);

	while (true) {
		this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}