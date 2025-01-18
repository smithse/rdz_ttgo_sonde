
#include "conn.h"

void Conn::netsetup() {}

void Conn::netshutdown() {}

void Conn::appendUptime(char *str, int maxlen, uint32_t uptime) {
  int l = strlen(str);
  if(l>maxlen) return;
  maxlen -= l;
  str += l;

  uint16_t up_d = uptime / (24*3600);
  uptime -= (24*3600) * up_d;
  uint16_t up_h = uptime / 3600;
  uptime -= 3600 * up_h;
  uint16_t up_m = uptime / 60;
  uint16_t up_s = uptime - 60 * up_m;
  snprintf(str, maxlen, "%0dd %02d:%02d:%02d", up_d, up_h, up_m, up_s);
}

void Conn::escapeJson(char *dst, const char *src, int maxlen) {
// take first line (until \r), skip until \r\r,
    int state = 0;
    while (*src && maxlen>1) {
        if(state==0) { if(*src=='\r') { state=1; src++; *dst++=' '; maxlen--; continue; } }  // take first line
        if(state==1) { if(*src=='\r') { state=2; } src++; continue; }
        if(state==2) { if(*src!='\r' && *src!='\n') { state=1; } if(*src=='\r') { state=3; } src++; continue; }

        if (*src == '"' || *src == '\\' || ('\x00' <= *src && *src <= '\x1f')) {
            snprintf(dst, maxlen, "\\u%04x", (int)*src);
            int n = strlen(dst);
            maxlen -= n;
            dst += n;
            src++;
        } else {
            *dst++ = *src++; maxlen--;
        }
    }
    *dst = 0;
}

