#include <Servo.h>
#include <SPI.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
      
RF24 radio(9,10);

//Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x000000001aLL, 0x000000001bLL };
static int x_start;
static int y_start;
static int x_target;
static int y_target;

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
int nsize = 1;
int vissize =  1;
int visited[81][2];
int neighbor[20][2];
int mazesize = 9;

const static int DIST_1 = A3; // front wall sensor
const static int DIST_2 = A0; //right wall sensor
const static int DIST_3 = A5; //left wall sensor
const static int SERVO1 = A1;
const static int SERVO2 = A4;
boolean usedVisited = 0;

#define SENSOR0_PIN 3 // right
#define SENSOR1_PIN 2 // left

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
static const int thresh1 = 600; // threshold for sensor 1
static const int thresh_wall_f = 150; 
static const int thresh_wall_r = 150; 
static const int thresh_wall_l = 150; 

static const String dir[4] = {"east","north","west","south"};
volatile static int orientation;  
volatile static int x_pos, y_pos;


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
  move_cw(myServo0, 20);
  move_ccw(myServo1, 20);
}

void corLeft() {
  move_cw(myServo0, 15);
  move_ccw(myServo1, 5);
}

void corRight() {
  move_cw(myServo0, 5);
  move_ccw(myServo1, 15);
}

/* uses left line sensor to turn until it reaches the next line, and keep going a little longer
 * stops moving before the end. Will do ~90deg at intersection, but 180 if on one line
 */
void turnLeft() {
  move_cw(myServo0, 10);
  move_cw(myServo1, 10);
  s1_read = readQD(SENSOR1_PIN);
  while(s1_read > thresh1) {
    s1_read = readQD(SENSOR1_PIN);
    delay(1);
  }
  while(s1_read <= thresh1) {
    s1_read = readQD(SENSOR1_PIN);
    delay(1);
  }
  delay(20);
  stopMoving();
}

// same thing as left using the other sensor
void turnRight() {

  move_ccw(myServo0, 10);
  move_ccw(myServo1, 10);
  s0_read = readQD(SENSOR0_PIN);
  while(s0_read > thresh0) {
    s0_read = readQD(SENSOR0_PIN);
    delay(1);
  }
  while(s0_read <= thresh0) {
    s0_read = readQD(SENSOR0_PIN);
    delay(1);
  }
  delay(20);
  stopMoving();
}

boolean checkVisited(int a[81][2], int x, int y) {
  boolean answer = false;
  for(int i = 0; i <= 81; i++) {
    if(a[i][0] == x && a[i][1] == y) {
      answer = true;
    }
  }
}


