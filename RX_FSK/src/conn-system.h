/*
 * conn-sdcard.h
 * Data exporter to SD card
 * Copyright (c) 2023 Hansi Reiser, dl9rdz
 * 
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef conn_system_h
#define conn_system_h

#include "conn.h"


class ConnSystem : public Conn
{
public:
        /* Called once on startup */
        void init();

        /* Called whenever the network becomes available */
        void netsetup();

        /* Close connections */
        void netshutdown();

        /* Called approx 1x / second (maybe only if good data is available) */
        virtual void updateSonde( SondeInfo *si );

        /* Called approx 1x / second* */
        virtual void updateStation( PosInfo *pi );

        String getStatus();

        String getName();

private:

};

extern ConnSystem connSystem;


#endif
