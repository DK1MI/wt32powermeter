/****************************************************************************************************************************
  Remote PA Monitor - solution to remotely monitor RF power, SWR and more of QO-100 power amplifiers

  For Ethernet shields using WT32_ETH01 (ESP32 + LAN8720)
  Uses WebServer_WT32_ETH01, a library for the Ethernet LAN8720 in WT32_ETH01 to run WebServer

  Based on and modified from ESP8266 https://github.com/esp8266/Arduino/releases
  Based on Built by Khoi Hoang https://github.com/khoih-prog/WebServer_WT32_ETH01
  Adapted by Michael Clemens, DK1MI
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
#include "index.h"  // Main Web page header file
#include "config.h"  // Config Web page header file
#include <Preferences.h>

Preferences translation;
Preferences config;

String config_items [ ] = {"show_fwd", "show_ref", "show_swr", "show_mV", "show_dBm", "show_watt"};
String config_defaults [ ] = {"true", "true", "true", "true", "false", "true"};

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

int voltage_fwd,voltage_ref;
int voltage_fwd_peak =0,voltage_ref_peak =0; 
int fwd_power=0, ref_power=0;
byte iii=0;

String conf_content;
String conf_translate_table = "";
String conf_config_table = "";
String del_action = "";

int IO2_FWD = 2;
int IO4_REF = 4;

WebServer server(80);

// Select the IP address according to your local network
IPAddress myIP(192, 168, 88, 247);
IPAddress myGW(192, 168, 88, 1);
IPAddress mySN(255, 255, 255, 0);

// Google DNS Server IP
IPAddress myDNS(8, 8, 8, 8);

int millivolt_to_dbm(int mv)
{
  int last = 0;
  for (int i=0; i<3400; i++) {
    unsigned int stored_val = translation.getUInt(String(i).c_str(), 0);
    //Serial.print(String(stored_val));
    if (stored_val > 0) {
      //Serial.print(String(stored_val));
      if (i <  mv) {
        //Serial.print("bigger");
        last = stored_val;
      } else {
        break;
      }
    }
  }
  return last;
}

void read_directional_couplers()
{   
  for(iii=0; iii<20; iii++)                                     // Take 20 samples and save the highest value
  { voltage_fwd = analogReadMilliVolts(IO2_FWD);
    voltage_ref = analogReadMilliVolts(IO4_REF);
    //Serial.println(String(voltage_fwd));
    if(voltage_fwd > voltage_fwd_peak) voltage_fwd_peak = voltage_fwd;    // safe the peak of 10 measurements
    if(voltage_ref > voltage_ref_peak) voltage_ref_peak = voltage_ref;  
  }
  voltage_fwd = voltage_fwd_peak;                                         // use peak voltage for processing
  voltage_ref = voltage_ref_peak;

  // TODO: dbm in W umrechnen


  fwd_power = millivolt_to_dbm(voltage_fwd);
  ref_power = millivolt_to_dbm(voltage_ref);
  
  voltage_fwd_peak = 0;                                                      // set peak voltages back to 0
  voltage_ref_peak = 0;

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
  //String output = String(voltage_fwd) + "/" + String(fwd_power);
  String output = String(fwd_power);
  server.send(200, "text/plane", output); //Send ADC value only to client ajax request
}

void handleREF() {
  read_directional_couplers();
  //String refValue = String(voltage_ref);
  //String refPower = String(millivolt_to_milliwatt(voltage_ref));
  //String output = String(voltage_ref) + "/" + String(ref_power);
  String output = String(ref_power);
  server.send(200, "text/plane", output); //Send ADC value only to client ajax request
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

void handleCONFIG() {
  if (conf_translate_table == "") {
    build_translate_table();
  }
  if (conf_config_table == "") {
    build_config_table();
  }
  
  conf_content = "<!DOCTYPE HTML>\r\n<html>";
  conf_content += "<h1>Configuration</h1>";
  conf_content += "<p>";
  conf_content += "<h3>Translation from mV to dBm</h3>";
  conf_content +=  conf_translate_table;
  conf_content += "<p>";
  conf_content += "<h3>General Configuration Items</h3>";
  conf_content += "<p>";
  conf_content += conf_config_table;
  conf_content += "</p><form method='POST' action='/'><button class='back' value='back' name='back' type='submit'>Back to Dashboard</button></form>";
  conf_content += "</html>";
  server.send(200, "text/html", conf_content);
}

void build_translate_table() {
  conf_translate_table = "<form action=\"/modtt\" method=\"POST\">";
  conf_translate_table += "<table border=1>";
  conf_translate_table += "<thead><tr><td>millivolt (mV)</td><td>decibel-milliwatts (dBm)</td><td>Action</td></tr></thead>";

  for (int i=0; i<3400; i++) {
    unsigned int stored_val = translation.getUInt(String(i).c_str(), 0);
    if (stored_val > 0) {
      conf_translate_table += "<tr><td>";
      conf_translate_table += String(i);
      conf_translate_table += "</td><td>";
      conf_translate_table += String(stored_val);
      conf_translate_table += "</td><td>";
      conf_translate_table += "<button class='delete' value='" + String(i) + "' name='delete' type='submit'>delete</button>";
      conf_translate_table += "</td></tr>";   
    } 
  }
  conf_translate_table += "<tr><td><input name='volt' length=16></td><td><input name='dBm' length=16></td><td><input type='submit'></td></tr>";
  conf_translate_table += "</table></form>"; 
  handleCONFIG();
}

void build_config_table() {
  conf_config_table = "<form action=\"/modcfg\" method=\"POST\">";
  conf_config_table += "<table border=1>";
  conf_config_table += "<thead><tr><td>Key</td><td>Value</td></td><td>Action</td></tr></thead>";
  for (int i=0; i<sizeof config_items/sizeof config_items[0]; i++) {
    String stored_val = config.getString(config_items[i].c_str(), "xxx");
    if (stored_val == "xxx"){
      config.putString(config_items[i].c_str(), config_defaults[i]);
      stored_val = config.getString(config_items[i].c_str(), "");
    }
    conf_config_table += "<tr><td>";
    conf_config_table += config_items[i];
    conf_config_table += "</td><td>";
    conf_config_table += String(stored_val);
    conf_config_table += "</td><td>";
    conf_config_table += "</td></tr>"; 
  }
  conf_config_table += "<tr><td><input name='conf_key' length=16></td><td><input name='conf_value' length=16></td><td><input type='submit'></td></tr>";
  conf_config_table += "</table></form>";
  handleCONFIG();
}


void handleMODTT() {
  String volt = server.arg("volt");
  String dBm = server.arg("dBm");
  del_action = server.arg("delete");
  if (del_action != "") {
    translation.remove(del_action.c_str());
  } else if (volt != "" and dBm != "") {
     translation.putUInt(volt.c_str(), dBm.toInt());
  }
  build_translate_table();
}

void handleMODCFG() {
  String key = server.arg("conf_key");
  String value = server.arg("conf_value");
  for (int i=0; i<sizeof config_items/sizeof config_items[0]; i++) {
    if (config_items[i] == key) {
      if (key != "" and value != "") {
        config.putString(config_items[i].c_str(), value);
        conf_config_table = "";
        break;
      }
    }
  }
  build_config_table();
}

void setup()
{
  analogReadResolution(12);
  translation.begin("translation", false);
  config.begin("config", false);
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
  server.on("/readSWR", handleSWR);
  server.on("/readTEMP", handleTEMP);
  server.on("/config", handleCONFIG);
  server.on("/modtt", handleMODTT);
  server.on("/modcfg", handleMODCFG);


  server.onNotFound(handleNotFound);
  server.begin();

  Serial.print(F("HTTP EthernetWebServer is @ IP : "));
  Serial.println(ETH.localIP());
}

void loop()
{
  server.handleClient();
}