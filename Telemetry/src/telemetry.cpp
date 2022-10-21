/**
 * @file telemetry.cpp
 * @author Chris Uustal
 * @brief NFR Telemetry firmware client and server code
 * @version 1
 * @date 2022-10-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/********** INCLUDES **********/
#include "telemetry.h"

/********** VARIABLES **********/
RH_RF95 rf95(RFM95_CS, RFM95_INT);
int16_t packetnum = 0;
bool rfm95_init_successful = true;

/********** PUBLIC FUNCTION DEFINITIONS **********/

/**
 * @brief Runs all setup functions for the RFM95 module 
 * @return true if setup was successful
 * @return false if setup was unsuccessful
 */
bool telemetry_setup()
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (rf95.init() == true) 
  {
    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (rf95.setFrequency(RF95_FREQ) == true) {
      // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

      // The default transmitter power is 13dBm, using PA_BOOST.
      // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
      // you can set transmitter powers from 5 to 23 dBm:
      rf95.setTxPower(23, false);
    }
    else
    {
      // Serial.println("setFrequency failed");
      rfm95_init_successful = false;
    }
  }
  else
  {
    // Serial.println("LoRa radio init failed");
    rfm95_init_successful = false;
  }

  return rfm95_init_successful;
}

/**
 * @brief 
 * 
 */
void client_task()
{
  if (rfm95_init_successful == true)
  {
    // Serial.println("Sending to rf95_server");
    // Send a message to rf95_server
    
    char radiopacket[20] = "Hello World #      ";
    itoa(packetnum++, radiopacket+13, 10);
    Serial.print("Sending "); 
    Serial.println(radiopacket);
    radiopacket[19] = 0;
    
    // Serial.println("Sending..."); 
    delay(10);
    rf95.send((uint8_t *)radiopacket, 20);

    // Serial.println("Waiting for packet to complete..."); 
    delay(10);
    rf95.waitPacketSent();
    // Now wait for a reply
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    Serial.println("Waiting for reply..."); 
    delay(10);
    if (rf95.waitAvailableTimeout(1000))
    { 
      // Should be a reply message for us now   
      if (rf95.recv(buf, &len))
      {
        Serial.print("Got reply: ");
        Serial.println((char*)buf);
        Serial.print("RSSI: ");
        Serial.println(rf95.lastRssi(), DEC);    
      }
      else
      {
        Serial.println("Receive failed");
      }
    }
    else
    {
      Serial.println("No reply, is there a listener around?");
    }
  }
}

/**
 * @brief 
 * 
 */
void server_task()
{
  if (rf95.available() && (rfm95_init_successful == true))
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
    if (rf95.recv(buf, &len))
    {
      RH_RF95::printBuffer("Received: ", buf, len);
      Serial.print("Got: ");
      Serial.println((char*)buf);
       Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
      
      // Send a reply
      uint8_t data[] = "And hello back to you";
      // Delay responding--otherwise you can respond too fast and it doesn't hear you
      delay(10);
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.println("Sent a reply");
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
}