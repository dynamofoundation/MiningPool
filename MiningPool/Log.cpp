#include "Log.h"


void Log::fatalError(const char* message) {
	printf("FATAL ERROR: %s\n", message);
	exit(-1);
}