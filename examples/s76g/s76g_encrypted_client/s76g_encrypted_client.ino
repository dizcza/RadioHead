// LoRa Simple Hello World Client with encrypted communications 
// In order for this to compile you MUST uncomment the #define RH_ENABLE_ENCRYPTION_MODULE line
// at the bottom of RadioHead.h, AND you MUST have installed the Crypto directory from arduinolibs:
// http://rweather.github.io/arduinolibs/index.html
//  Philippe.Rochat'at'gmail.com
//  06.07.2017
// It is designed to work with the other example s76g_encrypted_server
// Tested with LILYGO T-Impulse Wristband, Arduino 1.8.13, GrumpyOldPizza Arduino Core for STM32L0.

#include <SPI.h>
#include <RH_L0RA.h>
#include <RHEncryptedDriver.h>
#include <Speck.h>

RH_L0RA s76g;
Speck myCipher;   // Instanciate a Speck block ciphering
RHEncryptedDriver myDriver(s76g, myCipher); // Instantiate the driver with those two

float frequency = 868.0; // Change the frequency here. 
unsigned char encryptkey[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}; // The very secret key


void setup() {
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  Serial.println("S76G Encrypted Client");
  
  if (!s76g.init())
    Serial.println("init failed");

  s76g.setFrequency(frequency);

  // You can change the moduation speed etc from the default
  //s76g.setModemConfig(RH_RF95::Bw125Cr45Sf128);
  //s76g.setModemConfig(RH_RF95::Bw125Cr45Sf2048);

  s76g.setTxPower(3);
    
  myCipher.setKey(encryptkey, sizeof(encryptkey));
  
  Serial.println("Waiting for radio to setup");
  delay(1000);
  Serial.println("Setup completed");
}

void loop() { 
  Serial.println("Sending to rf95_server");
  // Send a message to s76_encrypted_server
  uint8_t data[] = "Hello World!";
  myDriver.send(data, sizeof(data));
  s76g.waitPacketSent();
  
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

// You might need a longer timeout for slow modulatiuon schemes and/or long messages
  if (s76g.waitAvailableTimeout(3000)) { 
    // Should be a reply message for us now   
    if (myDriver.recv(buf, &len)){
      Serial.print("got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(s76g.lastRssi(), DEC);    
    } else {
      Serial.println("recv failed");  
    }
  } else {
    Serial.println("No reply, is s76_encrypted_server running?");
  }
  delay(400);
}
