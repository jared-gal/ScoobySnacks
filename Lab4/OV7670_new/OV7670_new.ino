#include <Wire.h>

#define OV7670_I2C_ADDRESS 0x21  /*TODO: write this in hex (eg. 0xAB) */
#define triangle 2
#define square 3
#define diamond 4
#define red 5
#define blue 6
#define LED_Blue 10
#define LED_Red 11

//static byte tri;
//static byte sqr;
//static byte dia;
static byte rd;
static byte bl;
/*
int treasure_count = 0;
int red_count = 0;
int blue_count = 0;
int VSYNC_count = 0;
int VSYNC_last = VSYNC;
while (VSYNC_count < 500) {
  VSYNC_last = VSYNC;
  delay(1);
  if (VSYNC && VSYNC != VSYNC_last) {
    if (treasure) {
      if (red) {
        treasure_count++; red_count++;
      } else {
        treasure_count++; blue_count++;
      }
    }
  }
}
if (treasure_count > 375) {
  treasure_detected;
}
else {
  no_treasure_detected;
}
treasure_count = 0; blue_count = 0; red_count = 0; VSYNC_count = 0;
*/

int readValue(int pin){
  int sum = 0;
    for(int i=0; i < 1000; i++){
          if(digitalRead(pin) > 0){
                sum++;
            }
      } 
      if(sum > 500)
        return 1; 
      else
        return 0;
}


int printed_State;
///////// Main Program //////////////
void setup() {
  Wire.begin();
  Serial.begin(9600);
  //Serial.println("stuff");
  
  // TODO: READ KEY REGISTERS
  
  delay(100);
  
  // TODO: WRITE KEY REGISTERS
//    Serial.println("Old Register Values");
//  Serial.println("Address 0x12:");
//  Serial.println(read_register_value(0x12));
//  Serial.println("Address 0x0c:");
//  Serial.println(read_register_value(0x0C));
//  Serial.println("Address 0x11:");
//  Serial.println(read_register_value(0x11));
//  Serial.println("Address 0x40:");
//  Serial.println(read_register_value(0x40));
//  Serial.println("Address 0x42:");
//  Serial.println(read_register_value(0x42));
//  Serial.println("Address 1E:");
//  Serial.println(read_register_value(0x1E));


//  COm 7, com3, clkrc, com15, com17, mvfp
  //Com 7 for reset (first line resets regs, secocond is for colorbar/RGB mode)  
  OV7670_write_register(0x12 , 0x80);
    delay(5); // ta told us to do this but it makes it worse 
  //color bar test/ RGB       - value of 4 sets RGB mode, 6 sets color bar and RGB 
  //OV7670_write_register(0x12 , 0x06);
  
  //Com3 for enable scaling
  OV7670_write_register(0x0C , 0x08);
  delay(5);
  //CLKRC for external clock
  OV7670_write_register(0x11 , 0xC0); //0x40
  delay(5);
  //com15 for RGB format
  OV7670_write_register(0x40 , 0xD0); 
  delay(5);
  //COM7 
  OV7670_write_register(0x12 , 0x0C); //0x0C for non color bar
  delay(5);
  //com17 color bar test
  OV7670_write_register(0x42 , 0x00); // 0x00 for non color bar
  delay(5);
  //com7
  OV7670_write_register(0x14 , 0x51);
  delay(5);
  
  //mvfp for mirror and flip screen
  //OV7670_write_register(0x1E , 0x30);

  
  set_color_matrix();
//   Serial.println("-----------------------------------------------------------------------------------------------------------------------------");
//   Serial.println("New Register Values");
//  Serial.println("Address 0x12:");
//  Serial.println(read_register_value(0x12));
//  Serial.println("Address 0x0c:");
//  Serial.println(read_register_value(0x0C));
//  Serial.println("Address 0x11:");
//  Serial.println(read_register_value(0x11));
//  Serial.println("Address 0x40:");
//  Serial.println(read_register_value(0x40));
//  Serial.println("Address 0x42:");
//  Serial.println(read_register_value(0x42));
//  Serial.println("Address 1E:");
//  Serial.println(read_register_value(0x1E));
  //pinMode(triangle,INPUT);
  //pinMode(square,INPUT);
  //pinMode(diamond,INPUT);
  pinMode(red,INPUT);
  pinMode(blue,INPUT);
  pinMode(LED_Blue,OUTPUT);
  pinMode(LED_Red,OUTPUT);
  digitalWrite(LED_Blue, LOW);
  digitalWrite(LED_Red, LOW);
  printed_State = -3;
  //tri = 0;
  //sqr = 0;
  //dia = 0;
  rd = 0;
  bl = 0; 
}

