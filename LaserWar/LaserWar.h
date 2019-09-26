/*
  LaserWar.h - Library for flashing Morse code.
  Commands for sending, recieving and decoding LaserWar v9 commands
  Created by Eugeniy Maslyaev (mrjohn.ru), 
  September 9, 2019.
*/

#ifndef LaserWar_h
#define LaserWar_h
#include "Arduino.h"

class LaserWar {
	public:
		LaserWar();
		unsigned long waitCommand(byte pin);
		void send(byte pin, unsigned long cmd);
	private:
		void sendPulse(int cycles, byte pin);
		void sendByte(byte x, byte pin);
		void sendHeader(byte pin);
		int decodePulse(byte pin);
		bool isCorrect(unsigned long duration, int etalon);
};

#endif