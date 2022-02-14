#include "Log.h"



void Log::fatalError(const char* message) {
	printf("FATAL ERROR: %s\n", message);
	exit(-1);
}


void Log::log(const char* format, ...) {
	va_list args;
	va_start(args, format);
	vprintf( format, args);
	va_end(args);
}