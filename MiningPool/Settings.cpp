#include "Settings.h"


void Settings::readSettings() {

	if (FILE* settings = fopen("settings.txt", "r")) {
		int rc = fseek(settings, 0, SEEK_END);
		size_t fileSize = ftell(settings);
		fseek(settings, 0, SEEK_SET);
		string strSettings(fileSize, 0);
		fread(&strSettings[0], 1, fileSize, settings);
		json jSettings = json::parse(strSettings.c_str());
		fclose(settings);

		clientListenPort = jSettings["clientListenPort"];

		rpcURL = jSettings["rpcURL"];
		rpcUser = jSettings["rpcUser"];
		rpcPassword = jSettings["rpcPassword"];

		payoutWallet = jSettings["payoutWallet"];
		miningWallet = jSettings["miningWallet"];
		miningFeePercent = jSettings["miningFeePercent"];
		payoutPeriod = jSettings["payoutPeriod"];
		minPayout = jSettings["minPayout"];
		payoutFee = jSettings["payoutFee"];

	}
	else
		Log::fatalError("Cannot open settings.txt");



}