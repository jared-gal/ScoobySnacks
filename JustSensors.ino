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

#include <FFT.h>  // include the library

#include <Servo.h>

const static int DIST_1 = A3; // front wall sensor
const static int DIST_2 = A0; //right wall sensor
const static int DIST_3 = A5; //left wall sensor
const static int SERVO1 = A1;
const static int SERVO2 = A4;
const static int WALL_LED = 12; // green led, turn on when wall detected
const static int ROBOT_LED = 11; // red led, turn on when robot detected

#define SENSOR0_PIN 3 // right
#define SENSOR1_PIN 2 // left

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

static const int thresh0 = 600; // threshold for sensor 0
static const int thresh1 = 600; // threshold for sensor 1
static const int thresh_wall = 150; 
static const int thresh_ir = 80;
volatile int ir_fft_val;

volatile static int state = 0;
volatile static int valid = 0;
boolean detectedTone = 0;  

static const int sound_thresh = 50;

int stateMachine(int binValue){
    switch (state){
      case 0:
        valid = 0;
        if (binValue>sound_thresh) state++;
        break; 
      case 1:
        if(binValue >sound_thresh){
          state++;
          valid = 1;
        }
        else state--; 
        break;  
      case 2:
        if(binValue > sound_thresh){}
        else state++;
        break;
      case 3:
        if(binValue > sound_thresh){state--;}
        else {state = 0; valid = 0;}
        break;
    }
    return valid; 
}

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
  move_cw(myServo0, 20);
  move_ccw(myServo1, 20);
}

void corLeft() {
  move_cw(myServo0, 15);
  move_ccw(myServo1, 5);
}

void corRight() {
  move_cw(myServo0, 5);
  move_ccw(myServo1, 15);
}

/* uses left line sensor to turn until it reaches the next line, and keep going a little longer
 * stops moving before the end. Will do ~90deg at intersection, but 180 if on one line
 */
void turnLeft() {
  move_cw(myServo0, 10);
  move_cw(myServo1, 10);
  s1_read = readQD(SENSOR1_PIN);
  while(s1_read > thresh1) {
    s1_read = readQD(SENSOR1_PIN);
    delay(1);
  }
  while(s1_read <= thresh1) {
    s1_read = readQD(SENSOR1_PIN);
    delay(1);
  }
  delay(20);
  stopMoving();
}

// same thing as left using the other sensor
void turnRight() {

  move_ccw(myServo0, 10);
  move_ccw(myServo1, 10);
  s0_read = readQD(SENSOR0_PIN);
  while(s0_read > thresh0) {
    s0_read = readQD(SENSOR0_PIN);
    delay(1);
  }
  while(s0_read <= thresh0) {
    s0_read = readQD(SENSOR0_PIN);
    delay(1);
  }
  delay(20);
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

void detect_660 () {
  if(!detectedTone) { // Detect 660 Hz tone first
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      fft_input[i] = analogRead(A2); // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    sei();
    //Serial.println(".....");
    if(stateMachine(fft_log_out[20])){    
      //Serial.println("660Hz present"); // send out the data
      detectedTone = 1;
      ADCSRA = 0xe5; // set the adc to free running mode
      ADMUX = 0x40; // use adc0
      DIDR0 = 0x01; // turn off the digital input for adc0
      DIDR1 = 0x01; // turn off the digital input for adc1
    }
    delay(100);
  }
}

void setup() {
  myServo0.attach(SERVO1);
  myServo1.attach(SERVO2);
  stopMoving();
  //detect_660();
  //turnCount = 0;
  //states are as follows 
  
  //TIMSK0 = 0; // turn off timer0 for lower jitter
  pinMode(DIST_1, INPUT);
  pinMode(WALL_LED, OUTPUT);
  pinMode(ROBOT_LED, OUTPUT);
  pinMode(A2, INPUT) ;
  valid = 0;
  state = 0;
  detectedTone = 0;
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
//  if (!detectedTone) detect_660();
//  else {
  while(1) { // reduces jitter
    //cli();  // UDRE interrupt slows this way down on arduino1.0
  count++; // variable 
  /*if(count == 20){ ir_fft_val = ir_fft(); count = 0;}
  while (ir_fft_val > thresh_ir) {
    digitalWrite(ROBOT_LED, HIGH);
    stopMoving();
    delay(100);
    turnRight(); // this will do a 180 if it is between lines
    goStraight();
    delay(500); 
    ir_fft_val = ir_fft();
    digitalWrite(ROBOT_LED, LOW);
  }*/
  s0_read = readQD(SENSOR0_PIN);
  s1_read = readQD(SENSOR1_PIN);
  if(s0_read > thresh0 && s1_read > thresh1)
    goStraight();
  else if(s0_read < thresh0 && s1_read > thresh1){
    corRight();
    //delay(20);
  }
  else if(s0_read > thresh0 && s1_read < thresh1){
    corLeft();
    //delay(20);
  }
  else if(s0_read < thresh0 && s1_read < thresh1){ // at an intersection
          stopMoving();
          int dist_f;
          int dist_r;
          int dist_l;
          int dist_tmp;
          ADMUX = 0x43; // use adc1
          dist_tmp = adc_read(); // this is garbage value
          delay(10); // wait before reading again so that mux sets
          for(int i = 0; i<16; i++){
            dist_tmp = adc_read(); // useful value
            dist_f += dist_tmp;
          }
          ADMUX = 0x40;
          dist_tmp = adc_read();
          delay(10);
          for(int i = 0; i<16; i++){
            //reading the other distance sensor
            dist_tmp = adc_read();
            dist_r += dist_tmp;
          }
          ADMUX = 0x45;
          dist_tmp = adc_read();
          delay(10);
          for(int i = 0; i<16; i++){
            //reading the other distance sensor
            dist_tmp = adc_read();
            dist_l += dist_tmp;
          }
          dist_l = dist_l>>4;
          dist_r = dist_r>>4;
          dist_f = dist_f>>4;

          goStraight();
          if (dist_f > thresh_wall) {
            if (dist_r > thresh_wall && dist_l > thresh_wall) {
              delay(150);
              turnLeft();
              delay(20);
              turnLeft();
              //  `delay(200);
            }
            else if (dist_r > thresh_wall) {
              //goStraight();
              delay(150);
              turnLeft();
              //delay(200);
            }
            else {
              delay(150);
              turnRight();
            }
          }
          else {
            goStraight();
          }
  }
  }
  }
//}
