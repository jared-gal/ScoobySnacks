

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

#define SENSOR0_PIN 2
#define SENSOR1_PIN 3
#include <Servo.h>

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
  long t = micros()
  
  //time how long the input is HIGH, but quit after 3ms as nothing happens after that
  while (digitalRead(pin1) == HIGH && micros() - time < 3000);
  diff = micros()-time
  return diff
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
  move_ccw(myServo1, 4);
}

void corRight() {
  move_cw(myServo0, 4);
  move_ccw(myServo1, 6);
}

void turnLeft() {
  move_cw(myServo0, 5);
  move_cw(myServo1, 0);
}

void turnRight() {
  move_ccw(myServo0, 0);
  move_ccw(myServo1, 5);
}

int turnCount;

void setup() {
  myServo0.attach(A4);
  myServo1.attach(A5);
  turnCount = 0;
  //states are as follows 
}

void loop() {
  //getting the sensor values each loop
  s0_read = readQD(SENSOR0_PIN);
  s1_read = readQD(SENSOR1_PIN);
  //s2_read = readQD(SENSOR2_PIN);

  if(s0_read > threshold0 && s1_read > threshold1)
    goStraight();
  else if(s0_read < threshold0 && s1_read > threshold1)
    corLeft();
  else if(s0_read > threshold0 && s1_read < threshold1)
    corRight();
  else if(s0_read < threshold0 && s1_read < threshold1){
      if(turnCount<4){
        turnRight();
        turnCount++;
      }
       else
        turnLeft();

}
