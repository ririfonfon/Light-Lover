// #include <FastLED.h>  // include FastLED *before* Artnet
#include <esp32_digital_led.h>

// Please include ArtnetWiFi.h to use Artnet on the platform
// which can use both WiFi and Ethernet
#include <ArtnetWiFi.h>
// this is also valid for other platforms which can use only WiFi
// #include <Artnet.h>

// WiFi stuff
const char *ssid = "riri_new";
const char *pwd = "B2az41opbn6397";
const IPAddress ip(2, 0, 0, 26);
const IPAddress gateway(2, 0, 0, 1);
const IPAddress subnet(255, 255, 255, 0);

ArtnetWiFiReceiver artnet;
uint8_t universe = 1; // 0 - 15

void setup()
{
    Serial.begin(115200);
    Serial.println("Initializing...");
    dumpSysInfo();
    getMaxMalloc(1 * 1024, 16 * 1024 * 1024);

    /****************************************************************************
       If you have multiple strands connected, but not all are in use, the
       GPIO power-on defaults for the unused strand data lines will typically be
       high-impedance. Unless you are pulling the data lines high or low via a
       resistor, this will lead to noise on those unused but connected channels
       and unwanted driving of those unallocated strands.
       This optional gpioSetup() code helps avoid that problem programmatically.
    ****************************************************************************/
    gpioSetup(15, OUTPUT, LOW);
    gpioSetup(16, OUTPUT, LOW);
    gpioSetup(17, OUTPUT, LOW);
    gpioSetup(18, OUTPUT, LOW);

    // if (digitalLeds_initStrands(STRANDS, STRANDCNT))
    // {
    //     Serial.println("Init FAILURE: halting");
    //     while (true)
    //     {
    //     };
    // }
    for (int i = 0; i < STRANDCNT; i++)
    {
        strand_t *pStrand = &STRANDS[i];
        Serial.print("Strand ");
        Serial.print(i);
        Serial.print(" = ");
        Serial.print((uint32_t)(pStrand->pixels), HEX);
        Serial.println();
#if DEBUG_ESP32_DIGITAL_LED_LIB
        dumpDebugBuffer(-2, digitalLeds_debugBuffer);
#endif
        digitalLeds_resetPixels(pStrand);
#if DEBUG_ESP32_DIGITAL_LED_LIB
        dumpDebugBuffer(-1, digitalLeds_debugBuffer);
#endif
    }
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
    // artnet.subscribe(universe, [&](uint8_t* data, uint16_t size) {
    //     // set led
    //     // artnet data size per packet is 512 max
    //     // so there is max 170 pixel per packet (per universe)
    //     for (size_t pixel = 0; pixel < NUM_LEDS; ++pixel) {
    //         size_t idx = pixel * 3;
    //         leds[pixel].r = data[idx + 0];
    //         leds[pixel].g = data[idx + 1];
    //         leds[pixel].b = data[idx + 2];
    //     }
    // });
}

void loop()
{
    // artnet.parse(); // check if artnet packet has come and execute callback

    strand_t *strands[] = {&STRANDS[0], &STRANDS[1], &STRANDS[2], &STRANDS[3]};

    int m1 = getMaxMalloc(1 * 1024, 16 * 1024 * 1024);

    scanners(strands, 4, 0, 2000);
    scanners(strands, 3, 0, 2000);
    scanners(strands, 2, 0, 2000);
    scanners(strands, 1, 0, 2000);
    scanners(strands, 0, 0, 2000); // NOOP: empty strand array

    rainbows(strands, 4, 0, 4000);
    rainbows(strands, 3, 0, 4000);
    rainbows(strands, 2, 0, 4000);
    rainbows(strands, 1, 0, 4000);
    rainbows(strands, 0, 0, 4000); // NOOP: empty strand array

    int m2 = getMaxMalloc(1 * 1024, 16 * 1024 * 1024);
    assert(m2 >= m1); // Sanity check

    scanner(&STRANDS[2], 1, 2000);
    scanner(&STRANDS[2], 0, 2000);
    scanner(&STRANDS[2], 1, 2000); // A tiny delay can smooth things out
    scanner(&STRANDS[2], 5, 2000);

    for (int i = 0; i < STRANDCNT; i++)
    {
        strand_t *pStrand = &STRANDS[i];
        rainbow(pStrand, 0, 2000);
        scanner(pStrand, 0, 2000);
        digitalLeds_resetPixels(pStrand);
    }

#if DEBUG_ESP32_DIGITAL_LED_LIB
    dumpDebugBuffer(0, digitalLeds_debugBuffer);
#endif
}
