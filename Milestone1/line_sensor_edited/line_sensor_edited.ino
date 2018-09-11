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

const int POT = A0;
const int SERVO1 = A4;
const int SERVO2 = A5;
Servo myServo1;
Servo myServo2;

const int threshold = 100;

// These variables are marked 'volatile' to inform the compiler that they can change
// at any time (as they are set by hardware interrupts).
volatile long SENSOR0_TIMER;
volatile long SENSOR1_TIMER;

// Consider smoothing this value with your favorite smoothing technique (exponential moving average?)
volatile int SENSOR0_READING;
volatile int SENSOR1_READING;

//offsets that will be applied to avg the sensor reading
volatile int numReadings0 = 0;
volatile int numReadings1 = 0;
volatile int Sensor0_Offset = 0;
volatile int Sensor1_Offset = 0;
const int calibrate = 1000;

// A digital write is required to trigger a sensor reading.
void setup_sensor(int pin, long *sensor_timer) {
  *sensor_timer = micros();
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  pinMode(pin, INPUT);
}

// this function averages the sensors starting on the line
void averageLineSensor(int pin){
  if(pin == 2){
     int temp = 0;
      for(int i = 0; i < 1000; i++){
          temp += (micros()-SENSOR0_TIMER);
        }
      Sensor0_Offset = temp/1000.0;
    }
   else if(pin == 3){
     int temp = 0;
      for(int i = 0; i < 1000; i++){
          temp += (micros()-SENSOR1_TIMER);
        }
      Sensor1_Offset = temp/1000.0;
    }
}

void move_ccw(Servo servo, int speed){
  Serial.print("CCW speed " );
  Serial.println(speed);
  speed = map(speed, 0, 90, 90, 180);
  speed = constrain(speed, 90, 180);
  servo.write(speed);
}

void move_cw(Servo servo, int speed){
  Serial.print("CW speed " );
  Serial.println(speed);
  speed = map(speed, 0, 90, 90, 0);
  speed = constrain(speed, 0, 90);
  servo.write(speed);
}

void SENSOR0_ISR() {
  // The sensor light reading is inversely proportional to the time taken
  // for the pin to fall from high to low. Lower values mean lighter colors.
  if (numReadings0 < calibrate) {
      Sensor0_Offset += micros() - SENSOR0_TIMER;
      setup_sensor(SENSOR0_PIN, &SENSOR0_TIMER);
      numReadings0++;
  }
  else if (numReadings0 == calibrate) {
      Sensor0_Offset = Sensor0_Offset/calibrate;
      setup_sensor(SENSOR0_PIN, &SENSOR0_TIMER);
      numReadings0++; // don't increment after this so that overflow doesn't occur
  }
  else {
      SENSOR0_READING = micros() - SENSOR0_TIMER - Sensor0_Offset;
      // turn right from line
      Serial.print(SENSOR0_READING);
      delay(1000);
      if(SENSOR0_READING < 0) {
        Serial.println("[ Turning right ]");
        move_ccw(myServo1, 0);
        move_ccw(myServo2, 100);
        delay(650); // 2 sec
      }
      // Reset the sensor for another reading
      setup_sensor(SENSOR0_PIN, &SENSOR0_TIMER);
  }
}

void SENSOR1_ISR() {
  if (numReadings1 < calibrate) {
      Sensor1_Offset += micros() - SENSOR1_TIMER;
      setup_sensor(SENSOR1_PIN, &SENSOR1_TIMER);
      numReadings1++;
  }
  else if (numReadings1 == calibrate) {
      Sensor1_Offset = Sensor1_Offset/calibrate;
      setup_sensor(SENSOR1_PIN, &SENSOR1_TIMER);
      numReadings1++; // don't increment after this so that overflow doesn't occur
  }
  else {
      SENSOR1_READING = micros() - SENSOR1_TIMER - Sensor1_Offset;
      //turn left from line
      Serial.print(SENSOR1_READING);
      delay(1000);
      if(SENSOR1_READING < 0) {
        Serial.println("[ Turning left ]");
        move_ccw(myServo1, 100);
        move_ccw(myServo2, 0);
        delay(650); // 2 sec
      }
      setup_sensor(SENSOR1_PIN, &SENSOR1_TIMER);
  }
}

void setup() {
  //Initialize Servo motors
  myServo1.attach(SERVO1);
  myServo2.attach(SERVO2);
  Serial.begin(9600);
  Serial.println("Starting Servos Arduino");
  
  //averageLineSensor(SENSOR0_PIN);
  Serial.print("Done Averaging");
  // Tell the compiler which pin to associate with which ISR
  attachInterrupt(digitalPinToInterrupt(SENSOR0_PIN), SENSOR0_ISR, LOW);
  attachInterrupt(digitalPinToInterrupt(SENSOR1_PIN), SENSOR1_ISR, LOW);
  
  // Setup the sensors
  setup_sensor(SENSOR0_PIN, &SENSOR0_TIMER);
  setup_sensor(SENSOR1_PIN, &SENSOR1_TIMER);
}

//Drive in a figure 8
void figureEight() {
  for(int i = 0; i < 3; i++) {
    //Straight
    move_cw(myServo1, 100);
    move_ccw(myServo2, 100);
    delay(1000); // 1 sec

    //Turn Left
    move_cw(myServo1, 100);
    move_ccw(myServo2, 100);
    delay(1000); // 1 sec
  }
  for(int i = 0; i < 4; i++) {
    //Straight
    move_cw(myServo1, 100);
    move_ccw(myServo2, 100);
    delay(1000); // 1 sec

    //Turn Left
    move_cw(myServo1, 100);
    move_ccw(myServo2, 100);
    delay(1000); // 1 sec
  }
}

void loop() {
  // These delays are purely for ease of reading.
  if ((numReadings1 > calibrate) && (numReadings0 > calibrate)) {
      Serial.println("Sensor 0");
      Serial.println(SENSOR0_READING);
      delay(500);

      Serial.println("[ Straight 1 ]");
      move_cw(myServo1, 50);
      move_ccw(myServo2, 50);
      delay(1000); // 1 sec

      //figureEight();

      /*Serial.println("Sensor 1");
      Serial.println(SENSOR1_READING);
      delay(100);*/
  }
}
