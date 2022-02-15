#include "Payout.h"


void Payout::payoutJob( Global *global ) {

	Log::log("Starting payout server.");

	time_t lastPayout;
	time(&lastPayout);

	while (true) {
		time_t now;
		time(&now);
		if(true) {
		//if (now - lastPayout > global->settings->payoutPeriod) {
			lastPayout = now;

			//sum up shares
			//get mining wallet balance
			//calc and pay fee to pool op
			//calc amts payable to each miner (+fee)
			//send payments

			this_thread::sleep_for(std::chrono::seconds(2));

			vector<sShareSummary> shares = Database::countShares(now);
			Database::updateSharesProcessed(now);

			uint64_t totalShares = 0;
			for (int i = 0; i < shares.size(); i++)
				totalShares += shares[i].shareCount;

			json jResult = global->rpc->execRPC("{\"jsonrpc\": \"1.0\", \"id\": \"1\", \"method\": \"getbalance\", \"params\": [\"*\", 10]}", global->settings);
			
			uint64_t balance = jResult["result"] * 100000000ULL;

			if (balance > 100000000ULL) {
				uint64_t operatorFee = (balance * global->settings->miningFeePercent) / 100;
				sendMoney(global->settings->payoutWallet, operatorFee, global);

				balance -= operatorFee;
				balance -= 10000ULL;

				for (int i = 0; i < shares.size(); i++) {
					uint64_t payout = ((balance * shares[i].shareCount) / totalShares) - 10000ULL;
					if (payout >= global->settings->minPayout)
						sendMoney(shares[i].wallet.substr(0, 42), payout, global);
					else
						Database::savePendingPayout(shares[i].wallet.substr(0, 42), payout);
				}

				vector<sPendingPayout> pendingPayout = Database::getPendingPayout();
				for (int i = 0; i < pendingPayout.size(); i++) {
					if (pendingPayout[i].amount > global->settings->minPayout) {
						sendMoney(pendingPayout[i].wallet, pendingPayout[i].amount, global);
						Database::deletePendingPayout(pendingPayout[i].wallet);
					}
				}

			}


		}
		this_thread::sleep_for(std::chrono::seconds(60));
	}

}


void Payout::sendMoney(string address, uint64_t amount, Global *global) {

	double dAmount = amount / 1000000000.0;
	
	string strRequest = "{\"jsonrpc\": \"1.0\", \"id\": \"1\", \"method\": \"sendtoaddress\", \"params\": [\"" + address + "\", " + to_string(dAmount) + ", \"\", \"\", true]}";

	global->rpc->execRPC(strRequest, global->settings);

	Database::savePayout(address, amount);
}