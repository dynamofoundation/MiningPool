#pragma once

#include <thread>

#include "Settings.h"
#include "WorkerThread.h"

#ifdef _WIN32
#include <windows.h>
#endif


class SocketServer
{
public:
	void clientListener( Settings *settings );
};

