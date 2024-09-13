
#include "logger.h"
#include <stdio.h>
#include "colors.h"

const char *lvlcol[]={RED, YEL, GRN, BLU};

void Logger::logf(LOGLEVEL lvl, const char *module, const char *fmt, ...) {
	int color = sonde.config.debug >= 10 ? 1 : 0;
	int cfglvl = sonde.config.debug - 10*color;
	if(lvl > cfglvl) return;
	char buf[512];
 	va_list ptr;
	va_start(ptr, fmt);
	*buf = 0;
 	if(color) { strlcat(buf, lvlcol[lvl], 512); }
	strlcat(buf, module, 512);
        strcat(buf, ": ");
        int len = strlen(buf);
	vsnprintf(buf+len, 512-len, fmt, ptr);
	if(color) { strlcat(buf, COLOR_RESET, 512); }

	Serial.print(buf);
}

Logger Log;
