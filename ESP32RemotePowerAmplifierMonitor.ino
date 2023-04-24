/****************************************************************************************************************************
  Remote PA Monitor - solution to remotely monitor RF power, SWR and more of QO-100 power amplifiers

  For Ethernet shields using WT32_ETH01 (ESP32 + LAN8720)
  Uses WebServer_WT32_ETH01, a library for the Ethernet LAN8720 in WT32_ETH01 to run WebServer

  Based on and modified from ESP8266 https://github.com/esp8266/Arduino/releases
  Based on Built by Khoi Hoang https://github.com/khoih-prog/WebServer_WT32_ETH01
  Adapted by Michael Clemens
  Licensed under MIT license

  Copyright (c) 2015, Majenko Technologies
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

  Neither the name of Majenko Technologies nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************************************************************/

#define DEBUG_ETHERNET_WEBSERVER_PORT       Serial

// Debug Level from 0 to 4
#define _ETHERNET_WEBSERVER_LOGLEVEL_      3
#define NO  0
#define YES 1

#include <WebServer_WT32_ETH01.h>
#include "index.h"  //Web page header file

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
// milliVolt         0  100  200  300  400  500  600  700   800   900  1000  1100  1200  1300  1400  1500  1600  1700  1800  1900  2000  2100  2200  2300  2400  2500  2600   2700   2800   2900   3000   3100   3200   3300   3400   3500   3600
int milliwatt [ ] = {0,  50, 100, 200, 300, 400, 500, 700, 1000, 1000, 1000, 1000, 2000, 2000, 2000, 2000, 3000, 3000, 4000, 4000, 5000, 6000, 6000, 7000, 7000, 8000, 9000, 10000, 11000, 11000, 12000, 13000, 14000, 15000, 17000, 19000, 20000};

int voltage_fwd,voltage_ref, voltage_drv;
int voltage_fwd_peak =0,voltage_ref_peak =0, voltage_drv_peak =0; 
int fwd_power=0, ref_power=0;
byte iii=0;

int IO2_FWD = 2;
int IO4_REF = 4;
int IO12_DRV = 12;

WebServer server(80);

// Select the IP address according to your local network
IPAddress myIP(192, 168, 88, 247);
IPAddress myGW(192, 168, 88, 1);
IPAddress mySN(255, 255, 255, 0);

// Google DNS Server IP
IPAddress myDNS(8, 8, 8, 8);

int millivolt_to_milliwatt(int mv)
{
  int last = 0;
  for (int i=0; i<sizeof milliwatt/sizeof milliwatt[0]; i++) {
    if ((i*100) <  mv) last = milliwatt[i];
    else break;
  }
  return last;
}

void read_directional_couplers()
{   
  for(iii=0; iii<20; iii++)                                     // Take 20 samples and save the highest value
  { voltage_fwd = analogReadMilliVolts(IO2_FWD);
    voltage_ref = analogReadMilliVolts(IO4_REF);
    voltage_drv = analogReadMilliVolts(IO12_DRV);
    //Serial.println(String(voltage_fwd));
    if(voltage_fwd > voltage_fwd_peak) voltage_fwd_peak = voltage_fwd;    // safe the peak of 10 measurements
    if(voltage_ref > voltage_ref_peak) voltage_ref_peak = voltage_ref;  
    if(voltage_drv > voltage_drv_peak) voltage_drv_peak = voltage_drv;
  }
  voltage_fwd = voltage_fwd_peak;                                         // use peak voltage for processing
  voltage_ref = voltage_ref_peak;
  voltage_drv = voltage_drv_peak;

  fwd_power = millivolt_to_milliwatt(voltage_fwd);
  ref_power = millivolt_to_milliwatt(voltage_ref);
  
  voltage_fwd_peak = 0;                                                      // set peak voltages back to 0
  voltage_ref_peak = 0;
  voltage_drv_peak = 0;

}

void handleRoot()
{
  String s = MAIN_page; //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}

void handleNotFound()
{
  String message = F("File Not Found\n\n");

  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? F("GET") : F("POST");
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");

  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, F("text/plain"), message);
}

void handleFWD() {
  read_directional_couplers();
  //String fwdValue = String(voltage_fwd);
  //String fwdPower = String(millivolt_to_milliwatt(voltage_fwd));
  String output = String(voltage_fwd) + "/" + String(fwd_power);
  server.send(200, "text/plane", output); //Send ADC value only to client ajax request
}

void handleREF() {
  read_directional_couplers();
  //String refValue = String(voltage_ref);
  //String refPower = String(millivolt_to_milliwatt(voltage_ref));
  String output = String(voltage_ref) + "/" + String(ref_power);
  server.send(200, "text/plane", output); //Send ADC value only to client ajax request
}

void handleDRV() {
  read_directional_couplers();
  String drvValue = String(voltage_drv);
  server.send(200, "text/plane", drvValue); //Send ADC value only to client ajax request
}

void handleSWR(){
  read_directional_couplers();
  double swr = (1 + sqrt(ref_power/fwd_power)) / (1 - sqrt(ref_power/fwd_power));
  server.send(200, "text/plane", String(swr)); //Send ADC value only to client ajax reques
}

void handleTEMP() {
  int a = (temprature_sens_read() - 32) / 1.8;
  String tempValue = String(a);
  server.send(200, "text/plane", tempValue); //Send ADC value only to client ajax request
}

void setup()
{
  analogReadResolution(12);
  Serial.begin(115200);

  while (!Serial);

  // Using this if Serial debugging is not necessary or not using Serial port
  //while (!Serial && (millis() < 3000));

  Serial.print("\nStarting AdvancedWebServer on " + String(ARDUINO_BOARD));
  Serial.println(" with " + String(SHIELD_TYPE));
  Serial.println(WEBSERVER_WT32_ETH01_VERSION);

  // To be called before ETH.begin()
  WT32_ETH01_onEvent();
  
  ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);
  // Static IP, leave without this line to get IP via DHCP
  ETH.config(myIP, myGW, mySN, myDNS);

  WT32_ETH01_waitForConnect();

  server.on(F("/"), handleRoot);
  server.on("/readFWD", handleFWD);
  server.on("/readREF", handleREF);
  server.on("/readDRV", handleDRV);
  server.on("/readSWR", handleSWR);
  server.on("/readTEMP", handleTEMP);

  server.onNotFound(handleNotFound);
  server.begin();

  Serial.print(F("HTTP EthernetWebServer is @ IP : "));
  Serial.println(ETH.localIP());
}

void loop()
{
  server.handleClient();
}