//pass different extra nonce to each client
//tcp port based
//adjust diff so block submission hits target
//payout logic - frequency / minimum / fee
//drop connections with high reject
//handle mining addresses with bad payout address
//allow for addr.clientname format

#include <thread>


#include "SocketServer.h"
#include "Payout.h"
#include "Database.h"
#include "Settings.h"

using namespace std;

int main()
{

    Settings* settings = new Settings();
    settings->readSettings();


    if (!Database::databaseExists())
        Database::createDatabase();

    SocketServer *socketServer = new SocketServer();
    thread socketThread(&SocketServer::clientListener, socketServer, settings);
    socketThread.detach();

    Payout* payout = new Payout();
    thread payoutThread(&Payout::payoutJob, payout, settings);
    payoutThread.detach();


    while (true) {
        this_thread::sleep_for(std::chrono::seconds(1));
    }

}


