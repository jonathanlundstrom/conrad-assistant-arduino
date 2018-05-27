/**
 * Author: Jirka van der Roost & Jonathan Lundström
 * Thanks to Adafruit and their awesome example on multitasking and Neopixel patterns
 * https://learn.adafruit.com/multi-tasking-the-arduino-part-1?view=all
 */
 
#include <Adafruit_NeoPixel.h>

int PIN = 6;                          // The digital data pin connected to the NeoPixel.
int LEDS = 16;                        // The number of pixels on the connected NeoPixel ring.
int index = 0;                        // Global increment value for cycling through effects.
int Fade = 5;                         // Used for breathing/pulsating effects.

int lastMessage = 0;                  // The last message received.
int incomingMessage = 9;              // The current incoming message.
int confirmationCounter = 0;           // Current number of confirmation messages.
unsigned long lastUpdate = 0;          // Last time updated.

// Initiate NeoPixel ring with above settings.
Adafruit_NeoPixel ring = Adafruit_NeoPixel(LEDS, PIN, NEO_GRB + NEO_KHZ800);


/**
 * Setup method.
 * Initialize serial connection and start the NeoPixel ring.
 */
void setup() {
  Serial.begin(115200);
  ring.begin();
  ring.show();
}


/**
 * The main loop
 * Reads serial byte (USB) from the Raspberry Pi so that the Neopixel ring can react to the input.
 */
void loop() {
  if (Serial.available() > 0) {
    char ch = Serial.read(); 
    incomingMessage = ch - '0';
    if (incomingMessage != lastMessage) {
      lastMessage = incomingMessage;
      index = 0;
      confirmationCounter = 0;
    }
  }

  // Switch animation based on message.
  switch (incomingMessage) {
    
    /**
     * Starting (0)
     * Fast paced rainbow animation.
     */
    case 0:
      starting();
      break;

    /**
     * Idle (1)
     * Slow pulsating/breathing animation.
     */
    case 1:
      breathe();
      break;

    /**
     * Listening (2)
     * Turn all pixels into solid blue color.
     */
    case 2:
      fill(ring.Color(0, 152, 221));
      break;

    /**
     * Talking (3)
     * The Assistant is talking back.
     * Theater chase animation between blue and yellow.
     */
    case 3:
      talking();
      break;

    /**
     * Confirmation (4)
     * Two green blinks to end current session.
     * Automatically switches to idle state (1) on complete.
     */
    case 4:
      confirm();
      break;

    /**
     * Wait (5)
     * Gives the breathing animation a short pause.
     */
    case 5:
      wait();
      break;

    /**
     * Turn off (6)
     * Set all the NeoPixel LEDs to black.
     */
    case 6:
      fill(ring.Color(0, 0, 0));
      break;
  }
}


/**
 * Fill "animation"
 * Instantaneously fill all LEDs with provided solid color.
 */
void fill(uint32_t c) {
  for (uint16_t i = 0; i < ring.numPixels(); i++) {
    ring.setPixelColor(i, c);
    ring.show();
  }
}


/**
 * Wheel animation
 * Input a value 0 to 255 to get a color value.
 * The colours are a transition r - g - b - back to r.
 */
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return ring.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  
  if (WheelPos < 170) {
    WheelPos -= 85;
    return ring.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  
  WheelPos -= 170;
  return ring.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


/**
 * Starting animation
 * Display a rainbow cycle pattern on all LEDs.
 */
void starting() {
  if ((millis() - lastUpdate) > 5) {
    if (index >= 255) {
      index = 0;
    }

    for (int i = 0; i < ring.numPixels(); i++) {
      ring.setPixelColor(i, Wheel(((i * 256 / ring.numPixels()) + index) & 255));
    }
    
    index++;
    lastUpdate = millis();
    ring.show();
  }
}


/**
 * Idle animation
 * Breathing/pulsating animation when assistant is idle.
 */
void breathe() {
  if (confirmationCounter >= 1 && index <= 5) {
    incomingMessage = 5;
    confirmationCounter = 0;
  }

  if ((millis() - lastUpdate) > 50) {
    if (index <= 5 && Fade < 0) {
      Fade = -Fade;
      confirmationCounter += 1;
    }

    if (index >= 100 && Fade > 0) {
      Fade = -Fade;
      confirmationCounter += 1;
    }

    fill(ring.Color(index, index, index));

    index = index + Fade;
    lastUpdate = millis();
  }
}


/**
 * Talking animation
 * Shown then the assistant is talking back to you.
 * Theater chase animation between blue and yellow.
 */
void talking() {
  if ((millis() - lastUpdate) > 80) {
    if (index >= 255) index = 0;

    for (int i = 0; i < ring.numPixels(); i++) {
      if ((i + index) % 4 == 0) {
        ring.setPixelColor(i, ring.Color(255, 230, 0));  // Yellow
      } else {
        ring.setPixelColor(i, ring.Color(0, 152, 221));  // Blue
      }
    }
    
    index++;
    lastUpdate = millis();
    ring.show();
  }
}


/**
 * Confirm animation
 * Two green blinks to show that the current sesstion has ended.
 */
void confirm() {
  if (confirmationCounter >= 3 && index <= 5) {
    incomingMessage = 1;
    confirmationCounter = 0;
  }

  if ((millis() - lastUpdate) > 1) {
    if (index <= 5 && Fade < 0) {
      confirmationCounter += 1;
      Fade = -Fade;
    }

    if (index >= 100 && Fade > 0) {
      Fade = -Fade;
      confirmationCounter += 1;
    }
    
    fill(ring.Color(0, index, 0));
    index = index + Fade;
    lastUpdate = millis();
  }
}


/**
 * Wait method
 * Gives the breathing animation a short break before it continues.
 */
void wait() {
  if ((millis() - lastUpdate) > 1000) {
    incomingMessage = 1;
  }
}
