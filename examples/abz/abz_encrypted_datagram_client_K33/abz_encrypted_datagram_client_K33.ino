// abz_encrypted_datagram_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create an addressed unreliable messageing client with encrypted communications
// with the RHDatagram class, using the RH_ABZ driver to control a SX1276 radio in Murata CMWX1ZZABZ module.
// In order for this to compile you MUST uncomment the #define RH_ENABLE_ENCRYPTION_MODULE line
// at the bottom of RadioHead.h, AND you MUST have installed the Crypto directory from arduinolibs:
// http://rweather.github.io/arduinolibs/index.html
//  Philippe.Rochat'at'gmail.com
//  06.07.2017
// It is designed to work with the other example abz_encrypted_datagram_server_xx
// Tested with K33 custom board, Arduino 1.8.13, GrumpyOldPizza Arduino Core for STM32L0.

#include <SPI.h>
#include <RH_ABZ.h>
#include <RHEncryptedDriver.h>
#include <Speck.h>
#include <RHDatagram.h>

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

RH_ABZ abz;
Speck myCipher;   // Instanciate a Speck block ciphering
RHEncryptedDriver encryptedLoRa(abz, myCipher); // Instantiate the driver with those two

// Class to manage message delivery and receipt, using the driver declared above
RHDatagram manager(encryptedLoRa, CLIENT_ADDRESS);

float frequency = 868.0; // Change the frequency here. 
unsigned char encryptkey[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}; // The very secret key


void setup()
{
  pinMode(PIN_LED_GRN, OUTPUT);
  pinMode(PIN_LED_BLU, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);

  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  Serial.println("ABZ Encrypted Client");

  if (!manager.init())
    Serial.println("init failed");

  // On the K33 board, the radio TCXO is powered by MCU output PH1, so you have
  // to enable the power to the TCXO before telling the radio to use it
  SX1276SetBoardTcxo(true);
  
  abz.setFrequency(frequency);
  abz.setTxPower(3);
  // You can change the moduation speed etc from the default
//  abz.setModemConfig(RH_RF95::Bw125Cr45Sf128);
  //abz.setModemConfig(RH_RF95::Bw125Cr45Sf2048);
    
  myCipher.setKey(encryptkey, sizeof(encryptkey));
  
  Serial.println("Waiting for radio to setup");
  delay(1000);
  Serial.println("Setup completed");
  
}

void loop()
{
  digitalWrite(PIN_LED_BLU, 1);
  digitalWrite(PIN_LED_GRN, 0);
  digitalWrite(PIN_LED_RED, 0);
  
  Serial.println("Sending to rf95_server");
  // Send a message to rf95_server
  uint8_t data[] = "Hello World!";
  manager.sendto(data, sizeof(data), SERVER_ADDRESS);
  manager.waitPacketSent();
  
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  uint8_t from, to; 
// You might need a longer timeout for slow modulatiuon schemes and/or long messages
  if (abz.waitAvailableTimeout(3000)) { 
    // Should be a reply message for us now   
    if (manager.recvfrom(buf, &len, &from, &to)){
      Serial.print("got reply: ");
      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println(abz.lastRssi(), DEC);    
      digitalWrite(PIN_LED_GRN, 1);
    } else {
      Serial.println("recv failed");  
      digitalWrite(PIN_LED_RED, 1);
    }
  } else {
    Serial.println("No reply, is rf95_server running?");
      digitalWrite(PIN_LED_RED, 1);
  }
  digitalWrite(PIN_LED_BLU, 0);
  delay(2000);
}
