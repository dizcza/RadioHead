// abz_encrypted_datagram_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create an addressed unreliable messageing server with encrypted communications
// with the RHDatagram class, using the RH_ABZ driver to control a SX1276 radio in Murata CMWX1ZZABZ module.
// In order for this to compile you MUST uncomment the #define RH_ENABLE_ENCRYPTION_MODULE line
// at the bottom of RadioHead.h, AND you MUST have installed the Crypto directory from arduinolibs:
// http://rweather.github.io/arduinolibs/index.html
//  Philippe.Rochat'at'gmail.com
//  06.07.2017
// It is designed to work with the other example abz_encrypted_datagram_client_DISC
// Tested with ST Discovery B-L072Z-LRWAN1, Arduino 1.8.12, GrumpyOldPizza Arduino Core for STM32L0.

#include <SPI.h>
#include <RH_ABZ.h>
#include <RHEncryptedDriver.h>
#include <Speck.h>
#include <RHDatagram.h>

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// Singleton instance of the radio driver
RH_ABZ abz;
Speck myCipher;   // Instanciate a Speck block ciphering
RHEncryptedDriver encryptedLoRa(abz, myCipher); // Instantiate the driver with those two

// Class to manage message delivery and receipt, using the driver declared above
RHDatagram manager(encryptedLoRa, SERVER_ADDRESS);

float frequency = 868.0; // Change the frequency here. 
unsigned char encryptkey[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}; // The very secret key

void setup() 
{  
  pinMode(PIN_LED3, OUTPUT);  //RED  
  pinMode(PIN_LED,  OUTPUT);  //GREEN
  pinMode(PIN_LED2, OUTPUT);  //BLUE

  Serial.begin(9600);
  while (!Serial) ;
  Serial.println("ABZ Encrypted Server"); 

  if (!manager.init())
    Serial.println("init failed");  
  
  // On the EcoNode SmartTrap board, the radio TCXO is always powered, so 
  // you can tell the radio to use the TCXO like this/. For other boards, you may need 
  // to enable the power to the TCXO before telling the radio to use it
  SX1276SetBoardTcxo(true);

  abz.setFrequency(frequency);

  // You can change the moduation speed etc from the default
//  abz.setModemConfig(RH_RF95::Bw125Cr45Sf128);
  //abz.setModemConfig(RH_RF95::Bw125Cr45Sf2048);

  // The default transmitter power is 13dBm, using PA_BOOST.
  // You can set transmitter powers from 2 to 20 dBm:
  abz.setTxPower(3); // Max power

  myCipher.setKey(encryptkey, sizeof(encryptkey));
}

void loop()
{
  digitalWrite(PIN_LED3, 0);  //R
  digitalWrite(PIN_LED,  0);  //G
  digitalWrite(PIN_LED2, 1);  //B
  
  if (manager.available()){
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    uint8_t from, to;    
    if (manager.recvfrom(buf, &len, &from, &to)) {
       digitalWrite(PIN_LED, 1);
//      RH_ABZ::printBuffer("request: ", buf, len);
      Serial.print("got request: ");
      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println(abz.lastRssi(), DEC);
      // Send a reply
      uint8_t data[] = "And hello back to you";
      manager.sendto(data, sizeof(data), CLIENT_ADDRESS);
      abz.waitPacketSent();
      Serial.println("Sent a reply");
      digitalWrite(PIN_LED, 1);
    } else {
      Serial.println("recv failed");
      digitalWrite(PIN_LED3, 1);
    }
  } else {
    Serial.println("Manager not available!");
    digitalWrite(PIN_LED3, 1);
  }
  digitalWrite(PIN_LED2, 0);
}
