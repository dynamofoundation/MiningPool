#pragma once

#include <thread>

#include "Settings.h"
#include "WorkerThread.h"
#include "Global.h"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h> 
#include <unistd.h>
#endif


class SocketServer
{
public:
	void clientListener( Global *global );
};

