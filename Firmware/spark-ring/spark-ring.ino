#include "FastLED/FastLED.h"
FASTLED_USING_NAMESPACE;

#include "application.h"
#include <math.h>

#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)

#define LED_PIN     1
#define CLOCK_PIN   0
#define COLOR_ORDER GBR // 1 = GBR; 2 = BGR;
#define CHIPSET     NEOPIXEL
#define NUM_LEDS    24
CRGB leds[NUM_LEDS];

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
 
// List of patterns to cycle through.  Each is defined as a separate function below.

typedef uint8_t (*SimplePattern)();
typedef SimplePattern SimplePatternList[];
typedef struct { SimplePattern drawFrame;  char name[32]; } PatternAndName;
typedef PatternAndName PatternAndNameList[];
 
// These times are in seconds, but could be changed to milliseconds if desired;
// there's some discussion further below.
 
const PatternAndNameList patterns = { 
  { rainbow,            "Rainbow" },
  { rainbowWithGlitter, "Rainbow With Glitter" },
  { confetti,           "Confetti" },
  { sinelon,            "Sinelon" },
  { bpm,                "Beat" },
  { juggle,             "Juggle" },
  { fire,               "Fire" },
  { water,              "Water" },
  { analogClock,        "Analog Clock" },
  { fastAnalogClock,    "Fast Analog Clock Test" },
  { showSolidColor,     "Solid Color" }
};

uint8_t brightness = 32;

int patternCount = ARRAY_SIZE(patterns);
int patternIndex = 0;
char patternName[32] = "Rainbow";
int power = 1;
int flipClock = 0;

int timezone = -6;
unsigned long lastTimeSync = millis();

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

CRGBPalette16 palette = RainbowColors_p;
  
CRGB solidColor = CRGB::Blue;
int r = 0;
int g = 0;
int b = 255;

SYSTEM_MODE(SEMI_AUTOMATIC);

int offlinePin = D7;

void setup() {
    FastLED.addLeds<CHIPSET, LED_PIN>(leds, NUM_LEDS);
    // FastLED.setCorrection(CRGB( 160, 255, 255));
    FastLED.setBrightness(brightness);
    fill_solid(leds, NUM_LEDS, CRGB::Blue);
    FastLED.show();
    
    pinMode(offlinePin, INPUT_PULLUP);
    
    if(digitalRead(offlinePin) == HIGH) {
        Spark.connect();
    }
    
    // Serial.begin(9600);
    // load settings from EEPROM
    brightness = EEPROM.read(0);
    if(brightness < 1)
        brightness = 1;
    else if(brightness > 255)
        brightness = 255;
    
    FastLED.setBrightness(brightness);
    
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
    
    flipClock = EEPROM.read(4);
    if(flipClock < 0)
        flipClock = 0;
    else if (flipClock > 1)
        flipClock = 1;
        
    r = EEPROM.read(5);
    g = EEPROM.read(6);
    b = EEPROM.read(7);
    
    if(r == 0 && g == 0 && b == 0) {
        r = 0;
        g = 0;
        b = 255;
    }
    
    solidColor = CRGB(r, b, g);
    
    Spark.function("variable", setVariable);
    Spark.function("patternIndex", setPatternIndex);
    Spark.function("patternName", setPatternName);
    
    Spark.variable("power", &power, INT);
    Spark.variable("brightness", &brightness, INT);
    Spark.variable("timezone", &timezone, INT);
    Spark.variable("flipClock", &flipClock, INT);
    Spark.variable("patternCount", &patternCount, INT);
    Spark.variable("patternIndex", &patternIndex, INT);
    Spark.variable("patternName", patternName, STRING);
    Spark.variable("r", &r, INT);
    Spark.variable("g", &g, INT);
    Spark.variable("b", &b, INT);
    
    Time.zone(timezone);
}

