#include "../features.h"
#if FEATURE_SONDESEEKER

#include "conn-sondeseeker.h"
#include <WiFiUdp.h>
#include "posinfo.h"
#include "json.h"

extern const char *sondeTypeStrSH[];
extern WiFiUDP udp;

void ConnSondeseeker::init() {
}

void ConnSondeseeker::netsetup() {
}

void ConnSondeseeker::netshutdown() {
}

void ConnSondeseeker::updateSonde(SondeInfo *si) {
	char buf[1024];

	strcpy(buf, "{\"sonde\": {");
	sonde2json(buf + strlen(buf), 1024, si);
	strcat(buf, "}}");

	Serial.printf("Sending SondeSeeker json: %s\n", buf);
	udp.beginPacket(sonde.config.ss.host, sonde.config.ss.port);
	udp.write((const uint8_t *)buf, strlen(buf));
	udp.endPacket();

}

void ConnSondeseeker::updateStation(PosInfo *pi) {
}

String ConnSondeseeker::getStatus() {
        if(!sonde.config.ss.active) return String("disabled");
        char info[100];
	snprintf(info, 100, "active [%s:%d]", sonde.config.ss.host, sonde.config.ss.port);
	return String(info);
}

String ConnSondeseeker::getName() {
	return String("SondeSeeker");
}

ConnSondeseeker connSondeseeker;
#endif
