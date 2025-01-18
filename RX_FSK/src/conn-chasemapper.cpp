#include "../features.h"
#if FEATURE_CHASEMAPPER

#include "conn-chasemapper.h"
#include <WiFiUdp.h>

extern const char *sondeTypeStrSH[];
extern WiFiUDP udp;

void ConnChasemapper::init() {
}

void ConnChasemapper::netsetup() {
}

void ConnChasemapper::netshutdown() {
}

void ConnChasemapper::updateSonde(SondeInfo *si) {
	char buf[1024];
	struct tm tim;

        if(!sonde.config.cm.active) return;

	time_t t = si->d.time;
	gmtime_r(&t, &tim);
	uint8_t realtype = si->type;
	if (TYPE_IS_METEO(realtype)) {
		realtype = si->d.subtype == 1 ? STYPE_M10 : STYPE_M20;
	}
        char prefix[10];
        if(realtype == STYPE_RS41) {
            prefix[0] = 0;
        }
        else {
            strncpy(prefix, sondeTypeStrSH[realtype], 10);
            strcat(prefix, "-");
        }
	sprintf(buf, "{ \"type\": \"PAYLOAD_SUMMARY\","
		"\"callsign\": \"%s%s\","
		"\"latitude\": %.5f,"
		"\"longitude\": %.5f,"
		"\"altitude\": %d,"
		"\"speed\": %d,"
		"\"heading\": %d,"
		"\"time\": \"%02d:%02d:%02d\","
		"\"model\": \"%s\","
		"\"freq\": \"%.3f MHz\"",
                prefix,
		si->d.ser,
		si->d.lat,
		si->d.lon,
		(int)si->d.alt,
		(int)(si->d.hs * 1.9438445),  // m/s into knots
		(int)si->d.dir,
		tim.tm_hour, tim.tm_min, tim.tm_sec,
		sondeTypeStrSH[realtype],
		si->freq);
	if( !isnan(si->d.temperature) ) {
		sprintf(buf + strlen(buf), ", \"temp\": %.1f", si->d.temperature);
	}
	strcat(buf, "}");
	Serial.printf("Sending chasemapper json: %s\n", buf);
	udp.beginPacket(sonde.config.cm.host, sonde.config.cm.port);
	udp.write((const uint8_t *)buf, strlen(buf));
	udp.endPacket();	
}

void ConnChasemapper::updateStation(PosInfo *pi) {
}

String ConnChasemapper::getStatus() {
        if(!sonde.config.cm.active) return String("disabled");
        char info[100];
	snprintf(info, 100, "active [%s:%d]", sonde.config.cm.host, sonde.config.cm.port);
	return String(info);
}

String ConnChasemapper::getName() {
	return String("Chasemapper");
}

ConnChasemapper connChasemapper;
#endif
