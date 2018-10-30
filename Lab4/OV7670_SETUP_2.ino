#include <Wire.h>

#define OV7670_I2C_ADDRESS 0x21  /*TODO: write this in hex (eg. 0xAB) */


///////// Main Program //////////////
void setup() {
  Wire.begin();
  Serial.begin(9600);
  //Serial.println("stuff");
  
  // TODO: READ KEY REGISTERS
  
  delay(100);
  
  // TODO: WRITE KEY REGISTERS
  String a = OV7670_write_register(0x0C , ((0 | (1 << 3)) >> 1));
  //Serial.println(a);
  //Serial.println("stuff");
  OV7670_write_register(0x11 , ((0x80 | (1 << 6)) >> 1));
  OV7670_write_register(0x12 , ((0 | (33 << 1)) >> 1));
  OV7670_write_register(0x14 , ((0x4A | (1 << 4)) >> ));
  OV7670_write_register(0x1E , ((0 | (3 << 4)) >> 1);
  OV7670_write_register(0x3E , ((0x0E | (1 << 3)) >> 1));
  //Serial.println("stuff");
  
}

void loop(){
  //Serial.println("stuff");
  Serial.println("Address 0C:");
  Serial.println(read_register_value(0x0C));
  Serial.println("Address 11:");
  Serial.println(read_register_value(0x11));
  Serial.println("Address 12:");
  Serial.println(read_register_value(0x12));
  Serial.println("Address 14:");
  Serial.println(read_register_value(0x14));
  Serial.println("Address 1E:");
  Serial.println(read_register_value(0x1E));
  Serial.println("Address 3E:");
  Serial.println(read_register_value(0x3E));
  set_color_matrix();
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
