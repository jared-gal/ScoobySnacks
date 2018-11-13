/*
 * How to use the QRE1113 Digital Line Sensor by SparkFun with hardware interrupts
 * https://www.sparkfun.com/products/9454
 *
 * Note: The Arduino Uno is limited to two pins for digital state interrupts: pins 2 and 3.
 * This means you will be limited to two digital line sensors.
 *
 * Refer to this for information on attachInterrupt():
 * https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
  */

#include <Servo.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//wiring specific stuff
/*
 * A1 - front wall sensor
 * A2 - Left wall sensor
 * A3 - right wall sensor
 * A4 - Servo 1
 * A5 - Servo 2
 * 
 * Digital
 * 2 - right line sensor
 * 3 - left line sensor
 * 
 */

int stx = 5;
int sty = 5; //Starting position. Can obviously change.
int frontsize = 1;
int vissize =  1;
int frontier[20][2]; 
int visited[81][2];
int neighbor[20][2];
int mazesize = 9;

 // Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x000000001aLL, 0x000000001bLL };
static int x_start;
static int y_start;

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.
//

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_pong_back;



const static int DIST_1 = A1; // front wall sensor
const static int DIST_2 = A3; //side wall sensor
const static int DIST_3 = A2;
const static int SERVO1 = A4;
const static int SERVO2 = A5;

#define SENSOR0_PIN 2 // right
#define SENSOR1_PIN 3 // left

Servo myServo0;
Servo myServo1;
// These variables are marked 'volatile' to inform the compiler that they can change
// at any time (as they are set by hardware interrupts).
//volatile long SENSOR0_TIMER;
//volatile long SENSOR1_TIMER;

// Consider smoothing this value with your favorite smoothing technique (exponential moving average?)
volatile int s0_read;
volatile int s1_read;


static const int thresh0 = 500; // threshold for sensor 0
static const int thresh1 = 200; // threshold for sensor 1
static const int thresh_wall_f = 100; 
static const int thresh_wall_r = 100; 
static const int thresh_wall_l = 190; 

static const String dir[4] = {"east","north","west","south"};
volatile static int orientation;  
volatile static int pos;


//Code for the QRE1113 Digital board
//Outputs via the serial terminal – Lower numbers mean more reflected
//3000 or more means nothing was reflected
int readQD(int pin){
  //Returns value from the QRE1113
  //Lower numbers mean more refleacive
  //More than 3000 means nothing was reflected.
  int diff; 
  pinMode( pin, OUTPUT );
  digitalWrite( pin, HIGH );
  delayMicroseconds(10);
  pinMode( pin, INPUT );
  long t = micros();
  
  //time how long the input is HIGH, but quit after 3ms as nothing happens after that
  while (digitalRead(pin) == HIGH && (micros() - t) < 3000);
  diff = micros()-t;
  return diff;
}

// input 0-90
void move_cw(Servo servo, int speed) {
  servo.write(90-speed);
}

// input 0-90
void move_ccw(Servo servo, int speed) {
  servo.write(speed+90);
}

// stop both servos
void stopMoving() {
  move_cw(myServo0, 0);
  move_cw(myServo1, 0);
}

// go straight
void goStraight() {
  move_cw(myServo0, 5);
  move_ccw(myServo1, 5);
}

void corLeft() {
  move_cw(myServo0, 6);
  move_ccw(myServo1, 2);
}

void corRight() {
  move_cw(myServo0, 2);
  move_ccw(myServo1, 6);
}

/* uses left line sensor to turn until it reaches the next line, and keep going a little longer
 * stops moving before the end. Will do ~90deg at intersection, but 180 if on one line
 */
void turnLeft() {
  move_cw(myServo0, 5);
  move_cw(myServo1, 5);
  s1_read = readQD(SENSOR1_PIN);
  while(s1_read > thresh1) {
    s1_read = readQD(SENSOR1_PIN);
    delay(5);
  }
  while(s1_read <= thresh1) {
    s1_read = readQD(SENSOR1_PIN);
    delay(5);
  }
  delay(100);
  stopMoving();
}

// same thing as left using the other sensor
void turnRight() {

  move_ccw(myServo0, 5);
  move_ccw(myServo1, 5);
  s0_read = readQD(SENSOR0_PIN);
  while(s0_read > thresh0) {
    s0_read = readQD(SENSOR0_PIN);
    delay(5);
  }
  while(s0_read <= thresh0) {
    s0_read = readQD(SENSOR0_PIN);
    delay(5);
  }
  delay(100);
  stopMoving();
}