void loop(){
  
  delay(500);

  //tri = digitalRead(triangle);
  //sqr = digitalRead(square);
  //dia = digitalRead(dia);
  rd = readValue(red);
  bl = readValue(blue);
  int state = rd;

//  if(tri > 0 && sqr ==0 && dia ==0){
//      Serial.println("Triangle Detected") ;
//  }
//  else if(tri == 0 && sqr >0 && dia ==0){
//      Serial.println("Square Detected") ;
//  }
//  else if(tri == 0 && sqr ==0 && dia >0){
//      Serial.println("Diamond Detected") ;
//  }
//  else{
//   Serial.println("No Treasure"); 
//  }
  if( rd >0){
      Serial.println();
       Serial.println();
      Serial.println();
    printed_State = state;
    if( bl > 0){
      Serial.println("COLOR: BLUE");   
      digitalWrite(LED_Blue, HIGH);
      digitalWrite(LED_Red, LOW);
      delay(500);
    }
    else if( bl == 0){
        Serial.println("COLOR: RED"); 
        digitalWrite(LED_Blue, LOW);
        digitalWrite(LED_Red, HIGH);
        delay(500);
      }
    
  }
  else
     digitalWrite(LED_Blue, LOW);
    digitalWrite(LED_Red, LOW);
    Serial.println("NO TREASURE");
}


///////// Function Definition //////////////
void read_key_registers(){
  /*TODO: DEFINE THIS FUNCTION*/
  ///read_register_value();
}

byte read_register_value(int register_address){
  byte data = 0;
  Wire.beginTransmission(OV7670_I2C_ADDRESS);
  Wire.write(register_address);
  Wire.endTransmission();
  Wire.requestFrom(OV7670_I2C_ADDRESS,1);
  while(Wire.available()<1);
  data = Wire.read();
  return data;
}

String OV7670_write(int start, const byte *pData, int size){
    int n,error;
    Wire.beginTransmission(OV7670_I2C_ADDRESS);
    n = Wire.write(start);
    if(n != 1){
      return "I2C ERROR WRITING START ADDRESS";   
    }
    n = Wire.write(pData, size);
    if(n != size){
      return "I2C ERROR WRITING DATA";
    }
    error = Wire.endTransmission(true);
    if(error != 0){
      return String(error);
    }
    return "no errors :)";
 }

String OV7670_write_register(int reg_address, byte data){
  return OV7670_write(reg_address, &data, 1);
 }

void set_color_matrix(){
    OV7670_write_register(0x4f, 0x80);
    OV7670_write_register(0x50, 0x80);
    OV7670_write_register(0x51, 0x00);
    OV7670_write_register(0x52, 0x22);
    OV7670_write_register(0x53, 0x5e);
    OV7670_write_register(0x54, 0x80);
    OV7670_write_register(0x56, 0x40);
    OV7670_write_register(0x58, 0x9e);
    OV7670_write_register(0x59, 0x88);
    OV7670_write_register(0x5a, 0x88);
    OV7670_write_register(0x5b, 0x44);
    OV7670_write_register(0x5c, 0x67);
    OV7670_write_register(0x5d, 0x49);
    OV7670_write_register(0x5e, 0x0e);
    OV7670_write_register(0x69, 0x00);
    OV7670_write_register(0x6a, 0x40);
    OV7670_write_register(0x6b, 0x0a);
    OV7670_write_register(0x6c, 0x0a);
    OV7670_write_register(0x6d, 0x55);
    OV7670_write_register(0x6e, 0x11);
    OV7670_write_register(0x6f, 0x9f);
    OV7670_write_register(0xb0, 0x84);
}
