#include "Payout.h"


void Payout::payoutJob( Global *global ) {

	Log::log("Starting payout server.");

	time_t lastPayout;
	time(&lastPayout);

	while (true) {
		time_t now;
		time(&now);

		if (now - lastPayout > global->settings->payoutPeriod) {
			lastPayout = now;

			this_thread::sleep_for(std::chrono::seconds(2));

			vector<sShareSummary> shares = global->db->countShares(now);

			uint64_t totalShares = 0;
			for (int i = 0; i < shares.size(); i++)
				totalShares += shares[i].shareCount;

			json jResult = global->rpc->execRPC("{\"jsonrpc\": \"1.0\", \"id\": \"1\", \"method\": \"getbalance\", \"params\": [\"*\", 10]}", global->settings);
			
			uint64_t balance = jResult["result"];
			balance = balance * 100000000ULL;

			if (balance > 100000000ULL) {
				global->db->updateSharesProcessed(now);
				uint64_t operatorFee = (balance * global->settings->miningFeePercent) / 100;
				sendMoney(global->settings->payoutWallet, operatorFee, global);

				balance -= operatorFee;
				balance -= 10000ULL;

				for (int i = 0; i < shares.size(); i++) {
					uint64_t payout = ((balance * shares[i].shareCount) / totalShares) - 10000ULL;
					if (payout >= global->settings->minPayout)
						sendMoney(shares[i].wallet.substr(0, 42), payout, global);
					else
						global->db->savePendingPayout(shares[i].wallet.substr(0, 42), payout);
				}

				vector<sPendingPayout> pendingPayout = global->db->getPendingPayout();
				for (int i = 0; i < pendingPayout.size(); i++) {
					if (pendingPayout[i].amount > global->settings->minPayout) {
						sendMoney(pendingPayout[i].wallet, pendingPayout[i].amount, global);
						global->db->deletePendingPayout(pendingPayout[i].wallet);
					}
				}

			}


		}
		this_thread::sleep_for(std::chrono::seconds(60));
	}

}


string Payout::convertAtomToDecimal(uint64_t amount) {

	//12345 =>  0.00012345
	//0 => 0.00000000
	//4567000000 => 45.67000000
	//100000000 => 1.00000000

	if (amount == 0)
		return "0.00000000";
	else if (amount < 100000000ULL) {
		string strAmount = to_string(amount);
		while (strAmount.length() < 8)
			strAmount = "0" + strAmount;
		strAmount = "0." + strAmount;
		return strAmount;
	}
	else {
		string strAmount = to_string(amount);
		strAmount = strAmount.substr(0, strAmount.length() - 8) + "." + strAmount.substr(strAmount.length() - 8);
		return strAmount;
	}


}

void Payout::sendMoney(string address, uint64_t amount, Global *global) {

	string strAmount = convertAtomToDecimal(amount);
	
	string strRequest = "{\"jsonrpc\": \"1.0\", \"id\": \"1\", \"method\": \"sendtoaddress\", \"params\": [\"" + address + "\", " + strAmount + ", \"\", \"\", true]}";

	global->rpc->execRPC(strRequest, global->settings);

	global->db->savePayout(address, amount);
}