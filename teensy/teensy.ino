#include <RF24.h>
#include <printf.h>

#define BUFFER_LENGTH 60
uint8_t buf[BUFFER_LENGTH];

#define RED_LED 17
#define YELLOW_LED 18
#define GREEN_LED 15

uint32_t turn = 1;

typedef struct {
  uint8_t buf[30];
  uint8_t turn = 1;
} Data;

typedef struct {
  uint8_t correct;
} Check;

Data data;
Check checker;

RF24 radio(9, 10);

void init() {
  radio.setPALevel(RF24_PA_LOW);
  radio.setPayloadSize(32);
  radio.setChannel(15);
  radio.setCRCLength(RF24_CRC_16);
  radio.setDataRate(RF24_1MBPS);
  radio.openReadingPipe(1, 0xc2c2c2c2c2);
  radio.openWritingPipe(0xe7e7e7e7e7);
}

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  Serial.begin(9600);

  radio.begin();
  init();
 
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
}

uint8_t rngColor()
{
  int value = random(3);
  switch(value) {
    case 0:
      return 'R';
    case 1:
      return 'G';
    case 2:
      return 'Y';
  }
}

void setLED(uint8_t color, uint8_t status)
{
  switch(color) {
    case 'R': 
      digitalWrite(RED_LED, status);
      break;
    case 'G':
      digitalWrite(GREEN_LED, status); 
      break;
    case 'Y': 
      digitalWrite(YELLOW_LED, status);
      break;
  }
}

void loop() 
{
  buf[turn - 1] = rngColor();

  for (uint32_t i = 0; i < turn; i++) {
    setLED(buf[i], HIGH);   
    delay(1000);
    setLED(buf[i], LOW);
    delay(300);
  }
  data.turn = turn;

  for (int i = 0; i < 30; i++) {
    data.buf[i] = buf[i];
  }
  
  for ( int i = 0; i < 30; i++ ) {
    data.buf[i] = buf[30 + i];
  }
  
  radio.startListening();
  while (!radio.available()) {}
  
  radio.read(&checker, sizeof(checker));
  uint8_t input = checker.correct;
  radio.stopListening();
  
  switch(input) {
    case 0: 
      turn = 1;
      digitalWrite(RED_LED, HIGH);
      delay(1000);
      digitalWrite(RED_LED, LOW);
      delay(1000);
      Serial.println("Game Over.");
      break;
    case 1:  
      turn++;
      digitalWrite(GREEN_LED, HIGH);
      delay(1000);
      digitalWrite(GREEN_LED, LOW);
      delay(1000);
      Serial.println("Correct Code.");
      break;
    default: 
      break;
  }
}
