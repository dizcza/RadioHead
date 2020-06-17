// abz_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_ABZ class. RH_ABZ class does not provide for addressing or
// reliability, so you should only use RH_ABZ directly if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example abz_server_xx
// Tested with K33 custom board, Arduino 1.8.13, GrumpyOldPizza Arduino Core for STM32L0.

#include <SPI.h>
#include <RH_ABZ.h>

// Singleton instance of the radio driver
RH_ABZ abz;


void setup() 
{
  pinMode(PIN_LED_GRN, OUTPUT);
  pinMode(PIN_LED_BLU, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  
  Serial.begin(9600);
  // Wait for serial port to be available 
  // If you do this, it will block here until a USB serial connection is made.
  // If not, it will continue without a Serial connection, but DFU mode will not be available
  // to the host without resetting the CPU with the Boot button
  if(!Serial) ; 

  if (!abz.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // On the K33 board, the radio TCXO is powered by MCU output PH1, so you have
  // to enable the power to the TCXO before telling the radio to use it
  SX1276SetBoardTcxo(true);

  abz.setFrequency(868.0);

  // You can change the moduation speed etc from the default
//  abz.setModemConfig(RH_RF95::Bw125Cr45Sf128);
  //abz.setModemConfig(RH_RF95::Bw125Cr45Sf2048);
  
  // The default transmitter power is 13dBm, using PA_BOOST.
  // You can set transmitter powers from 2 to 20 dBm:
  //abz.setTxPower(20); // Max power
}

void loop()
{
  digitalWrite(PIN_LED_BLU, 1);
  digitalWrite(PIN_LED_GRN, 0);
  digitalWrite(PIN_LED_RED, 0);
  
  Serial.println("Sending to rf95_server");
  // Send a message to rf95_server
  uint8_t data[] = "Hello World!";
  abz.send(data, sizeof(data));
  abz.waitPacketSent();
  
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

// You might need a longer timeout for slow modulatiuon schemes and/or long messages
  if (abz.waitAvailableTimeout(3000))
  { 
    // Should be a reply message for us now   
    if (abz.recv(buf, &len))
   {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println(abz.lastRssi(), DEC);    
      digitalWrite(PIN_LED_GRN, 1);

    }
    else
    {
      Serial.println("recv failed");  
      digitalWrite(PIN_LED_RED, 1);

    }
  }
  else
  {
    Serial.println("No reply, is rf95_server running?");
      digitalWrite(PIN_LED_RED, 1);
  }
  digitalWrite(PIN_LED_BLU, 0);

  delay(400);
}
