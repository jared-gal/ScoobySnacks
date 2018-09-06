#include <Servo.h>

const int POT = A0;
const int SERVO1 = A4;
const int SERVO2 = A5;
Servo myServo1;
Servo myServo2;

void setup() {
  // put your setup code here, to run once:
  pinMode(POT, INPUT);
  myServo1.attach(SERVO1);
  myServo2.attach(SERVO2);
  Serial.begin(9600);
  Serial.println("Starting Servos Arduino");
}

void print_servo_value(int servo_value){
  Serial.print("Going to write to servo [ ");
  Serial.print(servo_value);
  Serial.println(" ]");
}

int convert_pot_to_servo(int pot_value){
  pot_value = map(pot_value, 0, 1023, 0, 180);
  pot_value = constrain(pot_value, 0, 180);
  print_servo_value(pot_value);
  return pot_value;
}

void write_servo(Servo servo, int pot_value){
  int servo_value = convert_pot_to_servo(pot_value);
  servo.write(servo_value);
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

void loop() {
  //int pot_value = analogRead(POT);
  // write_servo(pot_value);
  // move in a straight line
  Serial.println("[ Straight 1 ]");
  move_cw(myServo1, 100);
  move_ccw(myServo2, 100);
  delay(1000); // 1 sec

  // turn
  Serial.println("[ Turning 1 ]");
  move_ccw(myServo1, 100);
  move_ccw(myServo2, 100);
  delay(650); // 2 sec
//  // move in a straight line
//  Serial.println("[ Straight 2 ]");
//  move_cw(myServo1, 100);
//  move_ccw(myServo2, 100);
//  delay(1000); // 0.5 sec
//  
//  // turn another way
//  Serial.println("[ Turning 2 ]");
//  move_cw(myServo1, 0);
//  move_ccw(myServo2, 100);
//  delay(500); // 2 sec
}
