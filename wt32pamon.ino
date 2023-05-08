/****************************************************************************************************************************
  Remote PA Monitor - solution to remotely monitor RF power and SWR of amateur radio power amplifiers

  For Ethernet shields using WT32_ETH01 (ESP32 + LAN8720)
  Uses WebServer_WT32_ETH01, a library for the Ethernet LAN8720 in WT32_ETH01 to run WebServer

  Copyright 2023 by Michael Clemens, DK1MI
  Licensed under MIT license (see LICENSE)

 *****************************************************************************************************************************/

#define DEBUG_ETHERNET_WEBSERVER_PORT       Serial

// Debug Level from 0 to 4
#define _ETHERNET_WEBSERVER_LOGLEVEL_      3

#include <WebServer_WT32_ETH01.h>
#include "index.h"  // Main Web page header file
#include "config.h"  // Config Web page header file
#include <Preferences.h>

Preferences translation_fwd;
Preferences translation_ref;
Preferences config;

String config_items [ ] = {"show_fwd", "show_ref", "show_swr", "show_mV", "show_dBm", "show_watt"};
String config_defaults [ ] = {"true", "true", "true", "true", "false", "true"};

int voltage_fwd,voltage_ref;
float fwd_dbm=0, ref_dbm=0;
double fwd_watt=0, ref_watt=0;
byte iii=0;

String conf_content;
String conf_translate_fwd_table = "";
String conf_translate_ref_table = "";
String conf_config_table = "";
String del_action = "";

String band = "70cm";
String band_fwd = band + "_fwd";
String band_ref = band + "_ref";
String band_list []= {"3cm", "13cm", "70cm", "2m"};

int IO2_FWD = 2;
int IO4_REF = 4;

WebServer server(80);

// Select the IP address according to your local network
//IPAddress myIP(192, 168, 88, 247);
//IPAddress myGW(192, 168, 88, 1);
//IPAddress mySN(255, 255, 255, 0);

// Google DNS Server IP
//IPAddress myDNS(8, 8, 8, 8);


// converts dBm to Watt
double dbm_to_watt(float dbm) {
  return pow( 10.0, (dbm - 30.0) / 10.0);
}

// takes a voltage value and translates it
// to dBm based on the corresponding lookup table
float millivolt_to_dbm(int mv, bool fwd)
{
  float lastval = 0;
  float nextval = 0;
  int lastkey = 0;
  int nextkey = 0;
  float stored_val =0;
  for (int i=0; i<3400; i++) {
    if (fwd) {
      stored_val = translation_fwd.getFloat(String(i).c_str(), 0);
    } else {
      stored_val = translation_ref.getFloat(String(i).c_str(), 0);
    }
    if (stored_val > 0) {
      if (i <  mv) {
        lastval = stored_val;
        lastkey = i;
      } else {
        nextval = stored_val;
        nextkey = i;
        break;
      }
    }
  }
 
  float lowerkey = min(lastkey, nextkey);
  float lowerval = min(lastval, nextval);
   /*
  float higherkey = max(lastkey, nextkey);
  
  float higherval = max(lastval, nextval);
  float diffkey = higherkey - lowerkey;
  float diffval = higherval - lowerval;
  float x = diffval / diffkey;
  float y = mv - lowerkey;
  float z = x * y;
  
  float result = lowerval + z;
  */
  float diffkey = max(lastkey, nextkey) - min(lastkey, nextkey);
  float diffval = max(lastval, nextval) - min(lastval, nextval);
  float result = lowerval + ((diffval / diffkey) * (mv - lowerkey));

  //Serial.print("measured voltage: " + String(mv) + "   LastVal: " + String(lastval) + "    LastKey: " + String(lastkey) + "   Nextval: " + String(nextval) + "   NextKey:" + String(nextkey) + "\n");
  return result;
}


