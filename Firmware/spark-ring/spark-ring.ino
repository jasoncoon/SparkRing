/*-------------------------------------------------------------------------
  
  Spark Core sketch to control a NeoPixel 24 RGB LED ring: https://www.adafruit.com/product/1586
  Written by Jason Coon
  
  Based on:
  
  Written by Phil Burgess / Paint Your Dragon for Adafruit Industries.
  Modified to work with Spark Core by Technobly.
  Contributions by PJRC and other members of the open source community.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!
  --------------------------------------------------------------------*/

/* ======================= includes ================================= */

#include "application.h"
#include "neopixel/neopixel.h"
#include <math.h>

#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)

#define PIXEL_COUNT 24
#define PIXEL_PIN D2
#define PIXEL_TYPE WS2812B

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

uint8_t brightness = 32;
int patternCount = 10;
int patternIndex = 4;
char patternName[32] = "Rainbow Cycle";
int power = 1;

int timezone = -6;
unsigned long lastTimeSync = millis();

uint16_t pixelIndex;
uint16_t colorIndex;

int setVariable(String args) {
    if(args.startsWith("pwr:")) {
        return setPower(args.substring(4));
    }
    else if (args.startsWith("brt:")) {
        return setBrightness(args.substring(4));
    }
    else if (args.startsWith("tz:")) {
        return setTimezone(args.substring(3));
    }
    
    return -1;
}

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
    if(brightness < 1)
        brightness = 1;
    else if(brightness > 255)
        brightness = 255;
        
    strip.setBrightness(brightness);
    strip.show();
    
    EEPROM.write(0, brightness);
    
    return brightness;
}

int setTimezone(String args) {
    timezone = args.toInt();
    if(timezone < -12)
        power = -12;
    else if (power > 13)
        power = 13;
    
    Time.zone(timezone);
    
    if(timezone < 0)
        EEPROM.write(1, 0);
    else
        EEPROM.write(1, 1);
    
    EEPROM.write(2, abs(timezone));
    
    return power;
}

int setPatternIndex(String args)
{
    patternIndex = args.toInt();
    if(patternIndex < 0)
        patternIndex = 0;
    else if (patternIndex >= patternCount)
        patternIndex = patternCount - 1;
        
    pixelIndex = 0;
    
    EEPROM.write(3, patternIndex);
    
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
        
        case 6:
        strcpy(patternName, "Sparkle White");
        break;
        
        case 7:
        strcpy(patternName, "Sparkle Rainbow");
        break;
        
        case 8:
        strcpy(patternName, "Wave Slow");
        break;
        
        case 9:
        strcpy(patternName, "Analog Clock");
        break;
  }
  
    return pattern;
}

void setup() {
    Serial.begin(9600);
    
    // load settings from EEPROM
    brightness = EEPROM.read(0);
    if(brightness < 1)
        brightness = 1;
    else if(brightness > 255)
        brightness = 255;
    
    uint8_t timezoneSign = EEPROM.read(1);
    if(timezoneSign < 1)
        timezone = -EEPROM.read(2);
    else
        timezone = EEPROM.read(2);
    
    if(timezone < -12)
        power = -12;
    else if (power > 13)
        power = 13;
    
    patternIndex = EEPROM.read(3);
    if(patternIndex < 0)
        patternIndex = 0;
    else if (patternIndex >= patternCount)
        patternIndex = patternCount - 1;
    
    Spark.function("variable", setVariable);
    Spark.function("patternIndex", setPatternIndex);
    Spark.function("patternName", setPatternName);
    
    Spark.variable("power", &power, INT);
    Spark.variable("brightness", &brightness, INT);
    Spark.variable("timezone", &timezone, INT);
    Spark.variable("patternCount", &patternCount, INT);
    Spark.variable("patternIndex", &patternIndex, INT);
    Spark.variable("patternName", patternName, STRING);
    
    Time.zone(timezone);
    
    strip.begin();
    strip.setBrightness(brightness);
    strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  if (millis() - lastTimeSync > ONE_DAY_MILLIS) {
    // Request time synchronization from the Spark Cloud
    Spark.syncTime();
    lastTimeSync = millis();
  }
  
  if(power < 1) {
      colorAll(strip.Color(0, 0, 0)); // Black
      strip.show();
      delay(250);
      return;
  }
  
  uint8_t wait = 30;
  
  switch(patternIndex)
  {
      case 0:
      wait = colorWipe(strip.Color(255, 0, 0)); // Red
      break;
      
      case 1:
      wait = colorWipe(strip.Color(0, 255, 0)); // Green
      break;
      
      case 2:
      wait = colorWipe(strip.Color(0, 0, 255)); // Blue
      break;
      
      case 3:
      wait = rainbowCycle();
      break;
      
      case 4:
      default:
      wait = rainbowSpin();
      break;
      
      case 5:
      wait = colorAll(strip.Color(0, 255, 255)); // Magenta
      break;
      
      case 6:
      wait = sparkle(strip.Color(255, 255, 255));
      break;
      
      case 7:
        colorIndex++;
        if(colorIndex >= 256)
            colorIndex = 0;
        
      wait = sparkle(Wheel(colorIndex));
      break;
      
      case 8:
      wait = wave(.1);
      break;
      
      case 9:
      wait = analogClock();
      break;
  }
  
  strip.show();
  delay(wait);
}

