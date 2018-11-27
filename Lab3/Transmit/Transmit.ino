/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example for Getting Started with nRF24L01+ radios.
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two
 * different nodes.  Put one of the nodes into 'transmit' mode by connecting
 * with the serial monitor and sending a 'T'.  The ping node sends the current
 * time to the pong node, which responds by sending the value back.  The ping
 * node can then see how long the whole cycle took.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x000000001aLL, 0x000000001bLL };
static int x_start;
static int y_start;

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.
//

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_pong_back;

void setup(void)
{
  
  //
  // Print preamble
  //

  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");

  //
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  // set the channel
  radio.setChannel(0x50);
  // set the power
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_MIN);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_250KBPS);

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

  if ( role == role_ping_out )
  {
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  else
  {
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
  }

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
  x_start = 1;
  y_start = 2;
}

unsigned long shiftpack(unsigned long x_coord, unsigned long y_coord, unsigned long pos_data) {
  unsigned long new_data  =  x_coord << 8 | y_coord << 5 | pos_data;
  //printf("maze data is: %d", new_data);
  return new_data;
}

void senddata(unsigned long new_data) {
  radio.stopListening();

    // Take the time, and send it.  This will block until complete
    unsigned long time = millis();
    
    printf("Now sending data...");
    printf("Maze data is %lu\n", new_data);
    bool ok = radio.write(&new_data, sizeof(unsigned long) );
    if (ok)
      printf("ok...\n");
    else
      printf("failed.\n\r");

    // Now, continue listening
    radio.startListening();
}

void loop(void)
{
  //
  // Ping out role.  Repeatedly send the current time
  //
  
  if (role == role_ping_out)
  {
    // Run through a 3x3 maze
    Serial.println("reset");
    delay(1000);
    Serial.println("0,0,north=true,west=true,south=true");
    unsigned long x_coord = 0;
    unsigned long y_coord = 0;
    unsigned long pos_data = 22;
    unsigned long new_data = shiftpack(x_coord, y_coord, pos_data);
    senddata(new_data);
    delay(1000);
    Serial.println("0,1,north=true,south=true");
    x_coord = 0;
    y_coord = 1;
    pos_data = 20;
    new_data = shiftpack(x_coord, y_coord, pos_data);
    senddata(new_data);
    delay(1000);
    Serial.println("0,2,north=true,east=true");
    x_coord = 0;
    y_coord = 2;
    pos_data = 24;
    new_data = shiftpack(x_coord, y_coord, pos_data);
    senddata(new_data);
    delay(1000);
    Serial.println("1,2,east=true,south=true");
    x_coord = 1;
    y_coord = 2;
    pos_data = 12;
    new_data = shiftpack(x_coord, y_coord, pos_data);
    senddata(new_data);
    delay(1000);
    Serial.println("1,1,north=true,south=true");
    x_coord = 1;
    y_coord = 1;
    pos_data = 20;
    new_data = shiftpack(x_coord, y_coord, pos_data);
    senddata(new_data);
    delay(1000);
    Serial.println("1,0,west=true,north=true");
    x_coord = 1;
    y_coord = 0;
    pos_data = 18;
    new_data = shiftpack(x_coord, y_coord, pos_data);
    senddata(new_data);
    delay(1000);
    Serial.println("2,0,south=true,west=true");
    x_coord = 2;
    y_coord = 0;
    pos_data = 6;
    new_data = shiftpack(x_coord, y_coord, pos_data);
    senddata(new_data);
    delay(1000);
    Serial.println("2,1,south=true,north=true");
    x_coord = 2;
    y_coord = 1;
    pos_data = 20;
    new_data = shiftpack(x_coord, y_coord, pos_data);
    senddata(new_data);
    delay(1000);
    Serial.println("2,2,south=true,east=true,north=true");
    x_coord = 2;
    y_coord = 2;
    pos_data = 28;
    new_data = shiftpack(x_coord, y_coord, pos_data);
    senddata(new_data);
    delay(5000);

    // Try again 1s later
    delay(1000);
  }

  if ( Serial.available() )
  {
    //char c = toupper(Serial.read());
    char c = 'T';
    if ( c == 'T' && role == role_pong_back )
    {
      printf("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK\n\r");

      // Become the primary transmitter (ping out)
      role = role_ping_out;
      radio.openWritingPipe(pipes[0]);
      radio.openReadingPipe(1,pipes[1]);
    }
  }
}
// vim:cin:ai:sts=2 sw=2 ft=cpp