void orient(int xt, int yt, int x, int y) {
  //need to go west
  Serial.println("orient");
  if(xt > x){
    if(orientation == 2){/*already oriented properly*/}
      else if(orientation == 1){turnLeft();}
      else if(orientation == 0){turnLeft(); turnLeft();}
      else if(orientation == 3){turnRight();}
      orientation = 2;
    }
    //need to go east
    else if(xt < x){
      if(orientation == 2){turnLeft(); turnLeft();}
      else if(orientation == 1){turnRight();}
      else if(orientation == 0){/*oriented properly*/}
      else if(orientation == 3){turnLeft();}
      orientation = 0;
    }
    //need to go north
    else if(yt > y){
      if(orientation == 2){turnRight();}
      else if(orientation == 1){}
      else if(orientation == 0){turnLeft();}
      else if(orientation == 3){turnLeft();turnLeft();}
      orientation = 1;
    }
    //need to go south
    else if(yt < y){
      if(orientation == 2){turnLeft();}
      else if(orientation == 1){turnLeft();turnLeft();}
      else if(orientation == 0){turnRight();}
      else if(orientation == 3){}
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
  
  
  myServo0.attach(SERVO1);
  myServo1.attach(SERVO2);
  stopMoving();
  //detect_660();
  //turnCount = 0;
  //states are as follows 
  
  //TIMSK0 = 0; // turn off timer0 for lower jitter
  pinMode(DIST_1, INPUT);
  pinMode(DIST_2, INPUT);
  pinMode(DIST_3, INPUT);
  orientation = 1;
  x_pos = 0;
  y_pos = 0;

//  frontier[0][0] = x_pos;
//  frontier[0][1] = y_pos;
  visited[0][0] = x_pos;
  visited[0][1] = y_pos;



      

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

signed int adc_read() {
  //while(!(ADCSRA & 0x10)); // wait for adc to be ready
  ADCSRA = 0xf5; // restart adc
  byte m = ADCL; // fetch adc data
  byte j = ADCH;
  int k = (j << 8) | m; // form into an int
  return k;
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

void loop() {
  // These delays are purely for ease of reading.
    while(1) { // reduces jitter
      int wall_data = 0;
          
      s0_read = readQD(SENSOR0_PIN);
      s1_read = readQD(SENSOR1_PIN);
      
      // Thresholding
      Serial.print("Sensor0: ");
      Serial.print(s0_read);
      Serial.println();
      Serial.print("Sensor1: ");
      Serial.print(s1_read);
      Serial.println();
      delay(500);

      
      if(s0_read > thresh0 && s1_read > thresh1) {
        goStraight();
      }
      else if(s0_read < thresh0 && s1_read > thresh1) {
        corRight();
        //Serial.println("Correcting Right");
      }
      else if(s0_read > thresh0 && s1_read < thresh1) {
        corLeft();
   
      }
      else if(s0_read < thresh0 && s1_read < thresh1){ // at an intersection
        delay(200);
        stopMoving();
       
        //update current position
        if(orientation == 0)
          x_pos = x_pos + 1;
        else if(orientation == 1)
          y_pos = y_pos +1;
        else if(orientation == 2)
          x_pos = x_pos - 1;
        else if(orientation == 3)
          y_pos = y_pos -1;
        // add current position to visited
        visited[vissize][0] = x_pos;
        visited[vissize][1] = y_pos;
        vissize++;
        
          int dist_f;
          int dist_r;
          int dist_l;
          int dist_tmp;
          ADMUX = 0x43; // use adc1
          dist_tmp = adc_read(); // this is garbage value
          delay(10); // wait before reading again so that mux sets
          for(int i = 0; i<16; i++){
            dist_tmp = adc_read(); // useful value
            dist_f += dist_tmp;
          }
          ADMUX = 0x40;
          dist_tmp = adc_read();
          delay(10);
          for(int i = 0; i<16; i++){
            //reading the other distance sensor
            dist_tmp = adc_read();
            dist_r += dist_tmp;
          }
          ADMUX = 0x45;
          dist_tmp = adc_read();
          delay(10);
          for(int i = 0; i<16; i++){
            //reading the other distance sensor
            dist_tmp = adc_read();
            dist_l += dist_tmp;
          }
          dist_l = dist_l>>4;
          dist_r = dist_r>>4;
          dist_f = dist_f>>4;

          Serial.println("Left, Right, Front");
          Serial.println(dist_l);
          Serial.println(dist_r);
          Serial.println(dist_f);
          
          int r_ind = orientation - 1;
          if(r_ind < 0){
              r_ind = r_ind +4;
            }
            int l_ind = (orientation +1)%4;

          //_________________________________________________________________________________________________
          if(dist_l >= thresh_wall_l){ //if there is wall to left
            if(l_ind == 0)
              wall_data += 8;
            else if(l_ind == 1)
              wall_data += 16;
            else if(l_ind == 2)
              wall_data += 2;
            else if(l_ind == 3)
              wall_data += 4;
          }

//_________________________________________________________________________________________________
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~         
          //TODO: setup how to add to frontier array 
          else if(!usedVisited){
            if(orientation == 0){
              //add position x_pos , y_pos +1
              neighbor[nsize][0] = x_pos;
              neighbor[nsize][1] = y_pos+1;}
            else if(orientation == 1){
             //add position x_pos +1 , y_pos
              neighbor[nsize][0] = x_pos-1;
              neighbor[nsize][1] = y_pos;}
            else if(orientation == 2){
              neighbor[nsize][0] = x_pos;
              neighbor[nsize][1] = y_pos-1;}
              //add position x_pos , y_pos-1
            else if(orientation == 3){
              //add position x_pos-1 , y_pos
              neighbor[nsize][0] = x_pos+1;
              neighbor[nsize][1] = y_pos;}
              Serial.println("coordinates to add:");
              Serial.println(neighbor[nsize][0]);
              Serial.println(neighbor[nsize][1]);
              nsize++;

            }
        
//~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//_________________________________________________________________________________________________
          if(dist_r >= thresh_wall_r){ //if there is no wall to the right, turn right
            //transmit = transmit + "," + dir[r_ind] + "=true";
            if(r_ind == 0)
              wall_data += 8;
            else if(r_ind == 1)
              wall_data += 16;
            else if(r_ind == 2)
              wall_data += 2;
            else if(r_ind == 3)
              wall_data += 4;
          }


//_________________________________________________________________________________________________
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          //TODO: setup how to add to frontier array 
          else if (!usedVisited){
            if(orientation == 0){
              //add position x_pos , y_pos -1
              neighbor[nsize][0] = x_pos;
              neighbor[nsize][1] = y_pos-1;}
            else if(orientation == 1){
              //add position x_pos -1 , y_pos
              neighbor[nsize][0] = x_pos+1;
              neighbor[nsize][1] = y_pos;}
            else if(orientation == 2){
              //add position x_pos , y_pos+1
              neighbor[nsize][0] = x_pos;
              neighbor[nsize][1] = y_pos+1;}
            else if(orientation == 3){
              //add position x_pos+1 , y_pos
              neighbor[nsize][0] = x_pos-1;
              neighbor[nsize][1] = y_pos;}
              Serial.println("coordinates to add:");
              Serial.println(neighbor[nsize][0]);
              Serial.println(neighbor[nsize][1]);
              nsize++;
              //adjustfrontier();

            }
         
 
//~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~            
//_________________________________________________________________________________________________
           if(dist_f >= thresh_wall_f){ //if there is a wall to the front
            
            if(orientation == 0)
              wall_data += 8;
            else if(orientation == 1)
              wall_data += 16;
            else if(orientation == 2)
              wall_data += 2;
            else if(orientation == 3)
              wall_data += 4;
          }
 
//_________________________________________________________________________________________________
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          //TODO: setup how to add to frontier array 
          else if(!usedVisited){
            if(orientation == 0){
              //add position x_pos - 1, y_pos
              neighbor[nsize][0] = x_pos+1;
              neighbor[nsize][1] = y_pos;
             } else if(orientation == 1){
             //add position x_pos , y_pos+1
             neighbor[nsize][0] = x_pos;
             neighbor[nsize][1] = y_pos+1;
            } else if(orientation == 2){
              //add position x_pos + 1, y_pos
              neighbor[nsize][0] = x_pos-1;
              neighbor[nsize][1] = y_pos;
            } else if(orientation == 3){
              //add position x_pos , y_pos-1
              neighbor[nsize][0] = x_pos;
              neighbor[nsize][1] = y_pos-1;
            }
              Serial.println("coordinates to add:");
              Serial.println(neighbor[nsize][0]);
              Serial.println(neighbor[nsize][1]);
              nsize++;
              //adjustfrontier();
           }
            
        
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

          if((abs(x_pos - neighbor[nsize-1][0])+abs(y_pos - neighbor[nsize-1][1])) ==1) {
                  nsize--;
                  orient(neighbor[nsize][0],neighbor[nsize][1],x_pos,y_pos); 
                  goStraight();
                  usedVisited = 0;
          }
          else{
            orient(visited[vissize-2][0], visited[vissize-2][1], x_pos, y_pos);
            goStraight();
            vissize--;
            usedVisited = 1;
            }
          
          
          
          unsigned long t_data = ((x_pos)<<8) | ((y_pos)<<5) | wall_data;
          senddata(t_data);

//_________________________________________________________________________________________________
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

     Serial.print("target:  ");
     Serial.print(neighbor[nsize][0]);
     Serial.println(neighbor[(nsize)][1]);
//
//     
     Serial.print("current:  ");
     Serial.print(visited[(vissize-1)][0]);
     Serial.println(visited[(vissize-1)][1]);

      }
    }
}
