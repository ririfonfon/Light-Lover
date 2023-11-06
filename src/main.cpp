#include <Arduino.h>

#include "esp32_digital_led_lib.h"
#include "esp32_digital_led_funcs.h"
#include "led.h"
#include <ArtnetWiFi.h>

// WiFi stuff
const char *ssid = "riri_new";
const char *pwd = "B2az41opbn6397";
const IPAddress ip(2, 0, 0, 201);
const IPAddress gateway(2, 0, 0, 1);
const IPAddress subnet(255, 255, 255, 0);

ArtnetWiFiReceiver artnet;
uint8_t universe = 0; // 0 - 15

strand_t *strands[STRANDCNT];

void onArtnet(const uint8_t *data, const uint16_t length)
{
  for (size_t pixel = 0; pixel < STRANDS[0].numPixels; pixel++)
  {
    size_t idx = pixel * 4;
    if (idx + 3 < length)
      strands[0]->pixels[pixel] = pixelFromRGBW(data[idx + 0], data[idx + 1], data[idx + 2], data[idx + 3]);
  }
  digitalLeds_drawPixels(strands, STRANDCNT);
  // Serial.printf("Artnet recv %d\n", length);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");
  dumpSysInfo();
  getMaxMalloc(1 * 1024, 16 * 1024 * 1024);

  if (!initStrands())
  {
    Serial.println("Init FAILURE: halting");
    while (true)
    {
      delay(100);
    }
  }
  for (int i = 0; i < STRANDCNT; i++)
    strands[i] = &STRANDS[i];
  digitalLeds_resetPixels(strands, STRANDCNT);
  delay(100);
  Serial.println("Init complete");

  // WiFi stuff
  WiFi.begin(ssid, pwd);
  WiFi.config(ip, gateway, subnet);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("WiFi connected, IP = ");
  Serial.println(WiFi.localIP());

  artnet.shortname("Super");
  artnet.longname("Super Genial");

  artnet.begin();
  artnet.subscribe(universe, onArtnet);
}

void loop()
{
  artnet.parse();
}

