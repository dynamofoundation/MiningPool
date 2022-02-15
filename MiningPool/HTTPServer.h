#pragma once

#include <thread>

#include "Settings.h"
#include "HTTPThread.h"
#include "Global.h"

#ifdef _WIN32
#include <windows.h>
#endif

class HTTPServer
{

public:
	void clientListener(Global* global);

};

