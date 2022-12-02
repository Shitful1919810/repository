#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include <ESPmDNS.h>
#include <deque>
#include <DHTesp.h>
#include "pin.h"
const char ssid[] = "Team WiFi";
const char password[] = "Password";
WiFiServer server(80);
void initWebServer()
{
    WiFi.softAP(ssid, password);
    if (MDNS.begin("esp32"))
    {
        Serial.println("MDNS responder started");
    }
    server.begin();
    Serial.println("IP:");
    Serial.println(WiFi.softAPIP());
}
String readTxt = "";
extern const char *responseHeaders;
extern const char *responseHeadersPlain;
extern const char *html;
void handleWeb()
{
    WiFiClient client = server.available();
    if (client)
    {
        bool blankRequest = true;
        Serial.println("[Client connected]");
        while (client.connected())
        {

            if (client.available())
            {
                char c = client.read();
                readTxt += c;
                if (c == '\n' && blankRequest)
                {
                    if (readTxt.startsWith("GET / HTTP/1.1"))
                    {
                        client.print(responseHeaders);
                        client.print(html);
                        client.print("\r\n");
                    }
                    else if (readTxt.startsWith("GET "))
                    {
                        unsigned int firstSpace = 0, secondSpace = 0;
                        while (firstSpace < readTxt.length() && readTxt[firstSpace] != '/')
                            ++firstSpace;
                        secondSpace = firstSpace + 1;
                        while (secondSpace < readTxt.length() && readTxt[secondSpace] != ' ')
                            ++secondSpace;
                        String quest = readTxt.substring(firstSpace, secondSpace);
                        Serial.println("quest is : ");
                        Serial.println(quest);
                        if (quest == "/switch")
                        {
                            extern bool useOLED;
                            useOLED = !useOLED;
                            client.print(String("OLED is now ") + (useOLED ? "ON" : "OFF"));
                        }
                        else if (quest == "/clr")
                        {
                            extern std::deque<std::pair<TempAndHumidity, ticks>> dataList;
                            while (!dataList.empty())
                                dataList.pop_front();
                            client.print("DHT11 data is removed");
                            extern bool requestOLEDRefresh;
                            requestOLEDRefresh = true;
                        }
                        else if (quest.startsWith("/itv"))
                        {
                            int value = quest.substring(4, quest.length()).toInt();
                            if (value < 1000 || value > 60000 * 60 * 24)
                            {
                                client.print("Interval out of range");
                            }
                            else
                            {
                                extern ticks dhtReadInterval;
                                dhtReadInterval = value;
                                client.print(String("DHT11 Interval is set to ") + dhtReadInterval);
                            }
                        }
                        else if (quest.startsWith("/nda"))
                        {
                            int value = quest.substring(4, quest.length()).toInt();
                            if (value < 5 || value > 1000)
                            {
                                client.print("nData out of range");
                            }
                            else
                            {
                                extern std::deque<std::pair<TempAndHumidity, ticks>> dataList;
                                extern bool requestOLEDRefresh;
                                requestOLEDRefresh = true;
                                extern int nMaxData;
                                nMaxData = value;
                                client.print(String("nData is set to ") + nMaxData);
                                while (dataList.size() > nMaxData)
                                    dataList.pop_front();
                            }
                        }
                        else if (quest.startsWith("/led"))
                        {
                            int value = quest.substring(4, quest.length()).toInt();
                            if (value < 500 || value > 10000)
                            {
                                client.print("RedLEDInterval out of range");
                            }
                            else
                            {
                                extern int redLEDInterval;
                                redLEDInterval = value;
                                client.print(String("RedLEDInterval is set to ") + redLEDInterval);
                            }
                        }
                        else if (quest == "/download")
                        {
                            extern std::deque<std::pair<TempAndHumidity, ticks>> dataList;
                            String json = "{\n  \"size\" : ";
                            json += dataList.size();
                            json += ",\n  \"DHT11Data\" : [\n";
                            int cur = 0;
                            for (const auto &data : dataList)
                            {
                                ++cur;
                                String item = "    [";
                                item += data.first.humidity;
                                item += ", ";
                                item += data.first.temperature;
                                item += ", ";
                                item += data.second;
                                if (cur == dataList.size())
                                    item += "]\n";
                                else
                                    item += "],\n";
                                json += item;
                            }
                            json += "  ]\n}";
                            client.print(json);
                        }
                        else
                        {
                            client.print("bad request");
                        }
                    }
                    else
                    {
                        client.print("\r\n");
                    }
                    break;
                }
                if (c == '\n')
                {
                    blankRequest = true;
                }
                else if (c != '\r')
                {
                    blankRequest = false;
                }
            }
        }
        delay(1);
        client.stop();
        Serial.println("[Client disconnected]");

        Serial.println(readTxt);
        readTxt = "";
    }
}
