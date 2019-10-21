/*
  LaserWar.h - Library for flashing Morse code.
  Commands for sending, recieving and decoding LaserWar v9 commands
  Created by Eugeniy Maslyaev (mrjohn.ru), 
  September 9, 2019.
*/

#include "Arduino.h"
#include "LaserWar.h"

#define CARRIER_INTERVAL 4 // for DigiSpark
//#define CARRIER_INTERVAL 6 // for Arduino Pro Mini / Nano
#define MLT_SPACE_INTERVAL 600
#define MLT_HEADER_CYCLES 128 // adjusted experimentally
#define MLT_BIT0_CYCLES 32 // adjusted experimentally
#define MLT_BIT1_CYCLES 64 // adjusted experimentally

#define MISTAKE_MAX 100
#define ONE_DURATION 1200
#define ZERO_DURATION 600 
#define PAUSE_DURATION 600

LaserWar::LaserWar(){}

void LaserWar::sendHeader(byte pin) {
  sendPulse(MLT_HEADER_CYCLES, pin);
  delayMicroseconds(MLT_SPACE_INTERVAL);
}

void LaserWar::sendByte(byte x, byte pin) {
  for(int i=0; i<8; i++) {
    if (x & 0x80) {
      sendPulse(MLT_BIT1_CYCLES, pin);
    } else {
      sendPulse(MLT_BIT0_CYCLES, pin);
    }
    x = x << 1;
    delayMicroseconds(MLT_SPACE_INTERVAL);
  }
}

void LaserWar::sendPulse(int cycles, byte pin) {
  for (int i=0; i<cycles; i++ ) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(CARRIER_INTERVAL);
    digitalWrite(pin, LOW);
    delayMicroseconds(CARRIER_INTERVAL);
  }
}

void LaserWar::send(byte pin, unsigned long cmd){
	sendHeader(pin);
	sendByte(cmd >> 16, pin);
	sendByte((cmd & 0x00ffff) >> 8, pin);
	sendByte(cmd & 0x0000ff, pin);
}

bool LaserWar::isCorrect(unsigned long duration, int etalon){
  return duration >= (etalon - MISTAKE_MAX) && duration <= (etalon + MISTAKE_MAX);
}

int LaserWar::decodePulse(byte pin){
  unsigned long duration = pulseIn(pin, LOW, 2450);
  if (isCorrect(duration, ONE_DURATION)){
    return ONE_DURATION;
  } else if (isCorrect(duration, ZERO_DURATION)){
    return ZERO_DURATION;
  } else {
    return 0;
  }
}	
	
unsigned long LaserWar::waitCommand(byte pin){
  byte counter = 0;
  unsigned long cmd = 0;

  while(counter <= 24){
    if(digitalRead(pin) == HIGH){
      unsigned long d = decodePulse(pin);
      if (d) {
        byte b = d == ONE_DURATION ? 0x01 : 0x00;
        cmd = (cmd << 1) | b;
        counter++;

        if (counter == 24){
          return cmd;
        }
      }
    } else {
      if (!pulseIn(pin, HIGH, PAUSE_DURATION + MISTAKE_MAX)){
        counter = 0;
        cmd = 0;
      }
    }
  }
}