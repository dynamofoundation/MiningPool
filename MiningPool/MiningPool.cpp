//TODO:
// handle mining addresses with bad payout address
// kill inactive client threads
// log to console, file or database
// occasional high-hash reject on cards with high hashrate
// add index to tables for performance


#include <thread>

#include "json.hpp"

#include "SocketServer.h"
#include "Payout.h"
#include "Database.h"
#include "Global.h"
#include "BlockScanner.h"
#include "RPC.h"
#include "HTTPServer.h"
#include "WebPack.h"

using namespace std;
using json = nlohmann::json;

int main(int argc, char* argv[])
{

#ifdef _WIN32
    WSADATA wsa;
    int res = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (res != NO_ERROR) 
        Log::fatalError("WSAStartup failed");
#endif

    bool makeWebpack = false;
    if (argc > 1)
        if (string(argv[1]) == "make_webpack")
            makeWebpack = true;

    Global* global = new Global(makeWebpack);

    if (makeWebpack) {
        printf("webpack.dat created\n");
        exit(0);
    }

    if (!global->db->databaseExists())
        global->db->createDatabase();

    global->db->upgradeDatabase();

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

    global->payout = payout;


    HTTPServer* httpServer = new HTTPServer();
    thread httpThread(&HTTPServer::clientListener, httpServer, global);
    httpThread.detach();


    while (true) {
        this_thread::sleep_for(std::chrono::seconds(1));
    }

}


