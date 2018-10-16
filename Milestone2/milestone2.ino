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

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library

#include <Servo.h>

const static int DIST = A1; // front wall sensor
const static int SERVO1 = A4;
const static int SERVO2 = A5;
const static int WALL_LED = 12; // green led, turn on when wall detected
const static int ROBOT_LED = 11; // red led, turn on when robot detected

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

volatile int count = 0; // counts num loops to make fft happen less often

static const int thresh0 = 500; // threshold for sensor 0
static const int thresh1 = 200; // threshold for sensor 1
static const int thresh_wall = 120; 
static const int thresh_ir = 80;
volatile int ir_fft_val;

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
 * stops moving before the end. 
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

void turn180 (){
  move_ccw(myServo0, 5);
  move_ccw(myServo1, 5);
  s0_read = readQD(SENSOR0_PIN);
  while(s0_read > thresh0) {
    s0_read = readQD(SENSOR0_PIN);
    delay(1);
  }
  delay(5);
  while(s0_read <= thresh0) {
    s0_read = readQD(SENSOR0_PIN);
    delay(1);
  }
  delay(5);
  while(s0_read > thresh0) {
    s0_read = readQD(SENSOR0_PIN);
    delay(1);
  }
  delay(5);
  while(s0_read <= thresh0) {
    s0_read = readQD(SENSOR0_PIN);
    delay(1);
  }
  delay(50);
  stopMoving();
}

int ir_fft () {
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  //analogRead(0);
  delay(20);
  //delay(1);
  for (int i = 0 ; i < 90 ; i += 2) { // save 256 samples
    while(!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int
    fft_input[i] = k; // put real data into even bins
    fft_input[i+1] = 0; // set odd bins to 0
  }
  fft_window(); // window the data for better frequency response
  fft_reorder(); // reorder the data before doing the fft
  fft_run(); // process the data in the fft
  fft_mag_log(); // take the output of the fft
  sei();
  //Serial.println("start");
  for (byte i = 0 ; i < FFT_N/2 ; i++) { 
    //Serial.println(fft_log_out[i]); // send out the data
    if (i == 41) {
      return fft_log_out[i];
    }
  }
}

void setup() {
  myServo0.attach(A4);
  myServo1.attach(A5);
  //turnCount = 0;
  //states are as follows 
  
  //TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  DIDR1 = 0x01; // turn off the digital input for adc1
  pinMode(DIST, INPUT);
  pinMode(WALL_LED, OUTPUT);
  pinMode(ROBOT_LED, OUTPUT);
//  Serial.begin(9600);
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
    //cli();  // UDRE interrupt slows this way down on arduino1.0
  count++; // variable 
  if(count == 20){ ir_fft_val = ir_fft(); count = 0;}
  while (ir_fft_val > thresh_ir) {
    digitalWrite(ROBOT_LED, HIGH);
    stopMoving();
    delay(100);
    turnRight();//CHANGE THIS FOR A REAL RESPONSE IN A BIT
    goStraight();
    delay(500); 
    ir_fft_val = ir_fft();
    digitalWrite(ROBOT_LED, LOW);
  }
  s0_read = readQD(SENSOR0_PIN);
  s1_read = readQD(SENSOR1_PIN);
  if(s0_read > thresh0 && s1_read > thresh1)
    goStraight();
  else if(s0_read < thresh0 && s1_read > thresh1)
    corRight();
  else if(s0_read > thresh0 && s1_read < thresh1)
    corLeft();
  else if(s0_read < thresh0 && s1_read < thresh1){ // at an intersection
      ADMUX = 0x41; // use adc1
      int dist = adc_read(); // this is garbage value
      delay(20); // wait before reading again so that mux sets
      dist = adc_read(); // useful value
      if (dist < thresh_wall) goStraight(); // no wall in front
      else { // wall in front
        digitalWrite(WALL_LED, HIGH);
        goStraight(); // tries to center it a little bit
        delay(500); // might need to change this value
        turnLeft(); // if wall in front, turn left
        delay(500); // stops for half a second for stability
        dist = adc_read();
        //Serial.println("third");
        //Serial.println(dist);
        if (dist < thresh_wall) {goStraight(); digitalWrite(WALL_LED, LOW);} // if no wall to the left, go straight
        else { // wall to the left
          turnRight(); // turns until next intersection
          delay(50); // wait for stability
          turnRight();
          delay(200); // wait for stability
          dist = adc_read();
          //Serial.println(dist);
          if (dist < thresh_wall) {goStraight(); digitalWrite(WALL_LED, LOW);} // if no wall behind, go straight
          else { // wall to the right
            turnRight();
            goStraight();
            digitalWrite(WALL_LED, LOW);
          }
        }
      }
  }
  }
}
