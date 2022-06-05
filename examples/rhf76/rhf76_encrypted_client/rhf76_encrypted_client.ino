// rhf76_encrypted_client.pde
// -*- mode: C++ -*-
// LoRa Simple Hello World Client with encrypted communications 
// In order for this to compile you MUST uncomment the #define RH_ENABLE_ENCRYPTION_MODULE line
// at the bottom of RadioHead.h, AND you MUST have installed the Crypto directory from arduinolibs:
// http://rweather.github.io/arduinolibs/index.html
//  Philippe.Rochat'at'gmail.com
//  06.07.2017
// It is designed to work with the other example rhf76_encrypted_server_xx
// Tested with wired RHF76-052 module, Arduino 1.8.13, GrumpyOldPizza Arduino Core for STM32L0.

#include <SPI.h>
#include <RH_L0RA.h>
#include <RHEncryptedDriver.h>
#include <Speck.h>

RH_L0RA rhf76;
Speck myCipher;   // Instanciate a Speck block ciphering
RHEncryptedDriver myDriver(rhf76, myCipher); // Instantiate the driver with those two

float frequency = 868.0; // Change the frequency here. 
unsigned char encryptkey[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}; // The very secret key


void setup()
{
  pinMode(PIN_LED_GRN, OUTPUT);
  pinMode(PIN_LED_BLU, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);

  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  Serial.println("RHF76-052 Encrypted Client");
  
  if (!rhf76.init())
    Serial.println("init failed");

  rhf76.setFrequency(frequency);

  // You can change the moduation speed etc from the default
  //rhf76.setModemConfig(RH_RF95::Bw125Cr45Sf128);
  //rhf76.setModemConfig(RH_RF95::Bw125Cr45Sf2048);

  // NOTE! The default transmitter power is 13dBm, using PA_BOOST.
  
  // In HF (868/915MHz) band RHF76-052 uses transmitter RFO_HF pin
  // and not the PA_BOOST pin so you have to configure the output
  // power from -1 to 14 dBm and with useRFO true. 
  // Failure to do that will result in extremely low transmit powers.
  rhf76.setTxPower(14, true);

  // If you are using RHF76-052 in LF band you have to choose PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  //rhf76.setTxPower(23, false);
    
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
  // Send a message to rhf76_server
  uint8_t data[] = "Hello World!";
  myDriver.send(data, sizeof(data));
  rhf76.waitPacketSent();
  
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

// You might need a longer timeout for slow modulatiuon schemes and/or long messages
  if (rhf76.waitAvailableTimeout(3000)) { 
    // Should be a reply message for us now   
    if (myDriver.recv(buf, &len)){
      Serial.print("got reply: ");
      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println(rhf76.lastRssi(), DEC);    
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
  delay(400);
}
