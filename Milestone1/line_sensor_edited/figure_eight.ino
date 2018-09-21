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

  if(s0_read > thresh0 && s1_read > thresh1)
    goStraight();
  else if(s0_read < thresh0 && s1_read > thresh1)
    corRight();
  else if(s0_read > thresh0 && s1_read < thresh1)
    corLeft();
  else if(s0_read < thresh0 && s1_read < thresh1){
      if(turnCount<4){
        turnRight();
        delay(2000);
        turnCount++;
      }
       else{
        turnLeft();
        delay(2000);
       }
  }
}
