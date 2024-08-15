#include "../features.h"
#include "conn-system.h"

#include <WiFi.h>
#include "geteph.h"

extern uint32_t netup_time;
extern char eph_nowstr[];

void ConnSystem::init() { 
  /* empty function */
}

void ConnSystem::netsetup() {
  /* empty function, we don't use any network here */
}

void ConnSystem::updateSonde( SondeInfo *si ) {
}

void ConnSystem::updateStation( PosInfo *pi ) {
}

String ConnSystem::getName() {
  return String("TTGO");
}

extern WiFiClient rdzclient;

String ConnSystem::getStatus() {
  /* Special connector for obtaining system status.... */
  // uptime
  uint32_t uptime = esp_timer_get_time()/1000000;

  char buf[1024];
#if FEATURE_RS92
  const char *rs92 = ephtxt[ephstate];
#else
  const char *rs92 = "not supported in this version";
#endif

  int i = 0;
  const char *fpstr;
  while (fingerprintValue[i] != sonde.fingerprint && fingerprintValue[i] != -1) i++;
  if (fingerprintValue[i] == -1) {
    fpstr = "Unknown board";
  } else {
    fpstr = fingerprintText[i];
  }
  snprintf(buf, 1024, "Autodetect info: Fingerprint %d (", sonde.fingerprint);
  int p = strlen(buf);
  escapeJson(buf+p, fpstr, 1024-p);

  strlcat(buf, ")<br>Uptime: ", 1024);
  appendUptime(buf, 1024, uptime);
  strlcat(buf, ", WiFi uptime: ", 1024);
  appendUptime(buf, 1024, uptime - netup_time);
  p = strlen(buf);
  snprintf(buf+p, 1024-p, " <br> rdzwxGO app: %sconnected<br>RS92 RINEX eph state: %s", rdzclient.connected()?"":"not ", rs92);
  if(ephstate == EPH_GOOD) {
     p = strlen(buf);
     snprintf(buf+p, 1024-p, "[%s]", eph_nowstr);
  }
  return String(buf);
}


ConnSystem connSystem;