void senddata(unsigned long new_data) {
  radio.stopListening();

    // Take the time, and send it.  This will block until complete
    unsigned long time = millis();
    
    printf("Now sending data...");
    printf("Maze data is %lu\n", new_data);
    bool ok = radio.write(&new_data, sizeof(unsigned long) );
    if (ok)
      printf("ok...\n");
    else
      printf("failed.\n\r");

    // Now, continue listening
    radio.startListening();
}

boolean checkVisited(int a[81][2], int x, int y) {
  boolean answer = false;
  for(int i = 0; i <= 81; i++) {
    if(a[i][0] == x && a[i][1] == y) {
      answer = true;
    }
  }
}

int adjustfrontier(int frontier[20][2]) {
  for(int i = 1; i < 21; i++) {
    frontier[i-1][0] = frontier[i][0];
    frontier[i-1][1] = frontier[i][1];
  }
  return frontier[20][2];
}

void goPlaces(int xt, int yt, int x, int y) {
  //need to go west
  if(xt > x){
      if(orientation == 2){/*already oriented properly*/}
      if(orientation == 1){turnLeft();}
      if(orientation == 0){turnRight(); turnRight();}
      if(orientation == 3){turnRight();}
      orientation = 2;
    }
    //need to go east
    if(xt < x){
      if(orientation == 2){turnRight(); turnRight();}
      if(orientation == 1){turnRight();}
      if(orientation == 0){/*oriented properly*/}
      if(orientation == 3){turnLeft();}
      orientation = 0;
    }
    //need to go north
    if(yt > y){
      if(orientation == 2){turnRight();}
      if(orientation == 1){}
      if(orientation == 0){turnLeft();}
      if(orientation == 3){turnRight();turnRight();}
      orientation = 1;
    }
    //need to go south
    if(xt > x){
      if(orientation == 2){turnLeft();}
      if(orientation == 1){turnRight();turnRight();}
      if(orientation == 0){turnRight();}
      if(orientation == 3){}
      orientation = 3;
    }
}

void setup() {
  
  //
  // Print preamble
  //

  Serial.begin(9600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");

  //
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  // set the channel
  radio.setChannel(0x50);
  // set the power
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_MIN);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_250KBPS);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  //radio.setPayloadSize(8);

  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

  if ( role == role_ping_out )
  {
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  else
  {
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
  }

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
  x_start = 1;
  y_start = 2;
  
  
  myServo0.attach(A4);
  myServo1.attach(A5);
  stopMoving();
  //detect_660();
  //turnCount = 0;
  //states are as follows 
  
  //TIMSK0 = 0; // turn off timer0 for lower jitter
  pinMode(DIST_1, INPUT);
  pinMode(DIST_2, INPUT);
  pinMode(DIST_3, INPUT);
  orientation = 1;
  pos = 0;

  frontier[0][0] = stx;
  frontier[0][1] = sty;
  visited[0][0] = stx;
  visited[0][1] = sty;
}

signed int adc_read() {
  //while(!(ADCSRA & 0x10)); // wait for adc to be ready
  ADCSRA = 0xf5; // restart adc
  byte m = ADCL; // fetch adc data
  byte j = ADCH;
  int k = (j << 8) | m; // form into an int
  return k;
}