// read voltages from both input pins
// calculates avaerage value of 20 measurements
void read_directional_couplers()
{ 
  int voltage_sum_fwd = 0;
  int voltage_sum_ref = 0;
  for(iii=0; iii<20; iii++)                                     // Take 20 samples and save the highest value
  { voltage_sum_fwd += analogReadMilliVolts(IO2_FWD);
    voltage_sum_ref += analogReadMilliVolts(IO4_REF);
    //Serial.println(String(voltage_fwd));
    //if(voltage_fwd > voltage_fwd_peak) voltage_fwd_peak = voltage_fwd;    // safe the peak of 10 measurements
    //if(voltage_ref > voltage_ref_peak) voltage_ref_peak = voltage_ref;  
  }
  //voltage_fwd = voltage_fwd_peak;                                         // use peak voltage for processing
  //voltage_ref = voltage_ref_peak;

  voltage_fwd = voltage_sum_fwd/20;                                         // use peak voltage for processing
  voltage_ref = voltage_sum_ref/20;

  fwd_dbm = millivolt_to_dbm(voltage_fwd, true);
  ref_dbm = millivolt_to_dbm(voltage_ref, false);

  fwd_watt = dbm_to_watt(fwd_dbm);
  ref_watt = dbm_to_watt(ref_dbm);

  Serial.print(String(fwd_watt) + "\n");
  
  //voltage_fwd_peak = 0;                                                      // set peak voltages back to 0
  //voltage_ref_peak = 0;

}

// delivers the dashboard page in "index.h"
void handleRoot()
{
  String s = MAIN_page;
  server.send(200, "text/html", s);
}


// delivers a 404 page if a non-existant resouurce is requested
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


// executes the function to gather sensor data
// delivers gathered data to dashboard page
// invoked periodically by the dashboard page
void handleDATA() {
  read_directional_couplers();

  // calculate SWR
  float swr = (1 + sqrt(ref_watt/fwd_watt)) / (1 - sqrt(ref_watt/fwd_watt));

  String output = String(fwd_watt,3) + ";" + String(fwd_dbm,3) + ";" + String(voltage_fwd) + ";" + String(ref_watt,3) + ";" + String(ref_dbm,3) + ";" + String(voltage_ref) + ";" + String(swr) + ";" + band;
  server.send(200, "text/plane", output);
}

// main function for displaying the configuration page
// invoked by the "configuration" button on the dashboard page
void handleCONFIG() {
  if (conf_translate_fwd_table == "") {
    build_translate_table(true);
  }
  if (conf_translate_ref_table == "") {
    build_translate_table(false);
  }
  if (conf_config_table == "") {
    build_config_table();
  }
  
  conf_content = "<!DOCTYPE HTML>\r\n<html>";
  conf_content += "<style>";
  conf_content += ".styled-table{border-collapse: collapse; margin: 25px 0; font-size: 0.9em; font-family: sans-serif; min-width: 400px; box-shadow: 0 0 20px rgba(0, 0, 0, 0.15);}.styled-table thead tr{background-color: #009879; color: #ffffff; text-align: left;}.styled-table tbody tr{border-bottom: 1px solid #dddddd;}.styled-table tbody tr:nth-of-type(even){background-color: #f3f3f3;}.styled-table tbody tr:last-of-type{border-bottom: 2px solid #009879;}.styled-table tbody tr.active-row{font-weight: bold; color: #009879;}";
  conf_content += ".button{background-color: #009879; border: none; color: white; padding: 5px 5px; text-align: center; text-decoration: none; display: inline-block; margin: 4px 2px; cursor: pointer; border-radius: 8px;}";
  conf_content += "</style>";
  conf_content += "<h1>Configuration</h1>";
  conf_content += "<h3>Band Selection</h1>";
  conf_content += "<form method='POST' action='/selectband'>";
  conf_content += "<label for='bands'></label><select class='button' onchange='this.form.submit()'' id='band' name='bands' size='1'>";
  for (int i=0; i<sizeof band_list/sizeof band_list[0]; i++) {
    String selected = "";
    if (band_list[i] == band) {
      selected = "selected";
    }
    conf_content += "<option value='" + band_list[i] + "' " + selected + " >" + band_list[i] + "</option>";
  }
  conf_content += "</select></form>";
  conf_content += "<p>";
  conf_content += "<h2>Translation Detector voltage /mV to RF-Power level /dBm</h3>";
  conf_content += "<h3>FWD</h3>";
  conf_content +=  conf_translate_fwd_table;
  conf_content += "<p>";
  conf_content += "<h3>REF</h3>";
  conf_content +=  conf_translate_ref_table;
  conf_content += "<p>";
  conf_content += "<h2>General Configuration Items</h3>";
  conf_content += "<p>";
  conf_content += conf_config_table;
  conf_content += "<p><form method='POST' action='/'><button class='button' value='back' name='back' type='submit'>Back to Dashboard</button></form>";
  conf_content += "</html>";
  server.send(200, "text/html", conf_content);
}

