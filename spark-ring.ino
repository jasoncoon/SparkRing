/*-------------------------------------------------------------------------
  
  Spark Core sketch to control a NeoPixel 24 RGB LED ring: https://www.adafruit.com/product/1586
  Written by Jason Coon, based on:
  
  Spark Core library to control WS2811/WS2812 based RGB
  LED devices such as Adafruit NeoPixel strips.
  Currently handles 800 KHz and 400kHz bitstream on Spark Core, 
  WS2812, WS2812B and WS2811.

  Also supports:
  - Radio Shack Tri-Color Strip with TM1803 controller 400kHz bitstream.
  - TM1829 pixels
  
  PLEASE NOTE that the NeoPixels require 5V level inputs 
  and the Spark Core only has 3.3V level outputs. Level shifting is
  necessary, but will require a fast device such as one of the following:

  [SN74HCT125N]
  http://www.digikey.com/product-detail/en/SN74HCT125N/296-8386-5-ND/376860

  [SN74HCT245N] 
  http://www.digikey.com/product-detail/en/SN74HCT245N/296-1612-5-ND/277258

  Written by Phil Burgess / Paint Your Dragon for Adafruit Industries.
  Modified to work with Spark Core by Technobly.
  Contributions by PJRC and other members of the open source community.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!
  --------------------------------------------------------------------*/

/* ======================= includes ================================= */

#include "application.h"
//#include "spark_disable_wlan.h" (for faster local debugging only)
#include "neopixel/neopixel.h"

/* ======================= prototypes =============================== */

void colorAll(uint32_t c, uint8_t wait);
void colorWipe(uint32_t c, uint8_t wait);
void rainbow(uint8_t wait);
void rainbowCycle(uint8_t wait);
uint32_t Wheel(byte WheelPos);

/* ======================= extra-examples.cpp ======================== */

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 24
#define PIXEL_PIN D2
#define PIXEL_TYPE WS2812B

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
//               note: if not specified, D2 is selected for you.
// Parameter 3 = pixel type [ WS2812, WS2812B, WS2811, TM1803 ]
//               note: if not specified, WS2812B is selected for you.
//               note: RGB order is automatically applied to WS2811,
//                     WS2812/WS2812B/TM1803 is GRB order.
//
// 800 KHz bitstream 800 KHz bitstream (most NeoPixel products ...
//                         ... WS2812 (6-pin part)/WS2812B (4-pin part) )
//
// 400 KHz bitstream (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//                   (Radio Shack Tri-Color LED Strip - TM1803 driver
//                    NOTE: RS Tri-Color LED's are grouped in sets of 3)

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

int brightness = 64;
int patternCount = 6;
int patternIndex = 4;
char patternName[32] = "Rainbow Cycle";
int power = 1;

int setPower(String args) {
    power = args.toInt();
    if(power < 0)
        power = 0;
    else if (power > 1)
        power = 1;
    
    return power;
}

int setBrightness(String args)
{
    brightness = args.toInt();
    if(brightness < 0)
        brightness = 0;
    else if(brightness > 255)
        brightness = 255;
        
    strip.setBrightness(brightness);
    strip.show();
    return brightness;
}

int setPatternIndex(String args)
{
    patternIndex = args.toInt();
    if(patternIndex < 0)
        patternIndex = 0;
    else if (patternIndex >= patternCount)
        patternIndex = patternCount - 1;
        
    return patternIndex;
}

int setPatternName(String args)
{
    int pattern = args.toInt();
    if(pattern < 0)
        pattern = 0;
    else if (pattern >= patternCount)
        pattern = patternCount - 1;
    
    switch(pattern)
    {
        case 0:
        strcpy(patternName, "Color Wipe Red");
        break;
        
        case 1:
        strcpy(patternName, "Color Wipe Green");
        break;
      
        case 2:
        strcpy(patternName, "Color Wipe Blue");
        break;
        
        case 3:
        strcpy(patternName, "Rainbow Cycle");
        break;
        
        case 4:
        default:
        strcpy(patternName, "Rainbow Spin");
        break;
        
        case 5:
        strcpy(patternName, "Color All Magenta");
        break;
  }
  
    return pattern;
}

void setup() {
    Spark.function("power", setPower);
    Spark.function("brightness", setBrightness);
    Spark.function("patternIndex", setPatternIndex);
    Spark.function("patternName", setPatternName);
    
    Spark.variable("power", &power, INT);
    Spark.variable("brightness", &brightness, INT);
    Spark.variable("patternCount", &patternCount, INT);
    Spark.variable("patternIndex", &patternIndex, INT);
    Spark.variable("patternName", patternName, STRING);
    
    strip.begin();
    strip.setBrightness(64);
    strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  // Some example procedures showing how to display to the pixels:
  // Do not run more than one of these at a time, or the b/g tasks 
  // will be blocked.
  //--------------------------------------------------------------
  
  //strip.setPixelColor(0, strip.Color(255, 0, 255));
  //strip.show();
  
  if(power < 1) {
      colorAll(strip.Color(0, 0, 0), 0); // Black
      delay(250);
      return;
  }
  
  switch(patternIndex)
  {
      case 0:
      colorWipe(strip.Color(255, 0, 0), 50); // Red
      break;
      
      case 1:
      colorWipe(strip.Color(0, 255, 0), 50); // Green
      break;
      
      case 2:
      colorWipe(strip.Color(0, 0, 255), 50); // Blue
      break;
      
      case 3:
      rainbowCycle(20);
      break;
      
      case 4:
      default:
      rainbowSpin(20);
      break;
      
      case 5:
      colorAll(strip.Color(0, 255, 255), 50); // Magenta
      break;
  }
}

// Set all pixels in the strip to a solid color, then wait (ms)
void colorAll(uint32_t c, uint8_t wait) {
  uint16_t i;
  
  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
  delay(wait);
}

// Fill the dots one after the other with a color, wait (ms) after each one
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout, then wait (ms)
void rainbowSpin(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) { // 1 cycle of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}