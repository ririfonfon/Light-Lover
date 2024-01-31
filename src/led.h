#include "Arduino.h"

#if defined(ARDUINO) && ARDUINO >= 100
// No extras
#elif defined(ARDUINO) // pre-1.0
// No extras
#elif defined(ESP_PLATFORM)
#include "arduinoish.hpp"
#endif

#define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

// **Required** if debugging is enabled in library header
// TODO: Is there any way to put this in digitalLeds_addStrands() and avoid undefined refs?
#if DEBUG_ESP32_DIGITAL_LED_LIB
int digitalLeds_debugBufferSz = 1024;
char *digitalLeds_debugBuffer = static_cast<char *>(calloc(digitalLeds_debugBufferSz, sizeof(char)));
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers" // It's noisy here with `-Wall`

const int STRANDCNT = 5;

strand_t STRANDS[STRANDCNT] = {
    // Avoid using any of the strapping pins on the ESP32, anything >=32, 16, 17... not much left.
    // {.rmtChannel = 0, .gpioNum = 15, .ledType = LED_WS2814A_V1, .brightLimit = 254, .numPixels = 15},
    // {.rmtChannel = 1, .gpioNum =  4, .ledType = LED_WS2814A_V1, .brightLimit = 254, .numPixels = 15},
    // {.rmtChannel = 2, .gpioNum = 16, .ledType = LED_WS2814A_V1, .brightLimit = 254, .numPixels = 15},
    // {.rmtChannel = 3, .gpioNum = 17, .ledType = LED_WS2814A_V1, .brightLimit = 254, .numPixels = 15},
    // {.rmtChannel = 4, .gpioNum = 18, .ledType = LED_WS2814A_V1, .brightLimit = 254, .numPixels = 15},
    {.rmtChannel = 0, .gpioNum = 15, .ledType = LED_WS2814A_V1, .numPixels = 15},
    {.rmtChannel = 1, .gpioNum =  4, .ledType = LED_WS2814A_V1, .numPixels = 15},
    {.rmtChannel = 2, .gpioNum = 16, .ledType = LED_WS2814A_V1, .numPixels = 15},
    {.rmtChannel = 3, .gpioNum = 17, .ledType = LED_WS2814A_V1, .numPixels = 15},
    {.rmtChannel = 4, .gpioNum = 18, .ledType = LED_WS2814A_V1, .numPixels = 15},
};
//  {.rmtChannel = 0, .gpioNum = 14, .ledType = LED_SK6812W_V1, .brightLimit = 24, .numPixels =  144},
//  {.rmtChannel = 0, .gpioNum = 15, .ledType = LED_SK6812W_V1, .brightLimit = 24, .numPixels =  50},
//  {.rmtChannel = 1, .gpioNum = 15, .ledType = LED_WS2812B_V3, .brightLimit = 24, .numPixels =  93},
//  {.rmtChannel = 2, .gpioNum = 26, .ledType = LED_WS2812B_V3, .brightLimit = 32, .numPixels =  93},
//  {.rmtChannel = 3, .gpioNum = 27, .ledType = LED_WS2812B_V3, .brightLimit = 32, .numPixels =  93},
//  {.rmtChannel = 3, .gpioNum = 19, .ledType = LED_WS2812B_V3, .brightLimit = 32, .numPixels =  64},
//  {.rmtChannel = 0, .gpioNum = 16, .ledType = LED_WS2812B_V3, .brightLimit = 32, .numPixels = 256},
//  {.rmtChannel = 0, .gpioNum = 16, .ledType = LED_SK6812W_V1, .brightLimit = 32, .numPixels = 300},
//  {.rmtChannel = 0, .gpioNum = 16, .ledType = LED_WS2813_V2,  .brightLimit = 32, .numPixels = 300},

// strand_t STRAND0 = {.rmtChannel = 1, .gpioNum = 14, .ledType = LED_WS2812B_V3, .brightLimit = 24, .numPixels =  93,
//    .pixels = nullptr, ._stateVars = nullptr};

#pragma GCC diagnostic pop

