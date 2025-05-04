#include "../features.h"
#include "conn-system.h"

#include <WiFi.h>
#include "esp_netif.h"

#include "geteph.h"
#include "pmu.h"

extern uint32_t netup_time;
extern char eph_nowstr[];

void ConnSystem::init() { 
  /* empty function */
}

void ConnSystem::netsetup() {
  /* empty function, we don't use any network here */
}

void ConnSystem::netshutdown() {
}

void ConnSystem::updateSonde( SondeInfo *si ) {
}

void ConnSystem::updateStation( PosInfo *pi ) {
}

String ConnSystem::getName() {
  return String("TTGO");
}

extern WiFiClient rdzclient;

extern PMU *pmu;

static void appendBatt(char *buf, int maxlen) {
    float batt;
    if(!pmu) {
        if(sonde.config.batt_adc<0) return;
        batt = (float)(analogRead(sonde.config.batt_adc)) / 4095 * 2 * 3.3 * 1.1;
    } else {
        batt = pmu->getBattVoltage() * 0.001;
    }
    int p = strlen(buf);
    snprintf(buf+p, maxlen-p, ", Batt: %.2fV", batt);
}

float batteryVoltage() {
    if (!pmu && sonde.config.batt_adc < 0) {
        return 0.0;
    }
    if (pmu) {
        return pmu->getBattVoltage() * 0.001f; 
    }
    int raw = analogRead(sonde.config.batt_adc);
    float voltage = (float)raw / 4095.0f * 2.0f * 3.3f * 1.1f;
    return voltage;
}


String formatUptime(uint32_t seconds) {
  int days = seconds / 86400;
  int hours = (seconds % 86400) / 3600;
  int minutes = (seconds % 3600) / 60;
  int secs = seconds % 60;

  char buf[64];
  if (days > 0) {
    snprintf(buf, sizeof(buf), "%dd %dh %dm %ds", days, hours, minutes, secs);
  } else {
    snprintf(buf, sizeof(buf), "%dh %dm %ds", hours, minutes, secs);
  }

  return String(buf);
}

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
  char nowstr[30];
  time_t now;
  struct tm timeinfo;
  time(&now);
  gmtime_r(&now, &timeinfo);
  strftime(nowstr, 30, "%Y-%m-%dT%H:%M:%S", &timeinfo);
  p = strlen(buf);
  snprintf(buf+p, 1024-p, ")<br>%s, Uptime: ", nowstr);
  appendUptime(buf, 1024, uptime);
  strlcat(buf, ", WiFi uptime: ", 1024);
  appendUptime(buf, 1024, uptime - netup_time);
  appendBatt(buf, 1024);
  p = strlen(buf);
  snprintf(buf+p, 1024-p, " <br> rdzwxGO app: %sconnected<br>", rdzclient.connected()?"":"not ");
  #if FEATURE_RS92
  p = strlen(buf);
  snprintf(buf+p, 1024-p, "RS92 RINEX eph state: %s", rs92);
  if(ephstate == EPH_GOOD) {
     p = strlen(buf);
     snprintf(buf+p, 1024-p, "[%s]", eph_nowstr);
  }
  #endif
  // get DNS info, debug info...
  String s = WiFi.dnsIP(0).toString();
  strlcat(buf, "<br>DNS: ", 1024);
  strlcat(buf, s.c_str(), 1024);
  s = WiFi.dnsIP(1).toString();
  strlcat(buf, ", DNS2: ", 1024);
  strlcat(buf, s.c_str(), 1024);
  // arduino-esp32 supports only 2 DNS servers, whereas esp-idf can have three....
  // https://github.com/espressif/arduino-esp32/blob/19e4d0db4a5bc2f77c5222c0f12742ff9b98bf76/libraries/Network/src/NetworkInterface.cpp#L691
  // so can't get the backup ip this way...
  // s = WiFi.dnsIP(2).toString();
  esp_netif_dns_info_t d;
  esp_netif_get_dns_info(WiFi.STA.netif(), ESP_NETIF_DNS_FALLBACK, &d);
  s = IPAddress(d.ip.u_addr.ip4.addr).toString();  // IPv4 only this way....
  strlcat(buf, ", DNS3: ", 1024);
  strlcat(buf, s.c_str(), 1024);

  return String(buf);
}

String ConnSystem::getStatusAsJson() {
  uint32_t uptime = esp_timer_get_time() / 1000000;

#if FEATURE_RS92
  const char *rs92 = ephtxt[ephstate];
#else
  const char *rs92 = "not supported in this version";
#endif

  const char *fpstr;
  int i = 0;
  while (fingerprintValue[i] != sonde.fingerprint && fingerprintValue[i] != -1) i++;
  fpstr = (fingerprintValue[i] == -1) ? "Unknown board" : fingerprintText[i];

  time_t now;
  struct tm timeinfo;
  time(&now);
  gmtime_r(&now, &timeinfo);
  char nowstr[30];
  strftime(nowstr, sizeof(nowstr), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);

  // Get fallback DNS
  esp_netif_dns_info_t d;
  esp_netif_get_dns_info(WiFi.STA.netif(), ESP_NETIF_DNS_FALLBACK, &d);
  IPAddress fallbackDNS(d.ip.u_addr.ip4.addr);

  // Assemble clean JSON
  String json = "{";

  json += "\"uptime\":\"" + formatUptime(uptime) + "\",";
  json += "\"wifi_uptime\":\"" + formatUptime(uptime - netup_time) + "\",";
  json += "\"timestamp\":\"" + String(nowstr) + "\",";
  json += "\"fingerprint\":{\"id\":" + String(sonde.fingerprint) + ",\"text\":\"" + escapeJsonString(fpstr) + "\"},";
  json += "\"battery\":{\"voltage\":" + String(batteryVoltage(), 2) + "},";
  json += "\"rdzwxGO_connected\":" + String(rdzclient.connected() ? "true" : "false");

#if FEATURE_RS92
  json += ",\"rs92\":{\"eph_state\":\"" + String(rs92) + "\"";
  if (ephstate == EPH_GOOD) {
    json += ",\"eph_nowstr\":\"" + String(eph_nowstr) + "\"";
  }
  json += "}";
#endif

  json += ",\"dns\":[";
  json += "\"" + WiFi.dnsIP(0).toString() + "\",";
  json += "\"" + WiFi.dnsIP(1).toString() + "\",";
  json += "\"" + fallbackDNS.toString() + "\"";
  json += "]";

  json += "}";

  return json;
}

String ConnSystem::escapeJsonString(const String& input) {
  String escaped = "";
  for (unsigned int i = 0; i < input.length(); i++) {
    char c = input.charAt(i);
    switch (c) {
      case '\"': escaped += "\\\""; break;
      case '\\': escaped += "\\\\"; break;
      case '\b': escaped += "\\b";  break;
      case '\f': escaped += "\\f";  break;
      case '\n': escaped += "\\n";  break;
      case '\r': escaped += "\\r";  break;
      case '\t': escaped += "\\t";  break;
      default:
        if (c >= 0 && c <= 0x1F) {
          escaped += "\\u";
          char buf[5];
          snprintf(buf, sizeof(buf), "%04x", c);
          escaped += String(buf);
        } else {
          escaped += c;
        }
    }
  }
  return escaped;
}

ConnSystem connSystem;
