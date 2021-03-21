#include "config.h"
#include "states.h"
#include "power.h"
#include <Arduino_LoRaWAN_ttn.h>
#include <lmic.h>
#include <hal/hal.h>
#include <CayenneLPP.h>

extern int state;
int retry;

class cLoRaWAN : public Arduino_LoRaWAN_ttn
{
public:
    cLoRaWAN(){};

protected:
    virtual bool GetOtaaProvisioningInfo(Arduino_LoRaWAN::OtaaProvisioningInfo *) override;
    virtual void NetSaveFCntUp(uint32_t uFCntUp) override;
    virtual void NetSaveFCntDown(uint32_t uFCntDown) override;
    virtual void NetSaveSessionInfo(const SessionInfo &Info, const uint8_t *pExtraInfo, size_t nExtraInfo) override;
};

bool cLoRaWAN::GetOtaaProvisioningInfo(
    OtaaProvisioningInfo *pInfo)
{
    static const uint8_t DEVEUI[8] = TTN_DEVEUI;
    static const uint8_t APPEUI[8] = TTN_APPEUI;
    static const uint8_t APPKEY[16] = TTN_APPKEY;

    if (pInfo)
    {
        memcpy(pInfo->AppKey, APPKEY, sizeof(APPKEY));
        memcpy(pInfo->DevEUI, DEVEUI, sizeof(DEVEUI));
        memcpy(pInfo->AppEUI, APPEUI, sizeof(APPEUI));
    }
    return true;
}

void cLoRaWAN::NetSaveFCntDown(uint32_t uFCntDown)
{
}

void cLoRaWAN::NetSaveFCntUp(uint32_t uFCntUp)
{
}

void cLoRaWAN::NetSaveSessionInfo(
    const SessionInfo &Info,
    const uint8_t *pExtraInfo,
    size_t nExtraInfo)
{
}

const cLoRaWAN::lmic_pinmap pinMap = {
    .nss = 18,
    .rxtx = cLoRaWAN::lmic_pinmap::LMIC_UNUSED_PIN,
    .rst = 23,
    .dio = {26, 33, 32},
};

cLoRaWAN LoRaWAN{};

void loraSetup()
{
    LoRaWAN.begin(pinMap);
}

void loraLoop()
{
    LoRaWAN.loop();
}

int loraSend(CayenneLPP payload)
{
    Serial.println("Sending payload...");
    if (LMIC.opmode & OP_TXRXPEND)
    {
        Serial.println(F("TX/RX operation still pending, not sending payload."));
        return STATE_READY;
    }
    else
    {
        LMIC_setTxData2(1, payload.getBuffer(), payload.getSize(), 0);
        Serial.println(F("Payload queued for sending."));
    }
    return STATE_SENDING;
}

void onEvent(ev_t ev)
{
    Serial.printf("%d: ", os_getTime());
    switch (ev)
    {
    case EV_SCAN_TIMEOUT:
        Serial.println(F("EV_SCAN_TIMEOUT"));
        break;
    case EV_BEACON_FOUND:
        Serial.println(F("EV_BEACON_FOUND"));
        break;
    case EV_BEACON_MISSED:
        Serial.println(F("EV_BEACON_MISSED"));
        break;
    case EV_BEACON_TRACKED:
        Serial.println(F("EV_BEACON_TRACKED"));
        break;
    case EV_JOINING:
        Serial.println(F("EV_JOINING"));
        retry = 0;
        break;
    case EV_JOINED:
        Serial.println(F("EV_JOINED"));
        powerLed(AXP20X_LED_LOW_LEVEL);
        break;
    case EV_RFU1:
        Serial.println(F("EV_RFU1"));
        break;
    case EV_JOIN_FAILED:
        Serial.println(F("EV_JOIN_FAILED"));
        break;
    case EV_REJOIN_FAILED:
        Serial.println(F("EV_REJOIN_FAILED"));
        break;
        break;
    case EV_TXCOMPLETE:
        Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
        if (LMIC.txrxFlags & TXRX_ACK)
            Serial.println(F("Received ack"));
        if (LMIC.dataLen)
        {
            Serial.println(F("Received "));
            Serial.println(LMIC.dataLen);
            Serial.println(F(" bytes of payload"));
        }
        // go to sleep
        state = STATE_DONE;
        break;
    case EV_LOST_TSYNC:
        Serial.println(F("EV_LOST_TSYNC"));
        break;
    case EV_RESET:
        Serial.println(F("EV_RESET"));
        break;
    case EV_RXCOMPLETE:
        Serial.println(F("EV_RXCOMPLETE"));
        break;
    case EV_LINK_DEAD:
        Serial.println(F("EV_LINK_DEAD"));
        break;
    case EV_LINK_ALIVE:
        Serial.println(F("EV_LINK_ALIVE"));
        break;
    case EV_SCAN_FOUND:
        Serial.println(F("EV_SCAN_FOUND"));
        break;
    case EV_TXSTART:
        Serial.println(F("EV_TXSTART"));
        break;
    case EV_TXCANCELED:
        Serial.println(F("EV_TXCANCELLED"));
        break;
    case EV_JOIN_TXCOMPLETE:
        Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
        if (retry < LORA_RETRY)
        {
            //Serial.printf("\nLoRa Join failed, %d retries left.\n\n", LORA_RETRY - retry);
            // TODO: this will cause "CORRUPT HEAP: Bad head at / assertion "head != NULL" failed / abort() was called"
            //state = STATE_INIT;
            
            Serial.println("\nLoRa Join failed.");
            state = STATE_DONE;
        }
        else
        {
            Serial.println("\nLoRa Join failed, no retries left.");
            state = STATE_DONE;
        }
        break;
    default:
        Serial.println(F("Unknown event " + (unsigned)ev));
        break;
    }
}