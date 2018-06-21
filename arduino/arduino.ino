#include <printf.h>
#include <RF24.h>
#include <nrf24L01.h>

#define RED_BUT 9 
#define YEL_BUT 8
#define GRN_BUT 7

//having an array of 3 values helps with debouncing
//'memory aspect'
bool rbutt[3] = {false, false, false};
bool ybutt[3] = {false, false, false}; 
bool gbutt[3] = {false, false, false};

// maximum limit for buffers. 
#define BUFFER_SIZE 28  //we chose this such that it would be less than 32 with the extra one

//buf is the buffer that we build on the arduino side
char buf[2 * BUFFER_SIZE];

//correct is the buffer that we compare it to
char correct[2 * BUFFER_SIZE];

RF24 controller(14, 15); //we declare a RF24 controller

//this is the current number of buttons saved on the arduino
uint8_t cur_buf_len = 0;

uint8_t turn = 0;

//this is the current turn
//turn + 1 is going to be the wanted length of the buffer

typedef struct {
  uint8_t b[BUFFER_SIZE];
  uint8_t turn=0;
} Data;

Data d;

//this is the struct that we send back to acknowledge if it is correct or not
typedef struct { 
  uint8_t turn_correct;
} Ack;

Ack a; 

void radio_init() {
  controller.begin();
  
  controller.setPALevel(RF24_PA_MIN);
  controller.setPayloadSize(32);
  controller.setChannel(7);
  controller.setCRCLength(RF24_CRC_16);
  controller.setDataRate(RF24_1MBPS);
  controller.openReadingPipe(1, 0xDC);
  controller.openWritingPipe(0xCC);
}

void setup() {
  // put your setup code here, to run once:
  radio_init();
  printf_begin();

  //buttons are inputs
  pinMode(RED_BUT, INPUT); 
  pinMode(YEL_BUT, INPUT);
  pinMode(GRN_BUT, INPUT);

  Serial.begin(9600);
  controller.printDetails(); //prints details before starting
}

void updatevalues(int button, bool arr[3],  char letter){  //check for button presses
  //constantly keeps the last three read button values
  arr[0] = arr[1];
  arr[1] = arr[2];
  arr[2] = (bool) digitalRead(button);
  
  //basically if it detects that it was low, and then becomes high
  //transmission is successfull
  if ( !arr[0] && arr[1] && arr[2] ) {
    buf[cur_buf_len] = letter; //adds the letter to the array
    cur_buf_len++; //re-sets end of buffer. 
    Serial.write(letter);
    Serial.println();
  }

}

// returns 0 if the buttons pressed differ from the received buf.
// else returns 1. 
int compare()
{
  for (int i=0; i < turn; i++)
  {
    if (correct[i] != buf[i])
      return 0;
  }
  return 1;
}

void loop() {
  // update the button arrays with the current button status.
  controller.startListening();
  while(!controller.available()); //do nothing if there is nothing to read
  controller.read(&d, sizeof(d));  //reads the first half of data into variable d
  d = (Data)d;
  for (int i = 0; i < BUFFER_SIZE; i++)
    correct[i] = d.b[i]; //writes this into the correct array
  turn = d.turn;

Serial.println("First half of message received.");

  while(!controller.available()); //do nothing if there is nothing to read
  controller.read(&d, sizeof(d)); //reads the second half of data into variable d
  d = (Data)d;
  for (int i = 0; i < BUFFER_SIZE; i++)
    correct[i + BUFFER_SIZE] = d.b[i]; //writes this into the second half of the array
  if ( turn != d.turn )
    Serial.println("something messed up with synchro.");

  controller.stopListening();

  Serial.println("Entire message received.");

  cur_buf_len = 0; // reset the buffer just after receiving. 

  while (cur_buf_len < turn) { //check for button presses
    updatevalues(RED_BUT, rbutt, 'R'); 
    updatevalues(YEL_BUT, ybutt, 'Y');
    updatevalues(GRN_BUT, gbutt, 'G');
  }

  uint8_t cmp_result = compare();
  a.turn_correct = cmp_result;
  while(!controller.write(&a, sizeof(a)));
  if ( cmp_result )
    Serial.println("You did it!");
  else
    Serial.println("Unlucky.");
}
