// s76g_encrypted_datagram_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create an addressed unreliable messageing server with encrypted communications
// with the RHDatagram class, using the RH_L0RA driver to control a SX1276 radio in Murata CMWX1ZZABZ module.
// In order for this to compile you MUST uncomment the #define RH_ENABLE_ENCRYPTION_MODULE line
// at the bottom of RadioHead.h, AND you MUST have installed the Crypto directory from arduinolibs:
// http://rweather.github.io/arduinolibs/index.html
//  Philippe.Rochat'at'gmail.com
//  06.07.2017
// It is designed to work with the other example s76g_encrypted_datagram_client_xxx
// Tested with LILYGO T-Impulse Wristband, Arduino 1.8.13, GrumpyOldPizza Arduino Core for STM32L0.

#include <SPI.h>
#include <RH_L0RA.h>
#include <RHEncryptedDriver.h>
#include <Speck.h>
#include <RHDatagram.h>

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// Singleton instance of the radio driver
RH_L0RA s76g;
Speck myCipher;   // Instanciate a Speck block ciphering
RHEncryptedDriver encryptedLoRa(s76g, myCipher); // Instantiate the driver with those two

// Class to manage message delivery and receipt, using the driver declared above
RHDatagram manager(encryptedLoRa, SERVER_ADDRESS);

float frequency = 868.0; // Change the frequency here. 
unsigned char encryptkey[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}; // The very secret key


void setup() {  

  Serial.begin(9600);
  while (!Serial) ;
  Serial.println("S76G Encrypted Datagram Server"); 

  if (!manager.init())
    Serial.println("init failed");  
  
  s76g.setFrequency(frequency);

  // You can change the moduation speed etc from the default
  //s76g.setModemConfig(RH_RF95::Bw125Cr45Sf128);
  //s76g.setModemConfig(RH_RF95::Bw125Cr45Sf2048);

  // The default transmitter power is 13dBm, using PA_BOOST.
  // You can set transmitter powers from 2 to 20 dBm:
  s76g.setTxPower(3);

  myCipher.setKey(encryptkey, sizeof(encryptkey));
}

void loop() { 
  if (manager.available()){
    //Serial.println("manager.available()");
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    uint8_t from, to;    
    if (manager.recvfrom(buf, &len, &from, &to)) {

      //RH_L0RA::printBuffer("request: ", buf, len);
      Serial.print("got request: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(s76g.lastRssi(), DEC);

      // Send a reply
      uint8_t data[] = "And hello back to you";
      manager.sendto(data, sizeof(data), CLIENT_ADDRESS);
      s76g.waitPacketSent();
      Serial.println("Sent a reply");
    } else {
      Serial.println("recv failed");
    }
  } else {
    Serial.println("no manager available");
  }
}
