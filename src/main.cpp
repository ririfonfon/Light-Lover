/*
 * Demo code for driving multiple digital RGB(W) strands using esp32_digital_led_lib
 *
 * Modifications Copyright (c) 2017-2019 Martin F. Falatic
 *
 * Based on public domain code created 19 Nov 2016 by Chris Osborn <fozztexx@fozztexx.com>
 * http://insentricity.com
 *
 */

/*
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
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

  artnet.begin();
  // artnet.subscribe_net(0);     // optionally you can change
  // artnet.subscribe_subnet(0);  // optionally you can change

  // if Artnet packet comes to this universe, forward them to fastled directly
  // artnet.forward(universe, leds, NUM_LEDS);

  // this can be achieved manually as follows
  // if Artnet packet comes to this universe, this function (lambda) is called

  // artnet.subscribe(universe, [&](uint8_t *data, uint16_t size) {

  // for (size_t pixel = 0; pixel < 50; ++pixel)
  // {
  //   strand_t *strand[STRANDCNT];
  //   size_t idx = pixel * 4;

  //   strand[0]->pixels[pixel] = pixelFromRGBW(data[idx + 0],data[idx + 1], data[idx + 2], data[idx + 3]);
  // }
  //    }
  //    );
}

//**************************************************************************//
void loop()
{
  strand_t *strands[STRANDCNT];
  for (int i = 0; i < STRANDCNT; i++)
  {
    strands[i] = &STRANDS[i];
  }

  int m1 = getMaxMalloc(1 * 1024, 16 * 1024 * 1024);

  for (int i = STRANDCNT; i > 0; i--)
  {
    randomStrands(strands, i, 10, 1000);
  }

  for (int i = STRANDCNT; i > 0; i--)
  {
    randomStrands(strands, i, 10, 1000);
  }

  for (int i = STRANDCNT; i > 0; i--)
  {
    scanners(strands, i, 10, 1000);
  }

  for (int i = STRANDCNT; i >= 0; i--)
  {
    rainbows(strands, i, 10, 1000);
  }

  int m2 = getMaxMalloc(1 * 1024, 16 * 1024 * 1024);
  assert(m2 >= m1); // Sanity check

  for (int i = 0; i < STRANDCNT; i++)
  {
    strand_t *pStrand = &STRANDS[i];
    rainbow(pStrand, 10, 2000);
    scanner(pStrand, 10, 2000);
  }
  digitalLeds_resetPixels(strands, STRANDCNT);

#if DEBUG_ESP32_DIGITAL_LED_LIB
  dumpDebugBuffer(0, digitalLeds_debugBuffer);
#endif
}

//**************************************************************************//
