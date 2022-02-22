#pragma once

#include <thread>

#include "Settings.h"
#include "HTTPThread.h"
#include "Global.h"

#ifdef __linux__
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h> 
#include <unistd.h>
#endif


#ifdef _WIN32
#include <windows.h>
#endif

class HTTPServer
{

public:
	void clientListener(Global* global);

};