void loop() {
  // These delays are purely for ease of reading.
    while(1) { // reduces jitter
      int wall_data = 0;
      String transmit;
      //adding the current position to the transmit string
      transmit = transmit + pos/100 + "," + pos%100;
//      
//      s0_read = readQD(SENSOR0_PIN);
//      s1_read = readQD(SENSOR1_PIN);
//      if(s0_read > thresh0 && s1_read > thresh1)
//        goStraight();
//      else if(s0_read < thresh0 && s1_read > thresh1)
//        corRight();
//      else if(s0_read > thresh0 && s1_read < thresh1)
//        corLeft();
//      else if(s0_read < thresh0 && s1_read < thresh1){ // at an intersection
          int dist_f;
          int dist_r;
          int dist_l;
          int dist_tmp;
          for(int i = 0; i<20; i++){
            ADMUX = 0x41; // use adc1
            dist_tmp = adc_read(); // this is garbage value
            delay(10); // wait before reading again so that mux sets
            dist_tmp = adc_read(); // useful value
            dist_f += dist_tmp;
            
            //reading the other distance sensor
            ADMUX = 0x43;
            dist_tmp = adc_read();
            delay(10);
            dist_tmp = adc_read();
            dist_r += dist_tmp;
            
            //reading the other distance sensor
            ADMUX = 0x42;
            dist_tmp = adc_read();
            delay(10);
            dist_tmp = adc_read();
            dist_l += dist_tmp;
          }

          dist_l = dist_l/20.0;
          dist_r = dist_r/20.0;
          dist_f = dist_f/20.0;
          
          int r_ind = orientation - 1;
          if(r_ind < 0){
              r_ind = r_ind +4;
            }
            int l_ind = (orientation +1)%4;

          //__________________________________________________________________________________________________________________________
          if(dist_f >= thresh_wall_f){ //if there is no wall to the right, turn right
            transmit = transmit + "," + dir[orientation] + "=true";
            if(orientation == 0)
              wall_data += 8;
            else if(orientation == 1)
              wall_data += 16;
            else if(orientation == 2)
              wall_data += 2;
            else if(orientation == 3)
              wall_data += 4;
          }

          if(dist_l >= thresh_wall_l){ //if there is no wall to the right, turn right
            transmit = transmit + "," + dir[l_ind] + "=true";
            if(l_ind == 0)
              wall_data += 8;
            else if(l_ind == 1)
              wall_data += 16;
            else if(l_ind == 2)
              wall_data += 2;
            else if(l_ind == 3)
              wall_data += 4;
          }

          if(dist_r >= thresh_wall_r){ //if there is no wall to the right, turn right
            transmit = transmit + "," + dir[r_ind] + "=true";
            if(r_ind == 0)
              wall_data += 8;
            else if(r_ind == 1)
              wall_data += 16;
            else if(r_ind == 2)
              wall_data += 2;
            else if(r_ind == 3)
              wall_data += 4;
          }

          unsigned long t_data = ((pos/100)<<8) | ((pos%100)<<5) | wall_data;
          senddata(t_data);
      }

  while(frontier > 0) {
    int n[2] = {frontier[0][0], frontier[0][1]};
    frontier[81][2] = adjustfrontier(frontier[81][2]);
    frontsize--; //First in first out buffer (in theory).
    if(n[0] == mazesize && n[1] == mazesize) {
      Serial.println("Goal!");
      break;
    }
    else {
      //North
      if(n[1] > 1) {
        neighbor[0][0] = n[0];
        neighbor[0][1] = n[1] - 1;
        //depth++;
        if(checkVisited(visited, neighbor[0][0], neighbor[0][1]) == false) {
          //Insert code to turn robot in the appropriate direction and move
          visited[vissize][0] = neighbor[0][0]; //Array indicies start at 0, so these appended locations should always be fresh spots in the array.
          visited[vissize][1] = neighbor[0][1];
          frontier[frontsize][0] = neighbor[0][0];
          frontier[frontsize][1] = neighbor[0][1];
          vissize++;
          frontsize++;
        }
      }
      //East
      if(n[0] < mazesize) {
        neighbor[0][0] = n[0] + 1;
        neighbor[0][1] = n[1];
        //depth++;
        if(checkVisited(visited, neighbor[0][0], neighbor[0][1]) == false) {
          //Insert code to turn robot in the appropriate direction and move
          visited[vissize][0] = neighbor[0][0];
          visited[vissize][1] = neighbor[0][1];
          frontier[frontsize][0] = neighbor[0][0];
          frontier[frontsize][1] = neighbor[0][1];
          vissize++;
          frontsize++;
        }
      }
      //South
      if(n[1] < mazesize) {
        neighbor[0][0] = n[0];
        neighbor[0][1] = n[1] + 1;
        //depth++;
        if(checkVisited(visited, neighbor[0][0], neighbor[0][1]) == false) {
          //Insert code to turn robot in the appropriate direction and move
          visited[vissize][0] = neighbor[0][0];
          visited[vissize][1] = neighbor[0][1];
          frontier[frontsize][0] = neighbor[0][0];
          frontier[frontsize][1] = neighbor[0][1];
          vissize++;
          frontsize++;
        }
      }
      //West
      if(n[0] > 1) {
        neighbor[0][0] = n[0] - 1;
        neighbor[0][1] = n[1];
        //depth++;
        if(checkVisited(visited, neighbor[0][0], neighbor[0][1]) == false) {
          //Insert code to turn robot in the appropriate direction and move
          visited[vissize][0] = neighbor[0][0];
          visited[vissize][1] = neighbor[0][1];
          frontier[frontsize][0] = neighbor[0][0];
          frontier[frontsize][1] = neighbor[0][1];
          vissize++;
          frontsize++;
        }
      }
    }
  }
  
  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    //char c = 'T';
    if ( c == 'T' && role == role_pong_back )
    {
      printf("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK\n\r");

      // Become the primary transmitter (ping out)
      role = role_ping_out;
      radio.openWritingPipe(pipes[0]);
      radio.openReadingPipe(1,pipes[1]);
    }
  } 
}
