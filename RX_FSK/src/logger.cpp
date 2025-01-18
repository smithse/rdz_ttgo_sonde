#include "logger.h"
#include <stdio.h>
#include "colors.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#if USE_BAD_DEBUGGING_CODE
extern WiFiUDP udp;
#endif

extern boolean connected;

extern const char *version_name;
extern const char *version_id;

const char *lvlcol[]={RED, YEL, GRN, BLU};

void Logger::init() {
	sonde.config.debug = 3; // Use as initial value until config file is read from file system
}

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
	if(color) { 
		int x = strlen(buf);
		if(buf[x-1]=='\n') { strlcpy(buf+x-1, COLOR_RESET, 512-x+1); strlcpy(buf+x-1+sizeof(COLOR_RESET)-1, "\n", 512-x+1-sizeof(COLOR_RESET)+1); }
		else strlcat(buf, COLOR_RESET, 512); 
	}
	Serial.print(buf);
}

Logger Log;


// Let's put this here as it is serial communication as well?

void Logger::sendImprov(int type, int len, const char *data) {
    char buf[100];
    strcpy(buf, "IMPROV\x01");
    buf[7] = type;
    buf[8] = len;
    memcpy(buf+9, data, len);
    int crc = 0;
    for(int i=0; i<9+len; i++) crc += buf[i];
    buf[9+len] = crc & 0xff;
    buf[9+len+1] = '\n';
    buf[9+len+2] = 0;
    Serial.write(buf, 9+len+2);
#if USE_BAD_DEBUGGING_CODE
            udp.beginPacket("192.168.1.3", 12345);
	    udp.write((const uint8_t *)"Reply:",6);
            udp.write((const uint8_t *)buf, 9+len+2);
            udp.endPacket();
#endif
}

void Logger::sendImprovResult(int replyto, const char *strings[]) {
    char buf[250];
    strcpy(buf, "IMPROV\x01");
    buf[7] = 0x04; // type: RPC Response
    //buf[8] will be length, set at the end
    buf[9] = replyto;
    //buf[10] will be length, set at the end
    int i = 11;
    int nstr = 0;
    while(*strings) {
        int len = strlen(*strings);
        buf[i] = len;
        memcpy(buf+i+1, *strings, len);
        i += len+1;
        strings++;
        nstr++;
    }
    buf[8] = i-9;
    buf[10] = i-11;

    int crc = 0; 
    for(int j=0; j<i; j++) crc += buf[j];
    buf[i++] = crc & 0xff;
    buf[i++] = '\n';
    buf[i++] = 0; 
    Serial.write(buf, i-1);
#if USE_BAD_DEBUGGING_CODE
            udp.beginPacket("192.168.1.3", 12345);
	    udp.write((const uint8_t *)"Reply:",6);
            udp.write((const uint8_t *)buf, i-1);
            udp.endPacket();
#endif
}

int cmdlen = 0;
char cmd[50];

#if 0
void Logger::handleImprovByte(char c) {
    cmd[cmdlen++] = c;
    if(cmdlen==1)
	return c == 'I';
    else if(cmdlen==2)
	return c == 'M';
    else if(cmdlen==3)
	return c == 'P';
    else if(cmdlen==4)
	return c == 'R';
    else if(cmdlen==5)
	return c == 'O';
    else if(cmdlen==6)
	return c == 'V';
    else if(cmdlen==7)
	return c == 1; // version number
    else if(cmdlen<10)
	return 1;      // read type and len
    // type is cmd[7]
    // len is cmd[8]
    // wait to read 'len' more bytes...
    if(cmdlen < 8 + cmd[8] + 1) return 1;
}
#endif

extern int updateWiFi(String ssid, String pw);


void Logger::handleImprov() {
    while(Serial.available()) {
        cmd[cmdlen] = Serial.read();
        if(cmd[cmdlen] == '\n') { // check if command
#if USE_BAD_DEBUGGING_CODE
            udp.beginPacket("192.168.1.3", 12345);
            udp.write((const uint8_t *)cmd, cmdlen+1);
            udp.endPacket();
#endif
            if(strncmp(cmd, "IMPROV", 6)==0) {  // we have a command
                // TODO: CHeck CRC
                if(cmd[7]==0x03 && cmd[9]==0x03) { // RPC, get info
		    const char *info[]={version_name, version_id, ESP.getChipModel(), "rdzSonde", NULL};
                    sendImprovResult(0x03, info);
                }
		if(cmd[7]==0x03 && cmd[9]==0x02) { // request state
		    if(!connected) {
		        sendImprov(0x01, 1, "\x02"); // current state: ready
		    } else {
			sendImprov(0x01, 1, "\x04"); // cuurent state: provisioned
			// Send "next" url
			char url[50];
			const char *i[]={url, NULL};
			String ip = WiFi.localIP().toString();
			snprintf(url, 50, "http://%s/", ip.c_str());
			sendImprovResult(0x02, i);
		    }
		}
		if(cmd[7]==0x03 && cmd[9]==0x04) { // wifi scan, fake for now
		    //const char *info[]={"Dinosauro", "-60", "YES", NULL};
		    //sendImprovResult(0x04, info);
		    const char *i2[]={ NULL};
		    sendImprovResult(0x04, i2);
		}
		if(cmd[7]==0x03 && cmd[9]==0x01) { // send Wi-Fi settings
		    // data len, ssid len, ssid bytes, pw len, pw bytes
		    String ssid = String(cmd+12, cmd[11]);
		    int pwpos = 12+cmd[11];
		    String pw = String(cmd+pwpos+1, cmd[pwpos]);
		    updateWiFi(ssid, pw);
		}
            }
            cmdlen = 0;
        } else {
            cmdlen++;
            if(cmdlen>=50) cmdlen = 0;  // avoid overflow...
        }
    }
}
