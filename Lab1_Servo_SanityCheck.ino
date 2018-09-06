#include <Servo.h>

Servo myServo;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  myServo.attach(3);
  Serial.println("Starting");
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("MAX");
  myServo.write(180);
  delay(2000); // 2 seconds
  Serial.println("75");
  myServo.write(100);
  delay(2000);
}
