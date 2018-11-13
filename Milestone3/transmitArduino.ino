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

void setup() {
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
  Serial.begin(9600);
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
      String transmit = "";
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
          }

          if(dist_l >= thresh_wall_l){ //if there is no wall to the right, turn right
            transmit = transmit + "," + dir[l_ind] + "=true";
          }

          if(dist_r >= thresh_wall_r){ //if there is no wall to the right, turn right
            transmit = transmit + "," + dir[r_ind] + "=true";
          }

          Serial.println(transmit);
      }


}
