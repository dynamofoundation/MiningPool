#pragma once

#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <fcntl.h>


#ifdef _WIN32
#include <io.h>
#endif

#include "Log.h"

using namespace std;

struct sFile {
	int len;
	char* data;
};

class WebPack
{
public:
	void save();
	void load();

	map<string, sFile> pages;

};

