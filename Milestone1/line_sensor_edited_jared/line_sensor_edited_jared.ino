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

//Declaring the servo motors for the robot
Servo myServo1;
Servo myServo2;

//**********************update****************
//an arbitrary threshold for now 
const int threshold = 100;
//an arbitrary averaging length
const int avgLength = 10;
//*******************************************

// These variables are marked 'volatile' to inform the compiler that they can change
// at any time (as they are set by hardware interrupts).
volatile long SENSOR0_TIMER;
volatile long SENSOR1_TIMER;

// Consider smoothing this value with your favorite smoothing technique (exponential moving average?)
volatile int SENSOR0_READING;
volatile int SENSOR1_READING;

//offsets that will be applied to avg the sensor reading
volatile int SENSOR0_AVG_ARRAY[avgLength];
volatile int SENSOR1_AVG_ARRAY[avgLength];
volatile int index0;
volatile int index1;

// A digital write is required to trigger a sensor reading.
void setup_sensor(int pin, long *sensor_timer) {
  *sensor_timer = micros();
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  pinMode(pin, INPUT);
}

// this function averages the sensors values based on a simple 10 value avg
double averageLineSensor(int pin){
  double Sensor_Average = -1;
  if(pin == 2){
     int temp = 0;
      for(int i = 0; i < avgLength; i++){
          temp += SENSOR0_AVG_ARRAY[i];
        }
      Sensor_Average = temp/10.0;
    }
   else if(pin == 3){
     int temp = 0;
      for(int i = 0; i < avgLength; i++){
          temp += SENSOR1_AVG_ARRAY[i];
        }
      Sensor_Average = temp/10.0;
    }
  return Sensor_Average;
}

//this turns the specified servo motor in the counter clockwise direction
//90 - 180 when written to servo is ccw, and higher values are faster
//this function takes in a value from 0 - 90 for speed, with ascending beign faster
//and maps it to the 90-180 range before sending that to the servo
void move_ccw(Servo servo, int speed){
  Serial.print("CCW speed " );
  Serial.println(speed);
  speed = map(speed, 0, 90, 90, 180);
  speed = constrain(speed, 90, 180);
  servo.write(speed);
}

//this turns the specified servo motor in the clockwise direction
//90 - 0 when written to servo is cw, and lower values are faster
//this function takes in a value from 0 - 90 for speed, with ascending beign faster
//and maps it to the 90-0 range before sending that to the servo
void move_cw(Servo servo, int speed){
  Serial.print("CW speed " );
  Serial.println(speed);
  speed = map(speed, 0, 90, 90, 0);
  speed = constrain(speed, 0, 90);
  servo.write(speed);
}


//This function is called to cause the robot to move straight
//It employs the move_ccw and move_cw functions
//It currently is set to be slow ( speed = 50%)
void goStraight(){
  //turn servo one cw to go forwards
  move_cw(myServo1, 45);
  //turn servo two ccw to go forwards
  move_ccw(myServo2,45);
  
}

//This function stops the movement of the robot
void STOP(){
  move_cw(myServo1,0);
  move_cw(myServo2,0);
}

//This function slightly corrects the motion of the robot to the right
//by turning one direction for a brief moment before returning to the normal speed straight
//this is accomplished by slowing one wheel briefly
//In this case the the right wheel is slowed by an arbitrary value
void correctRight(){
  //find out the current speed of the right wheel, and then slowing it down 
  move_cw(myServo1, (myServo1.read() -5));
  //an arbitrary delay to let the slow down have an effect
  delay(10);
  //resetting the robot to go straight again
  goStraight();
}

//This function slightly corrects the motion of the robot to the left
//by turning one direction for a brief moment before returning to the normal speed straight
//this is accomplished by slowing one wheel briefly
//In this case the the left wheel is slowed by an arbitrary value
void correctLeft(){
  //find out the current speed of the right wheel, and then slowing it down 
  move_ccw(myServo2, (myServo2.read() -5));
  //an arbitrary delay to let the slow down have an effect
  delay(10);
  //resetting the robot to go straight again
  goStraight();

}



//In the ISR we only update the sensor reading to be fast
void SENSOR0_ISR() {
    //read the sensor
    SENSOR0_READING = micros() - SENSOR0_TIMER;
    if(index0 >= avgLength){
       index0 = 0;
    }
    SENSOR0_AVG_ARRAY[index0] = SENSOR0_READING;
    //reset the interrupt
    setup_sensor(SENSOR0_PIN, &SENSOR0_TIMER);
}

void SENSOR1_ISR() {
      //read the sensor
    SENSOR1_READING = micros() - SENSOR1_TIMER;
    if(index1 >= avgLength){
       index1 = 0;
    }
    SENSOR1_AVG_ARRAY[index1] = SENSOR1_READING;
    //reset the interrupt
    setup_sensor(SENSOR1_PIN, &SENSOR1_TIMER);
}

void setup() {
  //Initialize Servo motors
  //assuming that servo 1 is the right wheel servo
  myServo1.attach(A4);
  //assuming that servo 2 is the left wheel servo
  myServo2.attach(A5);

  //initialize serial for debugging
  Serial.begin(9600);
  Serial.println("Starting Servos Arduino");
  
  //setting up averaging
  index0 = 0;
  index1 = 0;
  
  // Tell the compiler which pin to associate with which ISR
  attachInterrupt(digitalPinToInterrupt(SENSOR0_PIN), SENSOR0_ISR, LOW);
  attachInterrupt(digitalPinToInterrupt(SENSOR1_PIN), SENSOR1_ISR, LOW);
  
  // Setup the sensors
  setup_sensor(SENSOR0_PIN, &SENSOR0_TIMER);
  setup_sensor(SENSOR1_PIN, &SENSOR1_TIMER);

  //delay some to allow the sensor averaging function to fill
  delay(10);
}


void loop() {
  Serial.println("Average of Sensor 0");
  Serial.println(averageLineSensor(SENSOR0_PIN));
  Serial.println("Average of Sensor 1");
  Serial.println(averageLineSensor(SENSOR1_PIN));
  Serial.println("end of transaction");

  //uncomment when ready to threshold
  //the case where the main sensor drops off the line
  /*
  if(averageLineSensor(SENSOR0_PIN) > threshold){
      //if the other sensor then comes on the line we know that the robot is veering left
      //so we must correct right
      if(averageLineSensor(SENSOR1_PIN) < threshold){
          correctRight();
        }
        //otherwise we assume the robot is veering right if the sensor 1 isn't detecting tape
       else if(averageLineSensor(SENSOR1_PIN) > threshold){
          correctLeft();
        }
        //if neither of these are the case we will stop the robot
       else{
          STOP();  
        }
    }
    //the case in which the main sensor is still on the line, meaning we continue to go straight
  else if (averageLineSensor(SENSOR0_PIN) < threshold){
      goStraight();
    }
  //if neither is the case then there is an error and we should stop the robot
  else{
      STOP();
    }
    */
}
