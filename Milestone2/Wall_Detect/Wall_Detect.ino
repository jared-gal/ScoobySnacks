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
int output[128];

#include <FFT.h> // include the library

#include <Servo.h>

const int DIST = A1;
const int SERVO1 = A4;
const int SERVO2 = A5;

#define SENSOR0_PIN 2
#define SENSOR1_PIN 3

Servo myServo0;
Servo myServo1;
// These variables are marked 'volatile' to inform the compiler that they can change
// at any time (as they are set by hardware interrupts).
//volatile long SENSOR0_TIMER;
//volatile long SENSOR1_TIMER;

// Consider smoothing this value with your favorite smoothing technique (exponential moving average?)
volatile int s0_read;
volatile int s1_read;

int thresh0 = 500;
int thresh1 = 200;
int thresh_wall = 200;

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

void turnLeft() {
  move_cw(myServo0, 5);
  move_cw(myServo1, 0);
  delay(1950);
  move_cw(myServo0, 0);
  move_cw(myServo1, 0);
  delay(500);
}

void turnRight() {
  move_ccw(myServo0, 10);
  move_ccw(myServo1, 10);
  delay(750);
}

int turnCount;

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
  //Serial.begin(9600);
}

signed int adc_read() {
  while(!(ADCSRA & 0x10)); // wait for adc to be ready
  ADCSRA = 0xf5; // restart adc
  byte m = ADCL; // fetch adc data
  byte j = ADCH;
  int k = (j << 8) | m; // form into an int
  k -= 0x0200; // form into a signed int
  k <<= 6; // form into a 16b signed int
  return k;
}

void loop() {
  // These delays are purely for ease of reading.
  while(1) { // reduces jitter
    //cli();  // UDRE interrupt slows this way down on arduino1.0
    //ADCSRA = 0xe5; // set the adc to free running mode
    ADMUX = 0x40; // use adc0
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
      output[i] = fft_log_out[i];
    }

  s0_read = readQD(SENSOR0_PIN);
  s1_read = readQD(SENSOR1_PIN);
  //s2_read = readQD(SENSOR2_PIN);
  if(s0_read > thresh0 && s1_read > thresh1)
    goStraight();
  else if(s0_read < thresh0 && s1_read > thresh1)
    corRight();
  else if(s0_read > thresh0 && s1_read < thresh1)
    corLeft();
  else if(s0_read < thresh0 && s1_read < thresh1){ // at an intersection
      ADMUX = 0x41;
      move_cw(myServo1, 0);
      move_cw(myServo0, 0);
      //delay(1);
      int dist = adc_read();
      //Serial.println(dist);
      if (dist <= thresh_wall) goStraight();
      else {
        turnLeft(); // if wall in front, turn left
        dist = adc_read();
        if (dist <= thresh_wall) goStraight(); // if no wall to the left, go straight
        else {
          turnLeft();
          dist = adc_read();;
          if (dist <= thresh_wall) goStraight(); // if no wall behind, go straight
          else {
            turnLeft();
            dist = adc_read();
            if (dist <= thresh_wall) goStraight(); // if no wall to the right, go straight
            else turnLeft();
          }
        }
      }
  }
  }
}
