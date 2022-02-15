#include "Global.h"

Global::Global() {
	settings = new Settings();
	settings->readSettings();

	rpc = new RPC();
	rpc->init();

	currentBlockHeight = -1;
}

uint32_t Global::getExtraNonce() {
	extraNonce++;
	return extraNonce;
}