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
Servo myServo1;
Servo myServo2;

void move_ccw(Servo servo, int speed){
  speed = map(speed, 0, 90, 90, 180);
  speed = constrain(speed, 90, 180);
  servo.write(speed);
}

void move_cw(Servo servo, int speed){
  speed = map(speed, 0, 90, 90, 0);
  speed = constrain(speed, 0, 90);
  servo.write(speed);
}

void setup() {
  //Initialize Servo motors
  myServo1.attach(SERVO1);
  myServo2.attach(SERVO2);

  //TIMSK0 = 0; // turn off timer0 for lower jitter
  //ADCSRA = 0xe5; // set the adc to free running mode
  //ADMUX = 0x40; // use adc0
  //DIDR0 = 0x01; // turn off the digital input for adc0
  //pinMode(A0, INPUT);
  pinMode(DIST, INPUT);
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
  

    while(1) { // reduces jitter
      //cli();  // UDRE interrupt slows this way down on arduino1.0
      /*for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
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
      }*/
      /*int maximum = 0;
      int ind = 0;
      for (byte i = 10; i < FFT_N/2; i++){
        if (output[i] > maximum) {
          maximum =  output[i];
          ind = i;
        }
      }
      Serial.println(maximum);
      Serial.println(ind);
      if(ind == 41 && maximum > 120){
        //avoid other robot detected
      }*/  
      int dist = analogRead(DIST);
      if (dist > 350) {
        // stop
        move_cw(myServo1, 0);
        move_ccw(myServo2, 0);
        // then turn left
      }
      /*else if (output[41] > 120) {
        move_cw(myServo1, 0);
        move_ccw(myServo2, 0);
        // avoid other robot
      }*/
      else {
        move_cw(myServo1, 50);
        move_ccw(myServo2, 50);
      }

  }
}
