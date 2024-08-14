#include "../features.h"
#include "conn-system.h"

#include <WiFi.h>
#include "geteph.h"

extern uint32_t netup_time;

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
  uint16_t up_d = uptime / (24*3600);
  uptime -= (24*3600) * up_d;
  uint16_t up_h = uptime / 3600;
  uptime -= 3600 * up_h;
  uint16_t up_m = uptime / 60;
  uint16_t up_s = uptime - 60 * up_m;

  uint32_t nuptime = uptime - netup_time;
  uint16_t nup_d = nuptime / (24*3600);
  nuptime -= (24*3600) * nup_d;
  uint16_t nup_h = nuptime / 3600;
  nuptime -= 3600 * nup_h;
  uint16_t nup_m = nuptime / 60;
  uint16_t nup_s = nuptime - 60 * nup_m;

  char buf[1024];
#if FEATURE_RS92
  const char *rs92 = ephtxt[ephstate];
#else
  const char *rs92 = "not supported in this version";
#endif
  snprintf(buf, 1024, "Uptime: %0dd %02d:%02d:%02d, WiFi uptime: %0dd %02d:%02d:%02d <br> rdzwxGO app: %sconnected<br>RS92 RINEX eph state: %s",
    up_d, up_h, up_m, up_s,
    nup_d, nup_h, nup_m, nup_s, rdzclient.connected()?"":"not ", rs92);
  return String(buf);
}


ConnSystem connSystem;
