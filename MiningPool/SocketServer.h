#pragma once

#include <thread>

#include "Settings.h"
#include "WorkerThread.h"



#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif


class SocketServer
{
public:
	void clientListener( Settings *settings );
};

