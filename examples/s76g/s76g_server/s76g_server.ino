// s76g_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing server
// with the RH_L0RA class. RH_L0RA class does not provide for addressing or
// reliability, so you should only use RH_L0RA directly if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example s76g_client_xx
// Tested with LILYGO T-Impulse Wristband, Arduino 1.8.13, GrumpyOldPizza Arduino Core for STM32L0.

#include <SPI.h>
#include <RH_L0RA.h>

// Singleton instance of the radio driver
RH_L0RA s76g;


void setup() {  
  Serial.begin(9600);
  // Wait for serial port to be available 
  // If you do this, it will block here until a USB serial connection is made.
  // If not, it will continue without a Serial connection, but DFU mode will not be available
  // to the host without resetting the CPU with the Boot button
//  while (!Serial) ; 

  if (!s76g.init())
    Serial.println("init failed");  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  s76g.setFrequency(868.0);

  // You can change the moduation speed etc from the default
  //s76g.setModemConfig(RH_RF95::Bw125Cr45Sf128);
  //s76g.setModemConfig(RH_RF95::Bw125Cr45Sf2048);

  // The default transmitter power is 13dBm, using PA_BOOST.
  // You can set transmitter powers from 2 to 20 dBm:
  s76g.setTxPower(3);
}

void loop() { 
  if (s76g.available()) {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (s76g.recv(buf, &len)) {
      //RH_L0RA::printBuffer("request: ", buf, len);
      Serial.print("got request: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(s76g.lastRssi(), DEC);

      // Send a reply
      uint8_t data[] = "And hello back to you";
      s76g.send(data, sizeof(data));
      s76g.waitPacketSent();
      Serial.println("Sent a reply");
    } else {
      Serial.println("recv failed");
    }
  }
}
