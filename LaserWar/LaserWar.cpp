/*
  LaserWar.h - Library for flashing Morse code.
  Commands for sending, recieving and decoding LaserWar v9 commands
  Created by Eugeniy Maslyaev (mrjohn.ru), 
  September 9, 2019.
*/

#include "Arduino.h"
#include "LaserWar.h"

#define CARRIER_INTERVAL 4 // adjusted experimentally
#define MLT_SPACE_INTERVAL 600
#define MLT_HEADER_CYCLES 128 // adjusted experimentally
#define MLT_BIT0_CYCLES 32 // adjusted experimentally
#define MLT_BIT1_CYCLES 64 // adjusted experimentally
#define MLT_HEADER_LENGTH   2400
#define MLT_BIT0_LENGTH   600
#define MLT_BIT1_LENGTH   1200
#define MLT_HEADER_MIN_LENGTH (MLT_HEADER_LENGTH-500)
#define MLT_HEADER_MAX_LENGTH (MLT_HEADER_LENGTH+300)
#define MLT_BIT0_MIN_LENGTH   (MLT_BIT0_LENGTH-200)
#define MLT_BIT0_MAX_LENGTH   (MLT_BIT0_LENGTH+200)
#define MLT_BIT1_MIN_LENGTH   (MLT_BIT1_LENGTH-250)
#define MLT_BIT1_MAX_LENGTH   (MLT_BIT1_LENGTH+250)
#define MLT_SHOT_DATA_LENGTH  14
#define MLT_CMD_DATA_LENGTH   24
#define MLT_MAX_DATA_LENGTH   24
#define PULSE_TIMEOUT 2000

LaserWar::LaserWar(){}

char LaserWar::scanBits(const char* bitBuf, byte count) {
  char value = bitBuf[0];
  for (char i=1; i < count; i++) {
    value = value << 1;
    value = value | bitBuf[i];
  }
  return value;
}

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
	
unsigned long LaserWar::waitCommand(byte pin){
	if( digitalRead(pin) == LOW ) {
    unsigned long headerStartTime = micros();
    while ( digitalRead(pin) == LOW );
    unsigned long headerEndTime = micros();

    int headerLength = headerEndTime - headerStartTime;
    if ( headerLength > MLT_HEADER_MIN_LENGTH && headerLength < MLT_HEADER_MAX_LENGTH ) {
	  char buf[MLT_MAX_DATA_LENGTH];
      char receivedPulseCount = 0;

      while (receivedPulseCount < MLT_MAX_DATA_LENGTH) {
        int pulseLength = 0;
        if( digitalRead(pin) == HIGH ) {
          pulseLength = pulseIn(pin, LOW, PULSE_TIMEOUT);
        }

        if(pulseLength > MLT_BIT0_MIN_LENGTH && pulseLength < MLT_BIT0_MAX_LENGTH ) {
          buf[receivedPulseCount] = 0;
          receivedPulseCount++;
        } else if(pulseLength > MLT_BIT1_MIN_LENGTH && pulseLength < MLT_BIT1_MAX_LENGTH ) {
          buf[receivedPulseCount] = 1;
          receivedPulseCount++;
        } else if (pulseLength > MLT_HEADER_MIN_LENGTH && pulseLength < MLT_HEADER_MAX_LENGTH) {
          receivedPulseCount = 0;
          continue;
        } else {
          break;
        }
      }

      if ( receivedPulseCount==MLT_CMD_DATA_LENGTH && buf[0]==1 ) {
        byte byte0 = (byte)scanBits(buf, 8);
        byte byte1 = (byte)scanBits(buf+8, 8);
        byte byte2 = (byte)scanBits(buf+16, 8);
		
		return ((((unsigned long)byte0 << 8) | byte1) << 8) | byte2;
      }
    }
  }

  return 0;
}