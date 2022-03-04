#include "HTTPThread.h"

#ifdef __linux__
#define closesocket close
#endif


inline bool ends_with(std::string const& value, std::string const& ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void HTTPThread::clientWorker(int clientSocket, Global* global) {

    const unsigned int MAX_BUF_LENGTH = 4096;
    std::vector<char> buffer(MAX_BUF_LENGTH);
    std::string httpRequest;
    int bytesReceived = 0;
    do {
        bytesReceived = recv(clientSocket, &buffer[0], buffer.size(), 0);
        if (bytesReceived == -1) {
            return;
        }
        else {
            httpRequest.append(buffer.cbegin(), buffer.cend());
        }
    } while (bytesReceived == MAX_BUF_LENGTH);


    try {
        string URI;
        istringstream sReq(httpRequest);
        getline(sReq, URI);

        istringstream sTokens(URI);
        vector<string> vTokens((istream_iterator<string>(sTokens)), istream_iterator<string>());

        if (vTokens.size() < 2) {
            closesocket(clientSocket);
            return;
        }

        string URL = vTokens[1];

        if (URL == "/")
            URL = "/index.html";

        if (URL.substr(0, 4) == "/api")
            processAPICall(URL, clientSocket, global);
        else {
            //its a page request, get from webpack

            char* content;
            int contentLen = 0;
            if (global->webpack->pages.count(URL) == 0) {
                closesocket(clientSocket);
                return;
            }
            else {
                content = global->webpack->pages[URL].data;
                contentLen = global->webpack->pages[URL].len;
            }

            string mimeType = "text/html";
            if (ends_with(URL, ".css"))
                mimeType = "text/css";
            else if (ends_with(URL, ".png"))
                mimeType = "image/png";
            else if (ends_with(URL, ".js"))
                mimeType = "text/javascript";


            string header = "HTTP/1.1 200 OK\r\n";
            header += "Content-Type: " + mimeType + "\r\n";
            header += "Content-Length: " + to_string(contentLen) + "\r\n";
            header += "\r\n";

            send(clientSocket, header.c_str(), header.size(), 0);
            send(clientSocket, content, contentLen, 0);

            closesocket(clientSocket);
        }
    }
    catch (exception e) {
        closesocket(clientSocket);
    }

}



vector<string> split(string str, string token) {
    vector<string>result;
    while (str.size()) {
        int index = str.find(token);
        if (index != string::npos) {
            result.push_back(str.substr(0, index));
            str = str.substr(index + token.size());
            if (str.size() == 0)result.push_back(str);
        }
        else {
            result.push_back(str);
            str = "";
        }
    }
    return result;
}

void HTTPThread::processAPICall(string URL, int clientSocket, Global* global) {

    json jResult;

    string api = URL.substr(5);
    string endpoint;
    string args;

    if (api.find("?") != string::npos) {
        endpoint = api.substr(0, api.find("?"));
        args = api.substr(api.find("?") + 1);
    }
    else
        endpoint = api;

    map<string,string> mArgs;
    if (args.length() > 0) {
        vector<string> params = split(args, "&");
        for (int i = 0; i < params.size(); i++) {
            vector<string> pair = split(params[i], "=");
            mArgs.emplace(pair[0], pair[1]);
        }
    }


    if (endpoint == "getuserdata") {
        string wallet = mArgs["wallet"];

        //sanitize for sql injection because we cant easily parameterize all queries
        for (int i = 0; i < wallet.length(); i++)
            if (string("0123456789abcdefghijklmnopqrstuvwxyz").find(wallet[i]) == string::npos)
                return;

        uint64_t unpaidBalance = global->db->getUnpaidBalanceForWallet(wallet);
        jResult["unpaid_balance"] = Global::convertAtomToDecimal(unpaidBalance);

        time_t now;
        time(&now);
        int nextPayout = global->settings->payoutPeriod - (now - global->payout->lastPayout);
        int nextPayoutMin = nextPayout / 60;
        int nextPayoutSec = nextPayout - (nextPayoutMin * 60);
        string strNextPayout = to_string(nextPayoutMin) + "min " + to_string(nextPayoutSec) + " sec";
        jResult["next_payout"] = strNextPayout;


        vector<vector<string>> myShares = global->db->execSQL("select sum(share_difficulty) from share where share_processed = 0 and share_wallet = '" + wallet + "'");
        int iMyShares = atoi(myShares[0][0].c_str());
        vector<vector<string>> totalShares = global->db->execSQL("select sum(share_difficulty) from share where share_processed = 0");
        int iTotalShares = atoi(totalShares[0][0].c_str());
        int pctShares = (iMyShares * 100) / iTotalShares;
        jResult["share_summary"] = "Your shares: " + to_string(iMyShares) + ", percent of pool: " + to_string(pctShares);

        vector<vector<string>> foundBlocks = global->db->execSQL("select block_submit_hash, block_submit_timestamp from block_submit order by block_submit_timestamp desc limit 30");
        string strFoundBlocks = "<table>";
        for (int row = 0; row < foundBlocks.size(); row++) {
            strFoundBlocks += "<tr>";
            strFoundBlocks += "<td style='color:#ffffff'>" + foundBlocks[row][0] + "</td>";
            time_t t = atoll(foundBlocks[row][1].c_str());
            struct tm* timeStamp = localtime(&t);
            char cTime[256];
            strftime(cTime, 256, "%c", timeStamp);
            strFoundBlocks += "</tr>";
        }
        strFoundBlocks += "</table>";
        jResult["found_blocks"] = strFoundBlocks;


        vector<vector<string>> dbCurrentShares = global->db->execSQL("select share_hash, share_difficulty, share_timestamp from share where share_processed = 0 and share_wallet = '" + wallet + "' order by share_timestamp desc");
        string strCurrentShares = "<table>";
        for (int row = 0; row < dbCurrentShares.size(); row++) {
            strCurrentShares += "<tr>";
            strCurrentShares += "<td style='color:#ffffff'>" + dbCurrentShares[row][0] + "</td>";
            strCurrentShares += "<td style='color:#ffffff'>" + dbCurrentShares[row][1] + "</td>";
            time_t t = atoll(dbCurrentShares[row][2].c_str());
            struct tm* timeStamp = localtime(&t);
            char cTime[256];
            strftime(cTime, 256, "%c", timeStamp);
            strCurrentShares += "</tr>";
        }
        strCurrentShares += "</table>";
        jResult["current_shares"] = strCurrentShares;


        vector<vector<string>> dbPayouts = global->db->execSQL("select payout_txid, payout_amount, payout_timestamp from payout where payout_wallet = '" + wallet + "' order by payout_timestamp desc limit 30 ");
        string strPayouts = "<table>";
        for (int row = 0; row < dbPayouts.size(); row++) {
            strPayouts += "<tr>";
            strPayouts += "<td style='color:#ffffff'>" + dbPayouts[row][0] + "</td>";
            strPayouts += "<td style='color:#ffffff'>" + dbPayouts[row][1] + "</td>";
            time_t t = atoll(dbPayouts[row][2].c_str());
            struct tm* timeStamp = localtime(&t);
            char cTime[256];
            strftime(cTime, 256, "%c", timeStamp);
            strPayouts += "</tr>";
        }
        strPayouts += "</table>";
        jResult["payouts"] = strPayouts;


    }


    string strResult = jResult.dump();

    string header = "HTTP/1.1 200 OK\r\n";
    header += "Content-Type: application/json\r\n";
    header += "Content-Length: " + to_string(strResult.length()) + "\r\n";
    header += "\r\n";

    send(clientSocket, header.c_str(), header.size(), 0);
    send(clientSocket, strResult.c_str(), strResult.size(), 0);

    closesocket(clientSocket);


}