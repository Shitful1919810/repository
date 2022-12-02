#include <deque>
#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include <ESPmDNS.h>
#include <DHTesp.h>
#include <U8g2lib.h>
#include "pin.h"
extern WiFiServer server;
void initWebServer();
void handleWeb();

DHTesp dht11;
U8G2_SSD1306_128X64_NONAME_F_SW_I2C oled(U8G2_R0, OLED_SCL, OLED_SDA, U8X8_PIN_NONE);
ticks dhtTick;
ticks oledTick;
ticks redLEDTick;
ticks dhtReadInterval = 5000;
ticks redLEDInterval = 3000;
int nMaxData = 100;
std::deque<std::pair<TempAndHumidity, ticks>> dataList;
bool requestOLEDRefresh = true;
bool useOLED = true;
bool lastUseOLED = true;
auto timerKillBlueLED = timerBegin(0, 8000, true);
auto timerKillRedLED = timerBegin(1, 8000, true);

void refreshOLEDBuf();
void setup()
{

  Serial.begin(115200);
  // pinMode(2, OUTPUT);
  // digitalWrite(2, LOW);
  pinMode(BlueLED, OUTPUT);
  digitalWrite(BlueLED, HIGH);
  pinMode(RedLED, OUTPUT);
  digitalWrite(RedLED, HIGH);
  initWebServer();
#ifndef DISABLE_DHT11
  dht11.setup(DHTPin, DHTesp::DHT11);
#endif
#ifndef DISABLE_OLED
  oled.begin();
  oled.setFont(u8g2_font_ncenB10_tr);
#endif
  dhtTick = oledTick = redLEDTick = millis();

  timerAttachInterrupt(
      timerKillBlueLED, []()
      { digitalWrite(BlueLED, HIGH);
    timerAlarmDisable(timerKillBlueLED); },
      true);
  timerAlarmWrite(timerKillBlueLED, 500, false);
  timerAttachInterrupt(
      timerKillRedLED, []()
      { digitalWrite(RedLED, HIGH);
    timerAlarmDisable(timerKillRedLED); },
      true);
  timerAlarmWrite(timerKillRedLED, 500, false);
}

void loop()
{
  handleWeb();

  if (!useOLED && lastUseOLED)
  {
    oled.clear();
    oled.setPowerSave(true);
    lastUseOLED = false;
  }
  if (useOLED && !lastUseOLED)
  {
    oled.setPowerSave(false);
    lastUseOLED = true;
    refreshOLEDBuf();
  }

#ifndef DISABLE_DHT11
  if (millis() - dhtTick > dhtReadInterval)
  {
    dhtTick = millis();
    TempAndHumidity data = dht11.getTempAndHumidity();
    if (dataList.size() >= nMaxData)
      dataList.pop_front();
    dataList.push_back({data, dhtTick});
    Serial.println(dataList.size());
    digitalWrite(BlueLED, LOW);
    timerRestart(timerKillBlueLED);
    timerAlarmEnable(timerKillBlueLED);
#ifndef DISABLE_OLED
    if (useOLED)
      refreshOLEDBuf();
#endif
  }
#endif

  if (millis() - redLEDTick >= redLEDInterval)
  {
    redLEDTick = millis();
    if (!useOLED)
    {
      digitalWrite(RedLED, LOW);
      timerRestart(timerKillRedLED);
      timerAlarmEnable(timerKillRedLED);
    }
  }
  if (useOLED && requestOLEDRefresh)
    refreshOLEDBuf();
}
void refreshOLEDBuf()
{
  requestOLEDRefresh = false;
  oled.clearBuffer();
  if (!dataList.empty())
  {
    oled.drawStr(0, 16, "Tem:");
    oled.drawStr(0 + 4 * 10, 16, String(dataList.back().first.temperature).c_str());
    oled.drawStr(0 + 4 * 10, 16, String(dataList.back().first.temperature).c_str());
    oled.drawStr(0 + 8 * 10, 16, " Deg");
    oled.drawStr(0, 32, "Hum:");
    oled.drawStr(0 + 4 * 10, 32, String(dataList.back().first.humidity).c_str());
    oled.drawStr(0 + 8 * 10, 32, " %RH");
  }
  oled.drawStr(0, 48, "Items:");
  oled.drawStr(0 + 5 * 10, 48, String(dataList.size()).c_str());
  oled.sendBuffer();
}
