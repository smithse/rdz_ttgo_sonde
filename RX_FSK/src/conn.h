/* 
 * conn.h
 * Interface for external data exporters
 * Copyright (c) 2023 Hansi Reiser, dl9rdz
 */

#ifndef conn_h
#define conn_h

#include "Sonde.h"


// to be moved elsewhere
struct PosInfo {
public:
	float lat;
	float lon;
};


/* Interface for all data exporters */
class Conn
{
public: 
	/* Called once on startup */
	virtual void init();

	/* Called whenever the network becomes available */
	virtual void netsetup();

        /* Called when the network is shut down or when reconfiguration is forced, netshutdown connections... */
        virtual void netshutdown();

	/* Called approx 1x / second (maybe only if good data is available) */
	virtual void updateSonde( SondeInfo *si );

	/* Called approx 1x / second* */
	virtual void updateStation( PosInfo *pi );

	/* Called to retrieve status (used for Info in about tab) */
	virtual String getStatus();

        /* Called to retrieve the name of this connector (for status display) */
	virtual String getName();

	static void appendUptime(char *str, int maxlen, uint32_t uptime);
	static void escapeJson(char *dst, const char *src, int maxlen);

};
#endif
