#include <EEPROM.h>

#define TONE_PIN 0
#define BTN_PIN 1
#define RESET_PIN 2
#define IR_PIN 3
#define BANGS_COUNT 3
#define BEFORE_BANG_DELAY 4
#define RESPAWN_CMD 0x8302e8

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

unsigned char used = '0';

void setup() {
  pinMode(IR_PIN, OUTPUT);
  pinMode(RESET_PIN, INPUT);
  used = EEPROM.read(0);
  if (used != '1') used = '0';
}

void loop() {
  if (used == '1'){
    if (waitRespawn()){
      restore();
    }
  } else {
    if (digitalRead(BTN_PIN) == HIGH){
      bang();
    }
  }
}

void restore(){
  tone(TONE_PIN, 1000);
  delay(700);
  noTone();

  used = '0';
  EEPROM.write(0, used);
}

void bang(){
  for (byte i = 0; i < BEFORE_BANG_DELAY; i++){
    tone(TONE_PIN, 1000);
    delay(500);
    noTone();
    delay(500);
  }

  for (byte i = 0; i < BANGS_COUNT; i++){
    fire();
    delay(100);
  }

  used = '1';
  EEPROM.write(0, used);
}

void fire() {
  sendHeader();
  sendByte(0x83);
  sendByte(0x00);
  sendByte(0xE8);
}

void sendHeader() {
  sendPulse(MLT_HEADER_CYCLES);
  delayMicroseconds(MLT_SPACE_INTERVAL);
}

void sendByte(byte x) {
  for(int i=0; i<8; i++) {
    if (x & 0x80) {
      sendPulse(MLT_BIT1_CYCLES);
    } else {
      sendPulse(MLT_BIT0_CYCLES);
    }
    x = x << 1;
    delayMicroseconds(MLT_SPACE_INTERVAL);
  }
}

void sendPulse(int cycles) {
  for (int i=0; i<cycles; i++ ) {
    digitalWrite(IR_PIN, HIGH);
    delayMicroseconds(CARRIER_INTERVAL);
    digitalWrite(IR_PIN, LOW);
    delayMicroseconds(CARRIER_INTERVAL);
  }
}

bool waitRespawn() {
  if( digitalRead(RESET_PIN) == LOW ) {
    unsigned long headerStartTime = micros();
    while ( digitalRead(RESET_PIN) == LOW );
    unsigned long headerEndTime = micros();

    int headerLength = headerEndTime - headerStartTime;
    if ( headerLength > MLT_HEADER_MIN_LENGTH && headerLength < MLT_HEADER_MAX_LENGTH ) {
      char buf[MLT_MAX_DATA_LENGTH];
      char receivedPulseCount = 0;

      while (receivedPulseCount < MLT_MAX_DATA_LENGTH) {
        int pulseLength = 0;
        if( digitalRead(RESET_PIN) == HIGH ) {
          pulseLength = pulseIn(RESET_PIN, LOW, PULSE_TIMEOUT);
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
        char byte0 = scanBits(buf, 8);
        char byte1 = scanBits(buf+8, 8);
        char byte2 = scanBits(buf+16, 8);

        if ((byte)byte0 == (RESPAWN_CMD & 0xff0000) >> 16
          && (byte)byte1 == (RESPAWN_CMD & 0x00ff00) >> 8
          && (byte)byte2 == (RESPAWN_CMD & 0x0000ff)){
            return true;
        }
      }
    }
  }

  return false;
}

char scanBits(const char* bitBuf, char count) {
  char value = bitBuf[0];
  for (char i=1; i < count; i++) {
    value = value << 1;
    value = value | bitBuf[i];
  }
  return value;
}
