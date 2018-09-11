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

// These variables are marked 'volatile' to inform the compiler that they can change
// at any time (as they are set by hardware interrupts).
volatile long SENSOR0_TIMER;
volatile long SENSOR1_TIMER;

// Consider smoothing this value with your favorite smoothing technique (exponential moving average?)
volatile int SENSOR0_READING;
volatile int SENSOR1_READING;

const int len = 30;
int sensor0[len];
int sensor1[len];
int i0 = 0;
int i1 = 0;
int total0 = 0;
int total1 = 0;

// A digital write is required to trigger a sensor reading.
void setup_sensor(int pin, long *sensor_timer) {
  *sensor_timer = micros();
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  pinMode(pin, INPUT);
}

void averaging(){//(int pin, int reading) {
//  if (pin == 2) {
//    total0 = total0 - sensor0[i0];
//    sensor0[i0] = reading;
//    total0 = total0 + sensor0[i0];
//    i0++;
//    if (i0 >= len) {
//      i0 = 0;
//    }
//    SENSOR0_READING = total0/len;
//  }
//  else {
//    total1 = total1 - sensor1[i1];
//    sensor1[i1] = reading;
//    total1 = total1 + sensor0[i1];
//    i1++;
//    if (i1 >= len) {
//      i1 = 0;
//    }
//    SENSOR1_READING = total1/len;
//  
}

void SENSOR0_ISR() {
  // The sensor light reading is inversely proportional to the time taken
  // for the pin to fall from high to low. Lower values mean lighter colors.
  SENSOR0_READING = micros() - SENSOR0_TIMER;
  for(int i = 0; i <100; i++)
    average(SENSOR0_PIN, SENSOR0_READING);
  // Reset the sensor for another reading
  setup_sensor(SENSOR0_PIN, &SENSOR0_TIMER);
}

void SENSOR1_ISR() {
  SENSOR1_READING = micros() - SENSOR1_TIMER;
  setup_sensor(SENSOR1_PIN, &SENSOR1_TIMER);
}

void setup() {
  Serial.begin(9600);

  // Tell the compiler which pin to associate with which ISR
  attachInterrupt(digitalPinToInterrupt(SENSOR0_PIN), SENSOR0_ISR, LOW);
  attachInterrupt(digitalPinToInterrupt(SENSOR1_PIN), SENSOR1_ISR, LOW);

  for (int i = 0; i < len; i++) {
    sensor0[i] = 0;
    sensor1[i] = 0;
  }
  
  // Setup the sensors
  setup_sensor(SENSOR0_PIN, &SENSOR0_TIMER);
  setup_sensor(SENSOR1_PIN, &SENSOR1_TIMER);
}

void loop() {
  // These delays are purely for ease of reading.
  Serial.println("Sensor 0");
  Serial.println(SENSOR0_READING);
  delay(500);
  /*Serial.println("Sensor 1");
  Serial.println(SENSOR1_READING);
  delay(100);*/
}