//**************************************************************************//
int getMaxMalloc(int min_mem, int max_mem)
{
  int prev_size = min_mem;
  int curr_size = min_mem;
  int max_free = 0;
  //  Serial.print("checkmem: testing alloc from ");
  //  Serial.print(min_mem);
  //  Serial.print(" : ");
  //  Serial.print(max_mem);
  //  Serial.println(" bytes");
  while (1)
  {
    void *foo1 = malloc(curr_size);
    //    Serial.print("checkmem: attempt alloc of ");
    //    Serial.print(curr_size);
    //    Serial.print(" bytes --> pointer 0x");
    //    Serial.println((uintptr_t)foo1, HEX);
    if (foo1 == nullptr)
    { // Back off
      max_mem = min(curr_size, max_mem);
      //      Serial.print("checkmem: backoff 2 prev = ");
      //      Serial.print(prev_size);
      //      Serial.print(", curr = ");
      //      Serial.print(curr_size);
      //      Serial.print(", max_mem = ");
      //      Serial.print(max_mem);
      //      Serial.println();
      curr_size = (int)(curr_size - (curr_size - prev_size) / 2.0);
      //      Serial.print("checkmem: backoff 2 prev = ");
      //      Serial.print(prev_size);
      //      Serial.print(", curr = ");
      //      Serial.print(curr_size);
      //      Serial.println();
    }
    else
    { // Advance
      free(foo1);
      max_free = curr_size;
      prev_size = curr_size;
      curr_size = min(curr_size * 2, max_mem);
      //      Serial.print("checkmem: advance 2 prev = ");
      //      Serial.print(prev_size);
      //      Serial.print(", curr = ");
      //      Serial.print(curr_size);
      //      Serial.println();
    }
    if (abs(curr_size - prev_size) == 0)
    {
      break;
    }
  }
  Serial.print("checkmem: max free heap = ");
  Serial.print(esp_get_free_heap_size());
  Serial.print(" bytes, max allocable = ");
  Serial.print(max_free);
  Serial.println(" bytes");
  return max_free;
}

void dumpSysInfo()
{
  esp_chip_info_t sysinfo;
  esp_chip_info(&sysinfo);
  Serial.print("Model: ");
  Serial.print((int)sysinfo.model);
  Serial.print("; Features: 0x");
  Serial.print((int)sysinfo.features, HEX);
  Serial.print("; Cores: ");
  Serial.print((int)sysinfo.cores);
  Serial.print("; Revision: r");
  Serial.println((int)sysinfo.revision);
}

void dumpDebugBuffer(int id, char *debugBuffer)
{
  Serial.print("DEBUG: (");
  Serial.print(id);
  Serial.print(") ");
  Serial.println(debugBuffer);
  debugBuffer[0] = 0;
}

//**************************************************************************//
boolean initStrands()
{
  /****************************************************************************
     If you have multiple strands connected, but not all are in use, the
     GPIO power-on defaults for the unused strand data lines will typically be
     high-impedance. Unless you are pulling the data lines high or low via a
     resistor, this will lead to noise on those unused but connected channels
     and unwanted driving of those unallocated strands.
     This optional gpioSetup() code helps avoid that problem programmatically.
  ****************************************************************************/

  digitalLeds_initDriver();

  for (int i = 0; i < STRANDCNT; i++)
  {
    gpioSetup(STRANDS[i].gpioNum, OUTPUT, LOW);
  }

  strand_t *strands[8];
  for (int i = 0; i < STRANDCNT; i++)
  {
    strands[i] = &STRANDS[i];
  }
  int rc = digitalLeds_addStrands(strands, STRANDCNT);
  if (rc)
  {
    Serial.print("Init rc = ");
    Serial.println(rc);
    return false;
  }

  for (int i = 0; i < STRANDCNT; i++)
  {
    strand_t *pStrand = strands[i];
    Serial.print("Strand ");
    Serial.print(i);
    Serial.print(" = ");
    Serial.print((uint32_t)(pStrand->pixels), HEX);
    Serial.println();
#if DEBUG_ESP32_DIGITAL_LED_LIB
    dumpDebugBuffer(-2, digitalLeds_debugBuffer);
#endif
  }

  return true;
}

// Hacky debugging method
// espPinMode((gpio_num_t)5, OUTPUT);
// gpio_set_level((gpio_num_t)5, 0);
// gpio_set_level((gpio_num_t)5, 1);  gpio_set_level((gpio_num_t)5, 0);

//**************************************************************************//

struct Led
{
  // state variables
  uint8_t pin;
  bool on;

  // methods
  void update()
  {
    digitalWrite(pin, on ? HIGH : LOW);
  }
};

Led onboard_led = {2, false};

void init_led()
{
  pinMode(onboard_led.pin, OUTPUT);
} // init_led