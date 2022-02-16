// handle mining addresses with bad payout address
// kill inactive client threads
// log to console, file or database


#include <thread>

#include "json.hpp"

#include "SocketServer.h"
#include "Payout.h"
#include "Database.h"
#include "Global.h"
#include "BlockScanner.h"
#include "RPC.h"
#include "HTTPServer.h"

using namespace std;
using json = nlohmann::json;

int main()
{

#ifdef _WIN32
    WSADATA wsa;
    int res = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (res != NO_ERROR) 
        Log::fatalError("WSAStartup failed");
#endif

    Global* global = new Global();

    if (!global->db->databaseExists())
        global->db->createDatabase();

    json jResult = global->rpc->execRPC("{\"jsonrpc\": \"1.0\", \"id\":\"\", \"method\": \"loadwallet\", \"params\": [\"" + global->settings->rpcWallet + "\"] }", global->settings);
    if (jResult["error"].contains("code")) {
        int error = jResult["error"]["code"];
        if (error != -35) {
            char msg[256];
            sprintf(msg, "Error loading wallet %s: %d", global->settings->rpcWallet.c_str(), error);
            Log::fatalError(msg);
        }
    }

    BlockScanner* scanner = new BlockScanner();
    thread scannerThread(&BlockScanner::scan, scanner, global);
    scannerThread.detach();

    SocketServer *socketServer = new SocketServer();
    thread socketThread(&SocketServer::clientListener, socketServer, global);
    socketThread.detach();

    Payout* payout = new Payout();
    thread payoutThread(&Payout::payoutJob, payout, global);
    payoutThread.detach();


    HTTPServer* httpServer = new HTTPServer();
    thread httpThread(&HTTPServer::clientListener, httpServer, global);
    httpThread.detach();


    while (true) {
        this_thread::sleep_for(std::chrono::seconds(1));
    }

}


