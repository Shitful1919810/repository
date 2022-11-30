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
std::deque<TempAndHumidity> dataList;
bool requestOLEDRefresh = true;
bool useOLED = true;
bool lastUseOLED = true;
auto timerKillBlueLED = timerBegin(0, 8000, true);
auto timerKillRedLED = timerBegin(1, 8000, true);

void refreshOLED();
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
    refreshOLED();
  }

#ifndef DISABLE_DHT11
  if (millis() - dhtTick > dhtReadInterval)
  {
    dhtTick = millis();
    TempAndHumidity data = dht11.getTempAndHumidity();
    if (dataList.size() >= nMaxData)
      dataList.pop_front();
    dataList.push_back(data);
    Serial.println(dataList.size());
    digitalWrite(BlueLED, LOW);
    timerRestart(timerKillBlueLED);
    timerAlarmEnable(timerKillBlueLED);
#ifndef DISABLE_OLED
    if (useOLED)
      refreshOLED();
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
    refreshOLED();
  delay(20);
}

void refreshOLED()
{
  requestOLEDRefresh = false;
  oled.firstPage();
  do
  {
    if (!dataList.empty())
    {
      oled.setCursor(16, 16);
      oled.print("Tem:");
      oled.print(dataList.back().temperature);
      oled.print(" Deg");
      oled.setCursor(16, 32);
      oled.print("Hum:");
      oled.print(dataList.back().humidity);
      oled.print("%RH");
    }
    oled.setCursor(16, 48);
    oled.print("Items: ");
    oled.print(dataList.size());
  } while (oled.nextPage());
}