// generates the translation table for either the FWD or
// REF values
void build_translate_table(bool fwd) {
  String tbl = "";
  if (fwd) {
    tbl = "<form action=\"/modttfwd\" method=\"POST\">";
  } else {
    tbl = "<form action=\"/modttref\" method=\"POST\">";
  }
  tbl += "<table class='styled-table'>";
  tbl += "<thead><tr><td>millivolt (mV)</td><td>decibel-milliwatts (dBm)</td><td>Watt</td><td>Action</td></tr></thead>";

  for (int i=0; i<3400; i++) {
    float stored_val = 0;
    if (fwd) {
      stored_val = translation_fwd.getFloat(String(i).c_str(), 0);
    } else {
      stored_val = translation_ref.getFloat(String(i).c_str(), 0);
    }
    if (stored_val > 0) {
      tbl += "<tr><td>";
      tbl += String(i);
      tbl += "</td><td>";
      tbl += String(stored_val,3);
      tbl += "</td><td>";
      tbl += String(dbm_to_watt(stored_val),3);
      tbl += "</td><td>";
      tbl += "<button class='button' value='" + String(i) + "' name='delete' type='submit'>delete</button>";
      tbl += "</td></tr>";   
    } 
  }
  tbl += "<tr><td><input name='volt' length=16></td><td><input name='dBm' length=16></td><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td><td><button class='button' type='submit'>add/edit</button></td></tr>";
  tbl += "</table></form>"; 
    if (fwd) {
      conf_translate_fwd_table = tbl;
    } else {
      conf_translate_ref_table = tbl;
    }
}

// generates the table with generic configuration items
void build_config_table() {
  conf_config_table = "<form action=\"/modcfg\" method=\"POST\">";
  conf_config_table += "<table class='styled-table'>";
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
  conf_config_table += "<tr><td><input name='conf_key' length=16></td><td><input name='conf_value' length=16></td><td><button class='button' type='submit'>edit</button></td></tr>";
  conf_config_table += "</table></form>";
  handleCONFIG();
}

// Handle request from the config page to change or add values
// to the FWD value table for the selected band
void handleMODTTFWD() {
  String volt = server.arg("volt");
  String dBm = server.arg("dBm");
  del_action = server.arg("delete");
  if (del_action != "") {
    translation_fwd.remove(del_action.c_str());
  } else if (volt != "" and dBm != "") {
     translation_fwd.putFloat(volt.c_str(), dBm.toFloat());
  }
  build_translate_table(true);
  handleCONFIG();
}

// Handle request from the config page to change or add values
// to the REF value table for the selected band
void handleMODTTREF() {
  String volt = server.arg("volt");
  String dBm = server.arg("dBm");
  del_action = server.arg("delete");
  if (del_action != "") {
    translation_ref.remove(del_action.c_str());
  } else if (volt != "" and dBm != "") {
     translation_ref.putFloat(volt.c_str(), dBm.toFloat());
  }
  build_translate_table(false);
  handleCONFIG();
}

// Handle request from the config page to change or add values
// to the genral config value table for the selected band
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

// changes the band according to the user's selection
// regenerates the translation tables and fills them
// with the values assigned to the respective band
// invoked by selecting a band from the select box of teh config page
void handleBAND() {
  band = server.arg("bands");
  band_fwd = band + "_fwd";
  band_ref = band + "_ref";
  translation_fwd.end();
  translation_ref.end();
  translation_fwd.begin(band_fwd.c_str(), false);
  translation_ref.begin(band_ref.c_str(), false);
  build_translate_table(true);
  build_translate_table(false);
  handleCONFIG();
}

// initialization routine
void setup()
{
  analogReadResolution(12);
  translation_fwd.begin(band_fwd.c_str(), false);
  translation_ref.begin(band_ref.c_str(), false);
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



  //ETH.config(myIP, myGW, mySN, myDNS);

  WT32_ETH01_waitForConnect();

  // activates single web server endpoints
  server.on(F("/"), handleRoot);
  server.on("/readDATA", handleDATA);
  server.on("/config", handleCONFIG);
  server.on("/modttfwd", handleMODTTFWD);
  server.on("/modttref", handleMODTTREF);
  server.on("/modcfg", handleMODCFG);
  server.on("/selectband", handleBAND);


  server.onNotFound(handleNotFound);
  server.begin();

  Serial.print(F("HTTP EthernetWebServer is @ IP : "));
  Serial.println(ETH.localIP());
}

void loop()
{
  server.handleClient();
}