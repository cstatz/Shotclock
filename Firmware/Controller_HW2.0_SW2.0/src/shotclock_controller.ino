#include <Bounce2.h>
#include "TimerOne.h"
#include <SPI.h>
#include "nRF24L01.h"
#include <RF24.h>
#include "printf.h"

// Configuration
#define CE_PIN 11
#define CS_PIN 12
#define BUTTON_CLEAR_PIN 2
#define BUTTON_START_PIN 3

#define DATARATE RF24_250KBPS
#define CHANNEL 99
#define TRANSMITTER_ADDRESS 0xABCDABCDE1LL
#define RECEIVER_ADDRESS 0xABCDABCDD2LL

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
      counter=30;
    }
  }
}


void setup(void)
{

  Serial.begin(57600);
  printf_begin();

  pinMode(BUTTON_CLEAR_PIN, INPUT_PULLUP);
  pinMode(BUTTON_START_PIN, INPUT_PULLUP);

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
  radio.setPALevel( RF24_PA_HIGH ) ;
  radio.setAutoAck( false ) ;
  radio.stopListening();

}

void loop(void)
{

  debouncer_start.update();
  debouncer_clear.update();

  int value_start = debouncer_start.read();
  int value_clear = debouncer_clear.read();
  
  if ((value_start == LOW) && last_value_start == HIGH) {
    if ((clock_running == false) && (clock_on == true)) {
      clock_running = true;
    }
    else {
      clock_running = false;
    }
  }
  if (clock_running==false) {
    if ((value_clear == LOW) && (last_value_clear == HIGH)) {
      if (clock_on == false) {
        clock_on = true;
      }
      else {
        clock_on = false;
        counter = 30;
      }
    }
  }
  else {
    if ((value_clear == LOW) && (last_value_clear == HIGH)) {
      counter = 30;
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
  
  bool ok = radio.write(send_buffer, 2, true );
  delay(5);

}
