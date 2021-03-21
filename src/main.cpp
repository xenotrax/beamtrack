// https://tum-gis-sensor-nodes.readthedocs.io/en/ttgo/wemos_ttgo_t-beam/README.html
// https://www.thethingsnetwork.org/forum/t/ttgo-t-beam/15297/266
// https://git.faked.org/jan/bikebeam/-/tree/master/
// https://github.com/LilyGO/TTGO-T-Beam
// https://www.thethingsnetwork.org/forum/t/mqtt-in-node-red-howto/39909
// https://www.thethingsnetwork.org/forum/t/ttgo-t-beam/15297/325 (  SX127x vs SX1262 LoRa module )


#include "config.h"
#include "states.h"
#include "power.h"
#include "gps.h"
#include "lora.h"
#include "button.h"
#include <CayenneLPP.h>

int state;
CayenneLPP payload(55);
TinyGPSPlus gps;

void createPayload()
{
    payload.reset();
    gps = getGps();
    payload.addGPS(1, gps.location.lat(), gps.location.lng(), gps.altitude.meters());
    payload.addAnalogInput(2, powerGetBattVoltage() / 1000);
}

void printPayload() {
    DynamicJsonDocument jsonBuffer(1024);
    JsonObject json = jsonBuffer.to<JsonObject>();
    payload.decodeTTN(payload.getBuffer(), payload.getSize(), json);
    Serial.println("\n########## Payload ##########");
    serializeJsonPretty(json, Serial);
    Serial.println("\n#########################\n");
}

void setup()
{
    Serial.begin(115200);
    Wire.begin(21, 22);
    powerBootInfo();
    powerSetup();
    gpsSetup();
    loraSetup();
    buttonSetup();
    state = STATE_INIT;
}

void loop()
{
    loraLoop();
    buttonLoop();

    switch (state)
    {
    case STATE_INIT:
        gpsCheck();
        break;
    case STATE_READY:
        createPayload();
        state = loraSend(payload); // success: STATE_SENDING, error: STATE_READY
        printPayload();
        break;
    case STATE_SENDING:
        // idle
        break;
    case STATE_DONE:
        powerSleep();
        break;
    default:
        break;
    }
}
