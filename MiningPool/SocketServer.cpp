#include "SocketServer.h"


void SocketServer::clientListener( Settings *settings ) {

	Log::log ("Starting socket server on port %d.\n", settings->clientListenPort);

	int serverSocket;

    sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        Log::fatalError("Cannot create listen socket.");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(settings->clientListenPort);

    if (::bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0)
        Log::fatalError("Failed to bind socket.");

    
    if (listen(serverSocket, 3) < 0)
        Log::fatalError("Failed to listen on socket.");



	while (true) {

        int newClientSocket;
        if ((newClientSocket = accept(serverSocket, (struct sockaddr*)&address,  (socklen_t*)&addrlen)) > 0)
        {
            WorkerThread* worker = new WorkerThread();
            thread workerthread(&WorkerThread::clientWorker, worker, newClientSocket);
            workerthread.detach();

        }


	}
	

}