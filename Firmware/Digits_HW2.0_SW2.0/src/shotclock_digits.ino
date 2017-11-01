/**
 *    10      1
 *  -----   -----
 * |  2  | |  2  |
 * |1   3| |3   1|
 * |  0  | |  0  |
 *  -----o o-----
 * |     | |     |
 * |6   4| |4   6|
 * |  5  | |  5  |
 *  -----   -----
 */

#include <SPI.h>
#include "nRF24L01.h"
#include <RF24.h>
#include <FastLED.h>

#define CE_PIN 11
#define CS_PIN 12
#define GND_PIN 13
#define LED_1_PIN A4
#define LED_10_PIN A5
#define HORN_PIN A3

#define PIXELS_PER_ELEMENT 5
#define ELEMENTS_PER_DIGIT 7

#define BRIGHTNESS  255
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds_1[PIXELS_PER_ELEMENT*ELEMENTS_PER_DIGIT];
CRGB leds_10[PIXELS_PER_ELEMENT*ELEMENTS_PER_DIGIT];

#define DATARATE RF24_250KBPS
#define CHANNEL 99
#define TRANSMITTER_ADDRESS 0xABCDABCDE1LL
#define RECEIVER_ADDRESS 0xABCDABCDD2LL

RF24 radio(CE_PIN, CS_PIN);
const uint64_t pipes[2] = {TRANSMITTER_ADDRESS, RECEIVER_ADDRESS};
byte receive_buffer[2];

bool horn_played = true;
uint8_t pipe_num;

byte digit_10[] = {
 0b01111110,
 0b00011000,
 0b01101101,
 0b00111101,
 0b00011011,
 0b00110111,
 0b01110111,
 0b00011100,
 0b01111111,
 0b00111111
};

byte digit_1[] = {
 0b01111110,
 0b01000010,
 0b00110111,
 0b01100111,
 0b01001011,
 0b01101101,
 0b01111101,
 0b01000111,
 0b01111111,
 0b01101111
};

void set_leds(CRGB * leds, byte digit, CRGB color)
{
  for (int i=0;i<ELEMENTS_PER_DIGIT;i++){
    bool on = (digit >> (ELEMENTS_PER_DIGIT-1-i)) & 0b1;
    if (on) {
      for (int j=0;j<PIXELS_PER_ELEMENT;j++){
        leds[i*PIXELS_PER_ELEMENT+j] = color;
      }
    }
    else {
      for (int j=0;j<PIXELS_PER_ELEMENT;j++){
        leds[i*PIXELS_PER_ELEMENT+j] = CRGB::Black;
      }
    }
  }
}

void setup(void)
{
  pinMode(GND_PIN, OUTPUT);
  digitalWrite(GND_PIN, LOW);
  delay(1000);

  Serial.begin(57600);
  Serial.println("Start");
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_10_PIN, OUTPUT);
  pinMode(HORN_PIN, OUTPUT);

  Serial.println("Radio:");

  radio.begin();
  Serial.println("Radio: 1");

  radio.setPayloadSize(2);
  Serial.println("Radio: 2");

  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  Serial.println("Radio: 3");

  radio.setDataRate(DATARATE);
  radio.setChannel(CHANNEL);
  radio.setPALevel(RF24_PA_MIN);
  Serial.println("Radio: 4");

  radio.setAutoAck(false);
  radio.printDetails();
  Serial.println("Radio: 5");

  radio.startListening();

  // Init LED_Strips
  delay(500); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_1_PIN, COLOR_ORDER>(leds_1, PIXELS_PER_ELEMENT*ELEMENTS_PER_DIGIT);
  FastLED.addLeds<LED_TYPE, LED_10_PIN, COLOR_ORDER>(leds_10, PIXELS_PER_ELEMENT*ELEMENTS_PER_DIGIT);
  FastLED.setCorrection( TypicalLEDStrip );

  for (int i=0; i<PIXELS_PER_ELEMENT*ELEMENTS_PER_DIGIT; i++) {
    leds_1[i] = CRGB::Red;
    leds_10[i] = CRGB::Red;
  }
  FastLED.show();
  delay(500);
  for (int i=0; i<PIXELS_PER_ELEMENT*ELEMENTS_PER_DIGIT; i++) {
    leds_1[i] = CRGB::Blue;
    leds_10[i] = CRGB::Blue;
  }
  FastLED.show();
  delay(500);
  for (int i=0; i<PIXELS_PER_ELEMENT*ELEMENTS_PER_DIGIT; i++) {
    leds_1[i] = CRGB::Green;
    leds_10[i] = CRGB::Green;
  }
  FastLED.show();
  delay(500);
  for (int i=0; i<PIXELS_PER_ELEMENT*ELEMENTS_PER_DIGIT; i++) {
    leds_1[i] = CRGB::Black;
    leds_10[i] = CRGB::Black;
  }
  FastLED.show();
  FastLED.clear();
}

void loop(void)
{

  Serial.println("Go");
  if ( radio.available(&pipe_num) && pipe_num == 1)
  {
    radio.read(receive_buffer, 2);
    if (receive_buffer[1] > 0) {

      if (receive_buffer[0] == 0) {
        if (!horn_played) {
          horn_played = true;
          digitalWrite(HORN_PIN, HIGH);
          delay(500);
          digitalWrite(HORN_PIN, LOW);
          }
        }
      else {horn_played = false;}

      CRGB color;
      if (receive_buffer[0] == 0) {color = CRGB::Red;}
      else if (receive_buffer[0] <= 10) {color = CRGB::Orange;}
      else {color = CRGB::Green;}

      uint8_t val_10 = receive_buffer[0] / 10;
      set_leds(leds_10, digit_10[val_10], color);
      uint8_t val_1 = receive_buffer[0] % 10;
      set_leds(leds_1, digit_1[val_1], color);

      }
    else {
      for (uint8_t i=0; i<PIXELS_PER_ELEMENT*ELEMENTS_PER_DIGIT; i++){
        leds_1[i] = CRGB::Black;
        leds_10[i] = CRGB::Black;
        }
      }

    FastLED.show();
  }

}
