#include <FastLED.h>
#include <WiFi.h>
#include <SPI.h>

#define LED_PIN     23
#define NUM_LEDS    144

CRGB leds[NUM_LEDS];
int stop = 1;


void settings(int hue, int brightness, int saturation){
   
}

void loop() {
  //LED??
  while(stop == 1){
    for(int i = 0;i < 72;i++){
        leds[i] = CRGB(153,0,0);
        leds[143 - i] = CRGB(153,0,0);
        FastLED.show();
        
    }
  }
    
  stop = 0;

}