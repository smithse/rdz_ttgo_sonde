#include "../features.h"
#if FEATURE_MQTT

#ifndef MQTT_h
#define MQTT_h

#include <WiFi.h>
#include <AsyncMqttClient.h>
#include "Sonde.h"
#include "conn.h"

#define MQTT_SEND_SONDE 0x01
#define MQTT_SEND_UPTIME 0x02
#define MQTT_SEND_PMU 0x04
#define MQTT_SEND_GPS 0x08
#define MQTT_SEND_RFINFO 0x10
#define MQTT_SEND_DEBUG 0x80
#define MQTT_SEND_ANY (MQTT_SEND_UPTIME|MQTT_SEND_SONDE|MQTT_SEND_PMU|MQTT_SEND_GPS|MQTT_SEND_RFINFO|MQTT_SEND_DEBUG)

class MQTT : public Conn
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
        void updateStation( PosInfo *pi );

        /* Say whether MQTT is connected, disconnected, or even enabled */
	String getStatus();

        String getName();

        /* Radio debug - spectrum and scanner*/
        void publishPeak(double pf, int rssi);
        void publishQRG(int num, const char* type, char* launchsite, float mhz);
        void publishDebug(char* debugmsg);

       private:
        WiFiClient mqttWifiClient;
        AsyncMqttClient mqttClient;
        TimerHandle_t mqttReconnectTimer;
        IPAddress ip;
        // uint16_t port;
        // const char *username;
        // const char *password;
        // const char *prefix;
        char clientID[21];

        // void init(const char *host, uint16_t port, const char *id, const char
        // *username, const char *password, const char *prefix);
        void publishPacket(SondeInfo* s);
        void publishUptime();
        void publishPmuInfo();
        void publishGps();
        void timeFormat();
        int mqttGate(uint flag);
        int connectToMqtt();

        unsigned long lastMqttUptime = 0;
        boolean mqttEnabled;
};

extern MQTT connMQTT;
#endif

#endif
