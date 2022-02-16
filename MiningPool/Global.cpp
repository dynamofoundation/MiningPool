#include "Global.h"

Global::Global() {
	settings = new Settings();
	settings->readSettings();

	rpc = new RPC();
	rpc->init();

	db = new Database();

	currentBlockHeight = -1;
}

uint32_t Global::getExtraNonce() {
	extraNonce++;
	return extraNonce;
}