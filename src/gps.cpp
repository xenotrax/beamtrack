#include "config.h"
#include "states.h"
#include "power.h"
#include <TinyGPS++.h>

HardwareSerial GPS(1);
extern TinyGPSPlus gps;
extern int state;
unsigned long gpsTimer;
unsigned long gpsCount;

TinyGPSPlus getGps()
{
    while (GPS.available())
    {
        gps.encode(GPS.read());
    }
    return gps;
}

void gpsStatus()
{
    gps = getGps();
    Serial.println("\n########## GPS ##########");
    Serial.printf("Latitude   : %f\n", gps.location.lat());
    Serial.printf("Longitude  : %f\n", gps.location.lng());
    Serial.printf("Satellites : %d\n", gps.satellites.value());
    Serial.printf("Altitude   : %g m\n", gps.altitude.meters());
    Serial.printf("Time       : %02d:%02d:%02d\n", gps.time.hour(), gps.time.minute(), gps.time.second());
    Serial.printf("Speed      : %g\n", gps.speed.kmph());
    Serial.println("#########################\n");
    gpsCount = 0;
}

void gpsSetup()
{
    gpsTimer = millis();
    gpsCount = 0;
    GPS.begin(9600, SERIAL_8N1, 34, 12); // IO34: RX, IO12: TX
    powerLed(AXP20X_LED_BLINK_1HZ);
}

void gpsCheck()
{
    if (millis() - gpsTimer >= 1000 * GPS_INT_SECONDS)
    {
        ++gpsCount;
        Serial.printf("Waiting for GPS fix... (%lus)\n", gpsCount * GPS_INT_SECONDS);
        gps = getGps();
        if (gps.location.isValid())
        {
            Serial.printf("Got GPS fix after %lu seconds.\n", gpsCount * GPS_INT_SECONDS);
            gpsStatus();
            powerLed(AXP20X_LED_BLINK_4HZ);
            state = STATE_READY;
        }
        else if (gpsCount * GPS_INT_SECONDS >= GPS_WAIT_SECONDS)
        {
            Serial.printf("No GPS fix after %d seconds, cancelling.\n", GPS_WAIT_SECONDS);
            state = STATE_DONE;
        }
        gpsTimer = millis();
    }
}