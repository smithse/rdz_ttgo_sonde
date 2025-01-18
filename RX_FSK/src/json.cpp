#include "json.h"
#include "RS41.h"
#include "DFM.h"

extern const char *sondeTypeStrSH[];
//extern const char *dfmSubtypeStrSH[];

static char typestr[11];

const char *getType(SondeInfo *si) {
    if( si->type == STYPE_RS41 ) {
        if ( RS41::getSubtype(typestr, 11, si) == 0 ) return typestr;
    } else if ( TYPE_IS_DFM(si->type) && si->d.subtype > 0 ) {
        const char *t = dfmSubtypeLong[si->d.subtype & 0xf];
	if( (si->d.subtype & 0xf) == DFM_UNK) {
           sprintf(typestr, "DFMx%X", si->d.subtype>>4);
           return typestr;
	}
        return t;
    }
    return sondeTypeStrSH[sonde.realType(si)];
}

int float2json(char **buf, int *maxlen, const char *fmt, float value) {
   if(isnan(value)) return 0;
   int n = snprintf(*buf, *maxlen, fmt, value);
   if(n>*maxlen) return -1;
   *buf += n; *maxlen -= n;
   return n;
}
   
// To be used by
// - MQTT
// - rdzJSON (for Android app)
// - Web map
int sonde2json(char *buf, int maxlen, SondeInfo *si, bool rssi_as_dbm)
{
    SondeData *s = &(si->d);
    int n;
    float rssi_conversion = 1.0;

    // this allows callers to get the raw reading and convert it themselves (mobile app)
    // or to request the conversion to be done internally for the benefit of downstream
    // consumers (mqtt subsystem, and anything else that calls this function)
    if (rssi_as_dbm)
      rssi_conversion = -0.5;

    n = float2json(&buf, &maxlen, "\"lat\": %.5f,", s->lat);
    if(n<0) return -1;
    n = float2json(&buf, &maxlen, "\"lon\": %.5f,", s->lon);
    if(n<0) return -1;
    n = float2json(&buf, &maxlen, "\"alt\": %.1f,", s->alt);
    if(n<0) return -1;
    n = float2json(&buf, &maxlen, "\"vs\": %.1f,", s->vs);
    if(n<0) return -1;
    n = float2json(&buf, &maxlen, "\"hs\": %.1f,", s->hs);
    if(n<0) return -1;
    n = float2json(&buf, &maxlen, "\"climb\": %.1f,", s->vs); // used by HTML map, to be removed (-> vs)
    if(n<0) return -1;
    n = float2json(&buf, &maxlen, "\"speed\": %.1f,", s->hs); // used by HTML map, to be removed (-> hs)
    if(n<0) return -1;
    n = float2json(&buf, &maxlen, "\"dir\": %.1f,", s->dir);
    if(n<0) return -1;
    n = float2json(&buf, &maxlen, "\"temp\": %.1f,", s->temperature );
    if(n<0) return -1;
    n = float2json(&buf, &maxlen, "\"humidity\": %.1f,", s->relativeHumidity);
    if(n<0) return -1;
    n = float2json(&buf, &maxlen, "\"pressure\": %.1f,", s->pressure);
    if(n<0) return -1;
    n = snprintf(buf, maxlen, 
        "\"type\":\"%s\","
        "\"id\": \"%s\","       // TODO: maybe remove in the future, ser is enough, client can calculate APRS id if needed
        "\"ser\": \"%s\","
        "\"frame\": %u,"	// raw frame, from sonde, can be 0. (TODO: add virtual frame # as in sondehub?)
        "\"vframe\": %d,"	
        "\"time\": %u,"
        "\"sats\": %d,"
        "\"freq\": %.2f,"
        "\"rssi\": %.1f,"
        "\"afc\": %d,"
        "\"launchKT\": %d,"
        "\"burstKT\": %d,"
        "\"countKT\": %d,"
        "\"crefKT\": %d,"
	"\"launchsite\": \"%s\","
	"\"res\": %d",
        getType(si),
        s->id,
        s->ser,
        s->frame,
        s->vframe,
        s->time,
        s->sats,
        si->freq,
        si->rssi * rssi_conversion,
        si->afc,
        s->launchKT,
        s->burstKT,
        s->countKT,
        s->crefKT,
	si->launchsite,
	(int)si->rxStat[0]
    );
    if(n>=maxlen) return -1;
    buf += n; maxlen -= n;

    // add only if available
    if(s->batteryVoltage > 0) {
	n = snprintf(buf, maxlen, ",\"batt\": %.1f", s->batteryVoltage);
	if(n>=maxlen) return -1;
	buf += n; maxlen -= n;
    }
    return 0;
}