void loop() {
  if (Spark.connected() && millis() - lastTimeSync > ONE_DAY_MILLIS) {
    // Request time synchronization from the Spark Cloud
    Spark.syncTime();
    lastTimeSync = millis();
  }
  
  if(power < 1) {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
      FastLED.delay(250);
      return;
  }
  
  uint8_t delay = patterns[patternIndex].drawFrame();
  
  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(delay); 
  
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}

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
    else if (args.startsWith("flpclk:")) {
        return setFlipClock(args.substring(7));
    }
    else if (args.startsWith("r:")) {
        r = parseByte(args.substring(2));
        solidColor.r = r;
        EEPROM.write(5, r);
        patternIndex = patternCount - 1;
        return r;
    }
    else if (args.startsWith("g:")) {
        g = parseByte(args.substring(2));
        solidColor.g = g;
        EEPROM.write(6, g);
        patternIndex = patternCount - 1;
        return g;
    }
    else if (args.startsWith("b:")) {
        b = parseByte(args.substring(2));
        solidColor.b = b;
        EEPROM.write(7, b);
        patternIndex = patternCount - 1;
        return b;
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
        
    FastLED.setBrightness(brightness);
    
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

int setFlipClock(String args) {
    flipClock = args.toInt();
    if(flipClock < 0)
        flipClock = 0;
    else if (flipClock > 1)
        flipClock = 1;
    
    EEPROM.write(4, flipClock);
    
    return flipClock;
}

byte parseByte(String args) {
    int c = args.toInt();
    if(c < 0)
        c = 0;
    else if (c > 255)
        c = 255;
    
    return c;
}

int setPatternIndex(String args)
{
    patternIndex = args.toInt();
    if(patternIndex < 0)
        patternIndex = 0;
    else if (patternIndex >= patternCount)
        patternIndex = patternCount - 1;
        
    EEPROM.write(3, patternIndex);
    
    return patternIndex;
}

int setPatternName(String args)
{
    int index = args.toInt();
    if(index < 0)
        index = 0;
    else if (index >= patternCount)
        index = patternCount - 1;
    
    strcpy(patternName, patterns[index].name);
    
    return index;
}

uint8_t rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 255 / NUM_LEDS);
  return 8;
}

uint8_t rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
  return 8;
}

uint8_t confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
  return 8;
}

uint8_t sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
  return 8;
}

uint8_t bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
  
  return 8;
}

uint8_t juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
  
  return 8;
}

uint8_t fire() {
    heatMap(HeatColors_p, true);
    
    return 30;
}

CRGBPalette16 icePalette = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);

uint8_t water() {
    heatMap(icePalette, false);
    
    return 45;
}

uint8_t analogClock() {
    dimAll(220);
    
    drawAnalogClock(Time.second(), Time.minute(), Time.hourFormat12(), true, true);
    
    return 8;
}

byte fastSecond = 0;
byte fastMinute = 0;
byte fastHour = 1;

uint8_t fastAnalogClock() {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    
    drawAnalogClock(fastSecond, fastMinute, fastHour, false, false);
        
    fastMinute++;
    
    // fastSecond++;
    
    // if(fastSecond >= 60) {
    //     fastSecond = 0;
    //     fastMinute++;
    // }
     
    if(fastMinute >= 60) {
        fastMinute = 0;
        fastHour++;
    }
    
    if(fastHour >= 13) {
        fastHour = 1;
    }
        
    return 125;
}

uint8_t showSolidColor() {
    fill_solid(leds, NUM_LEDS, solidColor);
    
    return 30;
}

