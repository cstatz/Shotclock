#include <SPI.h>
#include "nRF24L01.h"
#include <RF24.h>
#include "printf.h"

#define CE_PIN 9
#define CS_PIN 10
#define HORN_PIN 2

#define DATARATE RF24_250KBPS
#define CHANNEL 99
#define TRANSMITTER_ADDRESS 0xABCDABCDE1LL
#define RECEIVER_ADDRESS 0xABCDABCDD2LL

RF24 radio(CE_PIN, CS_PIN);
const uint64_t pipes[2] = {TRANSMITTER_ADDRESS, RECEIVER_ADDRESS};
byte receive_buffer[2];

bool horn_played = true;
uint8_t pipe_num;

int pin_1[] = {22, 23, 24, 25, 26, 27, 4};
int pin_0[] = {28, 29, 8, 7, 6, 5, 3};

byte numbers[] = {
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

void on(int pin[], byte state) {
  for (int i=0;i<7;i++){
    digitalWrite(pin[i],((state >> (6-i)) & 0b1 ));
  }
}

void off(int pin[]) {
  for (int i=0;i<7;i++){
    digitalWrite(pin[i],0);
  }
}

void setup(void)
{

  for (int i=0; i<7; i++) {
    pinMode(pin_1[i], OUTPUT);
    pinMode(pin_0[i], OUTPUT);
  }

  pinMode(HORN_PIN, OUTPUT);

  for (int i=0; i<7; i++){
    digitalWrite(pin_0[i],HIGH);
    delay(500);
    digitalWrite(pin_0[i],LOW);
  }

  for (int i=0; i<7; i++){
    digitalWrite(pin_1[i],HIGH);
    delay(500);
    digitalWrite(pin_1[i],LOW);
  }

  Serial.begin(57600);
  printf_begin();

  radio.begin();
  radio.setPayloadSize(2);
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1, pipes[0]);
  radio.setDataRate(DATARATE);
  radio.setChannel(CHANNEL);
  radio.setPALevel(RF24_PA_MIN);
  radio.setAutoAck(false);
  radio.printDetails();
  radio.startListening();

}

void loop(void)
{
  if ( radio.available(&pipe_num) && pipe_num == 1)
  {
    radio.read(receive_buffer, 2);
    if (receive_buffer[1] > 0) {

      off(pin_0);
      off(pin_1);

      if (receive_buffer[0] == 0) {
        if (!horn_played) {
          horn_played = true;
          digitalWrite(HORN_PIN, HIGH);
          delay(500);
          digitalWrite(HORN_PIN, LOW);
        }
      }
      else {horn_played = false;}

      uint8_t val_10 = receive_buffer[0] / 10;
      on(pin_1, numbers[val_10]);
      uint8_t val_1 = receive_buffer[0] % 10;
      on(pin_0, numbers[val_1]);

    }
    else {

      off(pin_0);
      off(pin_1);

    }

    printf("received pipe: %d %d \n\r", receive_buffer[1], receive_buffer[0]);
  }

}
