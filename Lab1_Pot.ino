const int POT = A0;
const int LED = 3;

void setup() {
  // put your setup code here, to run once:
  pinMode(POT, INPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  Serial.println("Starting to read the pot");
}

void loop() {
  // Read the pot
  int pot_value = analogRead(POT);
  // Convert the pot readings to PWM
  pot_value = map(pot_value, 0, 1023, 0, 255);
  pot_value = constrain(pot_value, 0, 255);

  // Debug print statements
  Serial.print("Writing PWM value of [ ");
  Serial.print(pot_value);
  Serial.println(" ]");

  // Write to LED
  analogWrite(LED, pot_value);
  delay(10);
}