void heatMap(CRGBPalette16 palette, bool up) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    
    // Add entropy to random number generator; we use a lot of it.
    random16_add_entropy(random(256));
    
    uint8_t cooling = 55;
    uint8_t sparking = 120;
    
    // Array of temperature readings at each simulation cell
    static const uint8_t halfLedCount = NUM_LEDS / 2;
    static byte heat[2][halfLedCount];
    
    byte colorindex;
    
    for(uint8_t x = 0; x < 2; x++) {
        // Step 1.  Cool down every cell a little
        for( int i = 0; i < halfLedCount; i++) {
          heat[x][i] = qsub8( heat[x][i],  random8(0, ((cooling * 10) / halfLedCount) + 2));
        }
        
        // Step 2.  Heat from each cell drifts 'up' and diffuses a little
        for( int k= halfLedCount - 1; k >= 2; k--) {
          heat[x][k] = (heat[x][k - 1] + heat[x][k - 2] + heat[x][k - 2] ) / 3;
        }
        
        // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
        if( random8() < sparking ) {
          int y = random8(7);
          heat[x][y] = qadd8( heat[x][y], random8(160,255) );
        }
        
        // Step 4.  Map from heat cells to LED colors
        for( int j = 0; j < halfLedCount; j++) {
            // Scale the heat value from 0-255 down to 0-240
            // for best results with color palettes.
            colorindex = scale8(heat[x][j], 240);
            
            CRGB color = ColorFromPalette(palette, colorindex);
            
            if(up) {
                if(x == 0) {
                    leds[(halfLedCount - 1) - j] = color;
                }
                else {
                    leds[halfLedCount + j] = color;
                }
            }
            else {
                if(x == 0) {
                    leds[j] = color;
                }
                else {
                    leds[(NUM_LEDS - 1) - j] = color;
                }
            }
        }
    }
}

int oldSecTime = 0;
int oldSec = 0;

void drawAnalogClock(byte second, byte minute, byte hour, boolean drawMillis, boolean drawSecond) {
    if(Time.second() != oldSec){
        oldSecTime = millis();
        oldSec = Time.second();
    }
    
    int millisecond = millis() - oldSecTime;
    
    int secondIndex = map(second, 0, 59, 0, NUM_LEDS);
    int minuteIndex = map(minute, 0, 59, 0, NUM_LEDS);
    int hourIndex = map(hour * 5, 5, 60, 0, NUM_LEDS);
    int millisecondIndex = map(secondIndex + millisecond * .06, 0, 60, 0, NUM_LEDS);
    
    if(millisecondIndex >= NUM_LEDS)
        millisecondIndex -= NUM_LEDS;
    
    hourIndex += minuteIndex / 12;
    
    if(hourIndex >= NUM_LEDS)
        hourIndex -= NUM_LEDS;
    
    // see if we need to reverse the order of the LEDS
    if(flipClock == 1) {
        int max = NUM_LEDS - 1;
        secondIndex = max - secondIndex;
        minuteIndex = max - minuteIndex;
        hourIndex = max - hourIndex;
        millisecondIndex = max - millisecondIndex;
    }
    
    if(secondIndex >= NUM_LEDS)
        secondIndex = NUM_LEDS - 1;
    else if(secondIndex < 0)
        secondIndex = 0;
    
    if(minuteIndex >= NUM_LEDS)
        minuteIndex = NUM_LEDS - 1;
    else if(minuteIndex < 0)
        minuteIndex = 0;
        
    if(hourIndex >= NUM_LEDS)
        hourIndex = NUM_LEDS - 1;
    else if(hourIndex < 0)
        hourIndex = 0;
        
    if(millisecondIndex >= NUM_LEDS)
        millisecondIndex = NUM_LEDS - 1;
    else if(millisecondIndex < 0)
        millisecondIndex = 0;
    
    if(drawMillis)
        leds[millisecondIndex] += CRGB(0, 0, 127); // Blue
        
    if(drawSecond)
        leds[secondIndex] += CRGB(0, 0, 127); // Blue
        
    leds[minuteIndex] += CRGB::Green;
    leds[hourIndex] += CRGB::Red;
}

// scale the brightness of all pixels down
void dimAll(byte value)
{
  for (int i = 0; i < NUM_LEDS; i++){
    leds[i].nscale8(value);
  }
}

void addGlitter( uint8_t chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}
