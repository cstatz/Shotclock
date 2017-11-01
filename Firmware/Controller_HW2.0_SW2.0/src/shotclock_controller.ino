#include <Bounce2.h>
#include "TimerOne.h"
#include <SPI.h>
#include "nRF24L01.h"
#include <RF24.h>
#include <FastLED.h>

// Configuration
#define CE_PIN 2
#define CS_PIN 3

#define BUTTON_CLEAR_PIN A0
#define BUTTON_START_PIN A2
#define BUTTON_CLEAR_GND A1
#define BUTTON_START_GND A3

#define LED_DATA_PIN 8
#define LED_PWR_PIN 7
#define LED_GND_PIN 9

#define DATARATE RF24_250KBPS
#define CHANNEL 99
#define TRANSMITTER_ADDRESS 0xABCDABCDE1LL
#define RECEIVER_ADDRESS 0xABCDABCDD2LL

#define BRIGHTNESS  255
#define LED_TYPE    PL9823
#define COLOR_ORDER RGB

CRGB leds[2];

RF24 radio(CE_PIN, CS_PIN);
const uint64_t pipes[2] = {TRANSMITTER_ADDRESS, RECEIVER_ADDRESS};
byte send_buffer[2];

uint8_t counter = 30;
bool clock_running = false;
bool clock_on = false;

Bounce debouncer_clear = Bounce();
Bounce debouncer_start = Bounce();

int last_value_start = HIGH;
int last_value_clear = HIGH;

void update_counter() {

  if (clock_running) {
    if (counter>0) {
      counter--;
    }
    else {
      clock_running=false;
    }
  }
}

void setup(void)
{

  delay(500); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, 2);
  FastLED.setCorrection( TypicalLEDStrip );
  FastLED.clear();

  Serial.begin(57600);

  pinMode(BUTTON_CLEAR_PIN, INPUT_PULLUP);
  pinMode(BUTTON_START_PIN, INPUT_PULLUP);
  pinMode(BUTTON_START_GND, OUTPUT);
  pinMode(BUTTON_CLEAR_GND, OUTPUT);

  pinMode(LED_DATA_PIN, OUTPUT);
  pinMode(LED_PWR_PIN, OUTPUT);
  pinMode(LED_GND_PIN, OUTPUT);

  digitalWrite(BUTTON_START_GND, LOW);
  digitalWrite(BUTTON_CLEAR_GND, LOW);
  digitalWrite(LED_GND_PIN, LOW);
  digitalWrite(LED_PWR_PIN, HIGH);

  debouncer_clear.attach(BUTTON_CLEAR_PIN);
  debouncer_start.attach(BUTTON_START_PIN);
  debouncer_clear.interval(50); // interval in ms
  debouncer_start.interval(50); // interval in ms

  Timer1.initialize(1000000);
  Timer1.attachInterrupt(update_counter);
  Timer1.start();

  radio.begin();
  radio.setPayloadSize(2);
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.setDataRate(DATARATE);
  radio.setChannel(CHANNEL);
  radio.setPALevel(RF24_PA_MIN) ;
  radio.setAutoAck(false) ;
  radio.stopListening();

}

void loop(void)
{
  if (clock_running){
    if (counter==0) {
      leds[0] = CRGB::Red;
    }
    else if (counter <= 10) {
      leds[0] = CRGB::Orange;
    }
    else {
      leds[0] = CRGB::Green;
    }
  }
  else {
    leds[0] = CRGB::Black;
  }

  debouncer_start.update();
  debouncer_clear.update();

  int value_start = debouncer_start.read();
  int value_clear = debouncer_clear.read();

  if ((value_start == LOW) && last_value_start == HIGH) {
    if ((clock_running == false) && (clock_on == true)) {
      counter = 30;
      clock_running = true;
    }
    else {
      clock_running = false;
      leds[0] = CRGB::Black;
    }
  }

  if (clock_running==false) {
    if ((value_clear == LOW) && (last_value_clear == HIGH)) {
      if (clock_on == false) {
        clock_on = true;
        counter = 30;
        leds[1] = CRGB::Red;
      }
      else {
        clock_on = false;
        leds[1] = CRGB::Black;
        counter = 30;
      }
    }
  }
  else {
    if ((value_clear == LOW) && (last_value_clear == HIGH)) {
      counter = 30;
      leds[0] = CRGB::Green;
    }
  }

  last_value_start = value_start;
  last_value_clear = value_clear;

  send_buffer[0] = counter;

  if (clock_on == true) {
    send_buffer[1] = 1;
  }
  else {
    send_buffer[1] = 0;
  }

  bool ok = radio.write(send_buffer, 2, true);
  delay(5);

  FastLED.show();

}
