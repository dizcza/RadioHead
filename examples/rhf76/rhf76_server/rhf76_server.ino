// rhf76_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing server
// with the RH_L0RA class. RH_L0RA class does not provide for addressing or
// reliability, so you should only use RH_L0RA directly if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rhf76_client_xx
// Tested with wired RHF76-052 module, Arduino 1.8.13, GrumpyOldPizza Arduino Core for STM32L0.

#include <SPI.h>
#include <RH_L0RA.h>

// Singleton instance of the radio driver
RH_L0RA rhf76;


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
//  while (!Serial) ; 

  if (!rhf76.init())
    Serial.println("init failed");  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  rhf76.setFrequency(868.0);

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
}

void loop()
{
  digitalWrite(PIN_LED_BLU, 1);
  digitalWrite(PIN_LED_GRN, 0);
  digitalWrite(PIN_LED_RED, 0);
  
  if (rhf76.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rhf76.recv(buf, &len))
    {
       digitalWrite(PIN_LED_GRN, 1);

//      RH_L0RA::printBuffer("request: ", buf, len);
      Serial.print("got request: ");
      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println(rhf76.lastRssi(), DEC);

      // Send a reply
      uint8_t data[] = "And hello back to you";
      rhf76.send(data, sizeof(data));
      rhf76.waitPacketSent();
      Serial.println("Sent a reply");
      digitalWrite(PIN_LED_GRN, 0);
    }
    else
    {
      Serial.println("recv failed");
    }
  }
}
