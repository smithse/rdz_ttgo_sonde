#ifndef _JSON_H
#define _JSON_H

#include "Sonde.h"

int sonde2json(char *buf, int maxlen, SondeInfo *si, bool rssi_as_dbm=false);

#endif
