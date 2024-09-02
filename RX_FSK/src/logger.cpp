
#include "logger.h"
#include <stdio.h>

void Logger::logf(LOGLEVEL lvl, const char *module, const char *fmt, ...) {
	char buf[256];
 	va_list ptr;
	va_start(ptr, fmt);
	strncpy(buf, module, 20);
        strcat(buf, ": ");
        int len = strlen(buf);
	vsnprintf(buf+len, 255-len, fmt, ptr);
	Serial.print(buf);
}

Logger Log;
