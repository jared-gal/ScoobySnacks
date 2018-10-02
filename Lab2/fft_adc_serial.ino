/*
fft_adc_serial.pde
guest openmusiclabs.com 7.7.14
example sketch for testing the fft library.
it takes in data on ADC0 (Analog0) and processes them
with the fft. the data is sent out over the serial
port at 115.2kb.
*/

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library

volatile static int state;
volatile static int valid;
int output[128];
boolean detectedTone;  

int stateMachine(int binValue){
    switch (state){
      case 0:
        valid = 0;
        if (binValue>35) state++;
        break; 
      case 1:
        if(binValue >35){
          state++;
          valid = 1;
        }
        else state--; 
        break;  
      case 2:
        if(binValue > 35){}
        else state++;
        break;
      case 3:
        if(binValue > 35){state--;}
        else {state = 0; valid = 0;}
        break;
    }
    return valid; 
}

void setup() {
  Serial.begin(115200); // use the serial port
  /*TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = B11100111; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0*/
  pinMode(A2, INPUT) ;
  valid = 0;
  state = 0;
  detectedTone = 0;
}

void loop() {
  while(1) { // reduces jitter
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
      Serial.println(".....");
      if(stateMachine(fft_log_out[20])){    
        Serial.println("660Hz present"); // send out the data
        detectedTone = 1;
        
        //Reset ADC to free running mode for IR detection
        TIMSK0 = 0; // turn off timer0 for lower jitter
        ADCSRA = B11100111; // set the adc to free running mode
        ADMUX = 0x40; // use adc0
        DIDR0 = 0x01; // turn off the digital input for adc0
      }
      delay(100);
      //for (byte i = 0 ; i < FFT_N/2 ; i++) { 
      //Serial.println(fft_log_out[i]); // send out the data
      //}
    }
    else { // Continue with IR detection
       
      cli();  // UDRE interrupt slows this way down on arduino1.0
      for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
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
      }
      int maximum = 0;
      int ind = 0;
      for (byte i = 10; i < FFT_N/2; i++){
        if (output[i] > maximum) {
          maximum =  output[i];
          ind = i;
        }
      }

     //Serial.println("Nothing detected");
      //Serial.println(maximum);
      //Serial.println(ind);
      if(ind == 41 && maximum > 120){
        //avoid other robot detected
        Serial.println("Other robot detected");
      }
    }
  }
}
