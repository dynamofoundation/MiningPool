//pass different extra nonce to each client
//tcp port based
//adjust diff so block submission hits target
//payout logic - frequency / minimum / fee
//drop connections with high reject
#include <thread>


#include "SocketServer.h"
#include "Payout.h"
#include "Database.h"


using namespace std;

int main()
{

    if (!Database::databaseExists())
        Database::createDatabase();

    SocketServer *socketServer = new SocketServer();
    thread socketThread(&SocketServer::clientListener, socketServer);
    socketThread.detach();

    Payout* payout = new Payout();
    thread payoutThread(&Payout::payoutJob, payout);
    payoutThread.detach();


    while (true) {
        this_thread::sleep_for(std::chrono::seconds(1));
    }

}


