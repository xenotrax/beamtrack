#include "config.h"
#include <axp20x.h>

RTC_DATA_ATTR int bootCount = 0;

AXP20X_Class axp;

void powerLed(axp_chgled_mode_t mode)
{
    axp.setChgLEDMode(mode);
}

float powerGetBattVoltage()
{
    return axp.getBattVoltage();
}

void powerStatus()
{
    Serial.println("\n########## Power ##########");
    Serial.printf("DCDC1/OLED Status   : %s\n", axp.isDCDC1Enable() ? "enabled" : "disabled");
    Serial.printf("DCDC1/OLED Voltage  : %g V\n", (float)axp.getDCDC1Voltage() / 1000);
    Serial.printf("DCDC2/N/C Status    : %s\n", axp.isDCDC2Enable() ? "enabled" : "disabled");
    Serial.printf("DCDC2/N/C Voltage   : %g V\n", (float)axp.getDCDC2Voltage() / 1000);
    Serial.printf("DCDC3/ESP32 Status  : %s\n", axp.isDCDC3Enable() ? "enabled" : "disabled");
    Serial.printf("DCDC3/ESP32 Voltage : %g V\n", (float)axp.getDCDC3Voltage() / 1000);
    Serial.printf("LDO2/LoRa Status    : %s\n", axp.isLDO2Enable() ? "enabled" : "disabled");
    Serial.printf("LDO2/LoRa Voltage   : %g V\n", (float)axp.getLDO2Voltage() / 1000);
    Serial.printf("LDO3/GPS Status     : %s\n", axp.isLDO3Enable() ? "enabled" : "disabled");
    Serial.printf("LDO3/GPS Voltage    : %g V\n", (float)axp.getLDO3Voltage() / 1000);
    Serial.println("--- Battery ---");
    Serial.printf("Battery Connected: %s\n", axp.isBatteryConnect() ? "true" : "false");
    if (axp.isBatteryConnect())
    {
        Serial.printf("Battery Percentage : %d %%\n", axp.getBattPercentage());
        Serial.printf("Battery Voltage    : %g V\n", axp.getBattVoltage() / 1000);
        Serial.printf("Battery Current    : %g mA\n", axp.getBattDischargeCurrent());
        Serial.println("--- Charging ---");
        Serial.printf("Charging Enabled   : %s\n", axp.isChargeingEnable() ? "true" : "false");
        Serial.printf("Battery Charging   : %s\n", axp.isChargeing() ? "true" : "false");
        Serial.printf("Set Charge Current : %g mA\n", axp.getSettingChargeCurrent());
    }
    Serial.println("--- Chip ---");
    Serial.printf("Chip Temp : %.1f C°\n", axp.getTemp() / 10);
    Serial.println("#########################\n");
}

void powerSetup()
{
    if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS))
    {
        Serial.println("AXP192 Begin PASS");
    }
    else
    {
        Serial.println("AXP192 Begin FAIL");
    }
    axp.setPowerOutPut(AXP192_DCDC1, AXP202_OFF); // OLED  : off
    axp.setPowerOutPut(AXP192_DCDC2, AXP202_OFF); // N/C   : off
    axp.setPowerOutPut(AXP192_DCDC3, AXP202_ON);  // ESP32 : on
    axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);   // LORA  : on
    axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);   // GPS   : on
    /*
    axp.setDCDC3Voltage(3300);                    // ESP32 : 3.3V
    axp.setLDO2Voltage(3300);                     // LORA  : 3.3V
    axp.setLDO3Voltage(3300);                     // GPS   : 3.3V
    */
    axp.setChgLEDMode(AXP20X_LED_OFF);
    powerStatus();
}

void powerSleep()
{
    Serial.printf("\nSystem has been up for %llu seconds.\n", esp_timer_get_time()/1000000);
    Serial.printf("Going to sleep for %d seconds.\n", SLEEP_SECONDS);
    axp.setPowerOutPut(AXP192_LDO2, AXP202_OFF); // LORA : off
    axp.setPowerOutPut(AXP192_LDO3, AXP202_OFF); // GPS  : off
    axp.setChgLEDMode(AXP20X_LED_OFF);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_38, LOW); // wake up with "user" button (middle)
    esp_sleep_enable_timer_wakeup(1000000 * SLEEP_SECONDS);
    Serial.println("\n### END ###");
    esp_deep_sleep_start();
}


void powerBootInfo()
{
    ++bootCount;
    Serial.printf("Boot number: %d\n", bootCount);

    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_EXT0:
        Serial.println("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        Serial.println("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        Serial.println("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        Serial.println("Wakeup caused by ULP program");
        break;
    default:
        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
        break;
    }
}