// Set all pixels in the strip to a solid color
uint8_t colorAll(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  
  return 50;
}

// Fill the dots one after the other with a color
uint8_t colorWipe(uint32_t c) {
    strip.setPixelColor(pixelIndex, c);
    
    pixelIndex++;
    if(pixelIndex >= strip.numPixels())
        pixelIndex = 0;
        
    return 50;
}

uint8_t rainbowCycle() {
    for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel((i+colorIndex) & 255));
    }
    
    colorIndex++;
    if(colorIndex >= 256)
        colorIndex = 0;
        
    return 20;
}

// Slightly different, this makes the rainbow equally distributed throughout, then wait (ms)
uint8_t rainbowSpin() {
    for(uint16_t i=0; i< strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + colorIndex) & 255));
    }
    
    colorIndex++;
    if(colorIndex >= 256)
        colorIndex = 0;
    
    return 20;
}

uint8_t sparkle(uint32_t c) {
    colorAll(strip.Color(0, 0, 0)); // Black
    
    strip.setPixelColor(random(0, strip.numPixels()), c);
    
    return 20;
}

float pos = 0.0;

uint8_t wave(float rate) {
    pos = pos + rate;
    for(int i = 0; i < strip.numPixels(); i++) {
        float level = sin(i+pos) * 255;
        strip.setPixelColor(i,(int)level,0,0);
    }
    
    return 20;
}

uint8_t analogClock() {
    colorAll(strip.Color(0, 0, 0)); // Black
    
    int secondIndex = Time.second() * 0.4;
    int minuteIndex = Time.minute() * 0.4;
    int hourIndex = Time.hourFormat12() * 2;
    
    // // draw a fading trail behind the second hand
    // // start at 0 V, at secondIndex + 1
    // // slowly increase V up to 255 at secondIndex
    // int v = 0;
    // for(int i = 0; i < strip.numPixels(); i++) {
    //     int j = secondIndex + 1 + i;
    //     if(j >= strip.numPixels())
    //         j -= strip.numPixels();
        
    //     strip.setPixelColor(j, hsv(240, 255, v));
        
    //     v += 10;
    // }
    
    // hour hand is red
    // minute hand is green
    // second hand is blue
    
    // if all hands are at the same position, draw a white pixel
    if(secondIndex == minuteIndex && minuteIndex == hourIndex) {
        strip.setPixelColor(secondIndex, strip.Color(255, 255, 255));
    }
    // if the second and minute hands are at the same position, draw a cyan pixel
    else if (secondIndex == minuteIndex) {
        strip.setPixelColor(secondIndex, strip.Color(0, 255, 255));
        strip.setPixelColor(hourIndex, strip.Color(255, 0, 0));
    }
    // if the second and hour hands are at the same position, draw a purple pixel
    else if (secondIndex == hourIndex) {
        strip.setPixelColor(secondIndex, strip.Color(255, 0, 255));
        strip.setPixelColor(minuteIndex, strip.Color(0, 255, 0));
    }
    // if the minute and hour hands are at the same position, draw a yellow pixel
    else if (minuteIndex == hourIndex) {
        strip.setPixelColor(hourIndex, strip.Color(255, 255, 0));
        strip.setPixelColor(secondIndex, strip.Color(0, 0, 255));
    }
    // all hands are separate, just draw them
    else {
        strip.setPixelColor(secondIndex, strip.Color(0, 0, 255));
        strip.setPixelColor(minuteIndex, strip.Color(0, 255, 0));
        strip.setPixelColor(hourIndex, strip.Color(255, 0, 0));
    }
        
    return 20;
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

uint32_t dimColor(uint32_t color, uint8_t width) {
   return (((color&0xFF0000)/width)&0xFF0000) + (((color&0x00FF00)/width)&0x00FF00) + (((color&0x0000FF)/width)&0x0000FF);
}

uint32_t hsv(int hue, int sat, int val) {
    int r, g, b, base;
    
    // hue: 0-359, sat: 0-255, val (lightness): 0-255

	if (sat == 0) { // Achromatic color (gray).
		r=val;
		g=val;
		b=val;
	} else  {
		base = ((255 - sat) * val)>>8;
		switch(hue/60) {
			case 0:
				r = val;
				g = (((val-base)*hue)/60)+base;
				b = base;
				break;
			case 1:
				r = (((val-base)*(60-(hue%60)))/60)+base;
				g = val;
				b = base;
				break;
			case 2:
				r = base;
				g = val;
				b = (((val-base)*(hue%60))/60)+base;
				break;
			case 3:
				r = base;
				g = (((val-base)*(60-(hue%60)))/60)+base;
				b = val;
				break;
			case 4:
				r = (((val-base)*(hue%60))/60)+base;
				g = base;
				b = val;
				break;
			case 5:
				r = val;
				g = base;
				b = (((val-base)*(60-(hue%60)))/60)+base;
				break;
		}
	}
	
	return strip.Color(r, g, b);
}