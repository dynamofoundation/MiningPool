//DONE  pass different extra nonce to each client  
//DONE  tcp port based
//DONE adjust diff so block submission hits target of 3 blocks per min
//payout logic - frequency / minimum / fee
//drop connections with high reject
//handle mining addresses with bad payout address
//allow for addr.clientname format
//DONE  watch for new blocks and push them as needed
//kill inactive client threads


#include <thread>


#include "SocketServer.h"
#include "Payout.h"
#include "Database.h"
#include "Global.h"
#include "BlockScanner.h"
#include "RPC.h"

using namespace std;

int main()
{

#ifdef _WIN32
    WSADATA wsa;
    int res = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (res != NO_ERROR) 
        Log::fatalError("WSAStartup failed");
#endif

    Global* global = new Global();

    if (!Database::databaseExists())
        Database::createDatabase();

    BlockScanner* scanner = new BlockScanner();
    thread scannerThread(&BlockScanner::scan, scanner, global);
    scannerThread.detach();

    SocketServer *socketServer = new SocketServer();
    thread socketThread(&SocketServer::clientListener, socketServer, global);
    socketThread.detach();

    Payout* payout = new Payout();
    thread payoutThread(&Payout::payoutJob, payout, global->settings);
    payoutThread.detach();


    while (true) {
        this_thread::sleep_for(std::chrono::seconds(1));
    }

}


