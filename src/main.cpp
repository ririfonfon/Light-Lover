#include <Arduino.h>

#include "esp32_digital_led_lib.h"
#include "esp32_digital_led_funcs.h"
#include "led.h"
#include <ArtnetWiFi.h>

#define ID 1

// WiFi stuff
// const char *ssid = "riri_new";
// const char *pwd = "B2az41opbn6397";
const char *ssid = "kxkm24";
const char *pwd = "";
const IPAddress ip(2, 0, 0, 200 + ID);
const IPAddress gateway(2, 0, 0, 1);
const IPAddress subnet(255, 255, 255, 0);

uint8_t universe;

uint16_t ch_start;
uint8_t Modulo;

ArtnetWiFiReceiver artnet;

strand_t *strands[STRANDCNT];

void onArtnet(const uint8_t *data, const uint16_t length)
{
  for (uint8_t str = 0; str < STRANDCNT; str++)
  {
    size_t idx;
    for (size_t pixel = 0; pixel < STRANDS[str].numPixels; pixel++)
    {
      // size_t idx = pixel * 4;
      if (pixel < 5)
      {
        idx = ch_start + 12 * str - 1;
      }
      else if (pixel > 4 && pixel < 10)
      {
        idx = ch_start + 4 + 12 * str - 1;
      }
      else if (pixel > 9)
      {
        idx = ch_start + 8 + 12 * str - 1;
      }
      if (idx + 3 < length)
      {
        strands[str]->pixels[pixel] = pixelFromRGBW(data[idx + 0], data[idx + 1], data[idx + 2], data[idx + 3]);
      }
    }
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

  // init led
  init_led();

  if (!initStrands())
  {
    Serial.println("Init FAILURE: halting");
    while (true)
    {
      delay(100);
    }
  }

  Modulo = ID % 8;
  if(Modulo == 0)
  {
    Modulo = 8;
  }
  ch_start = Modulo * 60 - 59;
  Serial.print("ch_start = ");
  Serial.println(ch_start);

  if (ID < 9)
  {
    universe = 0; // 1 - 8
  }
  else if (ID > 8 && ID < 17)
  {
    universe = 1; // 9 - 16
  }
  else if (ID > 16 && ID < 25)
  {
    universe = 2; // 17 - 24
  }
  else if (ID > 24 && ID < 33)
  {
    universe = 3; // 25 - 32
  }
  else if (ID > 32 && ID < 41)
  {
    universe = 4; // 33 - 40
  }
  Serial.print("universe = ");
  Serial.println(universe);

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

  String NAME = "L_LOVER " + String(ID);
  artnet.shortname(NAME);
  artnet.longname(NAME);

  artnet.begin();
  artnet.subscribe(universe, onArtnet);
}

void loop()
{
  artnet.parse();
  onboard_led.on = millis() % 2000 < 1000;
  onboard_led.update();
  delay(10); // est ce vraiment utile ???
}
