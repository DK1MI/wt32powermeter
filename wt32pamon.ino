/****************************************************************************************************************************
  Remote PA Monitor - solution to remotely monitor RF power, VSWR and more of QO-100 power amplifiers

  For Ethernet shields using WT32_ETH01 (ESP32 + LAN8720)
  Uses WebServer_WT32_ETH01, a library for the Ethernet LAN8720 in WT32_ETH01 to run WebServer

  Michael Clemens, DK1MI
  Licensed under MIT license

 *****************************************************************************************************************************/

#define DEBUG_ETHERNET_WEBSERVER_PORT       Serial

// Debug Level from 0 to 4
#define _ETHERNET_WEBSERVER_LOGLEVEL_      3

#define FORMAT_SPIFFS_IF_FAILED true


#include <WebServer_WT32_ETH01.h>
#include "index.h"  // Main Web page header file
#include "config.h"  // Config Web page header file
#include <Preferences.h>
#include "FS.h"
#include "SPIFFS.h"

Preferences config;

String config_items [ ] = {"show_mV", "show_dBm", "show_watt", "vswr_threshold", "selected_band"};
String config_defaults [ ] = {"true", "true", "true", "2", "70cm"};
double fwd_array [3300] = {};
double ref_array [3300] = {};

int voltage_fwd,voltage_ref;
double fwd_dbm=0, ref_dbm=0;
double fwd_watt=0, ref_watt=0;
byte iii=0;

String conf_content;
String conf_textareas = "";
String conf_config_table = "";

String band = "70cm";
String default_band = "70cm";
String band_fwd = band + "_fwd";
String band_ref = band + "_ref";
String band_list []= {"1.25cm", "3cm", "6cm", "9cm", "13cm", "23cm", "70cm", "2m", "HF"};

int IO2_FWD = 2;
int IO4_REF = 4;

WebServer server(80);

// Select the IP address according to your local network
//IPAddress myIP(192, 168, 88, 247);
//IPAddress myGW(192, 168, 88, 1);
//IPAddress mySN(255, 255, 255, 0);

// Google DNS Server IP
//IPAddress myDNS(8, 8, 8, 8);

String readFile(fs::FS &fs, const char * path){
   Serial.printf("Reading file: %s\r\n", path);

   File file = fs.open(path);
   if(!file || file.isDirectory()){
       Serial.println("− failed to open file for reading");
       return "";
   }
  String ret = "";
   Serial.println("− read from file:");
   while(file.available()){
      ret+=char(file.read());
   }
   file.close();
   return ret;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
   Serial.printf("Writing file: %s\r\n", path);

   File file = fs.open(path, FILE_WRITE);
   if(!file){
      Serial.println("− failed to open file for writing");
      return;
   }
   if(file.print(message)){
      Serial.println("− file written");
   }else {
      Serial.println("− frite failed");
   }
   file.close();
}


// converts dBm to Watt
double dbm_to_watt(double dbm) {
  return pow( 10.0, (dbm - 30.0) / 10.0);
}

// takes a voltage value and translates it
// to dBm based on the corresponding lookup table
double millivolt_to_dbm(int mv, bool fwd)
{
  double lastval = 0;
  double nextval = 0;
  int lastkey = 0;
  int nextkey = 0;
  double stored_val = 0;

  for (int i=0; i<3300; i++) {
    if (fwd) {
      //stored_val = translation_fwd.getFloat(String(i).c_str());
      stored_val = fwd_array[i];
      if (stored_val != 0){
        Serial.println("key: " + String(i) + " - " + String(stored_val));
      }
      
    } else {
      //stored_val = translation_ref.getFloat(String(i).c_str());
      stored_val = ref_array[i];
    }
    if (stored_val != 0) {
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

  Serial.println("lastkey: "+String(lastkey));
  Serial.println("lastval: "+String(lastval));
 
  double lowerkey = min(lastkey, nextkey);
  double lowerval = min(lastval, nextval);

  double diffkey = max(lastkey, nextkey) - min(lastkey, nextkey);
  double diffval = max(lastval, nextval) - min(lastval, nextval);
  double result = lowerval + ((diffval / diffkey) * (mv - lowerkey));

  //Serial.print("measured voltage: " + String(mv) + "   LastVal: " + String(lastval) + "    LastKey: " + String(lastkey) + "   Nextval: " + String(nextval) + "   NextKey:" + String(nextkey) + "\n");
  return result;
}


// read voltages from both input pins
// calculates avaerage value of 20 measurements
void read_directional_couplers()
{ 
  int voltage_sum_fwd = 0;
  int voltage_sum_ref = 0;
  for(iii=0; iii<50; iii++)                                     // Take 20 samples and save the highest value
  { voltage_sum_fwd += analogReadMilliVolts(IO2_FWD);
    voltage_sum_ref += analogReadMilliVolts(IO4_REF);
  }

  voltage_fwd = voltage_sum_fwd/50;                                         // use peak voltage for processing
  voltage_ref = voltage_sum_ref/50;

  fwd_dbm = millivolt_to_dbm(voltage_fwd, true);
  ref_dbm = millivolt_to_dbm(voltage_ref, false);

  fwd_watt = dbm_to_watt(fwd_dbm);
  ref_watt = dbm_to_watt(ref_dbm);

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

String watt_or_williwatt(double val){
  String ret = "0";
  if (val < 1){
    ret = String(val*1000,0) + "mW";
  } else {
    ret = String(val,3) + "W";
  }
  ret.replace("nan", "---");
  return ret;
}

// executes the function to gather sensor data
// delivers gathered data to dashboard page
// invoked periodically by the dashboard page
void handleDATA() {
  read_directional_couplers();
  // calculate VSWR
  double vswr = (1 + sqrt(ref_watt/fwd_watt)) / (1 - sqrt(ref_watt/fwd_watt));
  String vswr_str = "-1";
  if (vswr >= 1) {
    vswr_str = String(vswr);
  }

  double rl = fwd_dbm - ref_dbm;

  // get vswr_threshold from general config
  String vswr_threshold = config.getString(String("vswr_threshold").c_str());

  String voltage_fwd_str = "";
  String voltage_ref_str = "";
  if (config.getString(String("show_mV").c_str()) != "false") {
    voltage_fwd_str = String(voltage_fwd) + "mV";
    voltage_ref_str = String(voltage_ref) + "mV";
  }

  String fwd_dbm_str = "";
  String ref_dbm_str = "";
  if (config.getString(String("show_dBm").c_str()) != "false") {
    fwd_dbm_str = String(fwd_dbm,3) + "dBm";
    ref_dbm_str = String(ref_dbm,3) + "dBm";
  }

  fwd_dbm_str.replace("nan", "-- ");
  ref_dbm_str.replace("nan", "-- ");
  String rl_str = (String(rl));
  rl_str.replace("nan", "-- ");


  String output = watt_or_williwatt(fwd_watt) + ";" + fwd_dbm_str + ";" + voltage_fwd_str + ";" + watt_or_williwatt(ref_watt) + ";" + ref_dbm_str + ";" + voltage_ref_str + ";" + vswr_str + ";" + rl_str + ";" + band + ";" + String(vswr_threshold);
  server.send(200, "text/plane", output);
}

// main function for displaying the configuration page
// invoked by the "configuration" button on the dashboard page
void handleCONFIG() {


  if (conf_textareas == "") {
    build_textareas();
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
  conf_content +=  conf_textareas;
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
void build_textareas() {
  String fwd = readFile(SPIFFS, String("/" + band + "fwd.txt").c_str());
  String ref = readFile(SPIFFS, String("/" + band + "ref.txt").c_str());

  Serial.println("From disk: "+ fwd);

  clear_fwd_ref_array();

  save_string_to_array(fwd, fwd_array);
  save_string_to_array(ref, ref_array);

  String tbl = "<form action=\"/modtranslation\" method=\"POST\">";
  tbl += "<table class='styled-table'>";
  tbl += "<thead><tr><td>FWD (mV:dBm)</td><td>REF (mV:dBm)</td></tr></thead>";
  tbl += "<tr><td>";
  tbl += "<textarea id='fwd_textarea' name='fwd_textarea' rows='30' cols='25'>";
  for (int i=0; i<sizeof fwd_array/sizeof fwd_array[0]; i++) {
    if (fwd_array[i] != 0){
      tbl += String(i) +":"+String(fwd_array[i],5) + "\n";
    }
  }
  tbl += "</textarea>";
  tbl += "</td><td>";
  tbl += "<textarea id='ref_textarea' name='ref_textarea' rows='30' cols='25'>";
  for (int i=0; i<sizeof ref_array/sizeof ref_array[0]; i++) {
    if (ref_array[i] != 0){
      tbl += String(i) +":"+String(ref_array[i],5) + "\n";
    }
  }
  tbl += "</textarea>";
  tbl += "</td></tr></table>";
  tbl += "<p><button class='button' value='save' name='save' type='submit'>save</button>";
  tbl += "</form>";
  conf_textareas = tbl;
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
// to the XXXX value table for the selected band
void handleMODTRANS() {
  String fwd = server.arg("fwd_textarea") + "\n";
  String ref = server.arg("ref_textarea") + "\n";
  clear_fwd_ref_array();
  save_string_to_array(fwd,fwd_array);
  save_string_to_array(ref,ref_array);


  // OWN FUNCTION
  String fwd_of_array = "";
  for (int i=0; i<sizeof fwd_array/sizeof fwd_array[0]; i++) {
    if (fwd_array[i] != 0){
      fwd_of_array += String(i) +":"+String(fwd_array[i],5) + "\n";
    }
  }
  writeFile(SPIFFS, String("/" + band + "fwd.txt").c_str(), fwd_of_array.c_str());


  String ref_of_array = "";
  for (int i=0; i<sizeof ref_array/sizeof ref_array[0]; i++) {
    if (ref_array[i] != 0){
      ref_of_array += String(i) +":"+String(ref_array[i],5) + "\n";
    }
  }
  writeFile(SPIFFS, String("/" + band + "ref.txt").c_str(), ref_of_array.c_str());

  build_textareas();
  handleCONFIG();
}


void clear_fwd_ref_array(){
  for (int x = 0; x < sizeof(fwd_array) / sizeof(fwd_array[0]); x++)
  {
    fwd_array[x] = 0;
    ref_array[x] = 0;
  }
}

void save_string_to_array(String table_data, double arr []){

  int r=0,t=0;
    
  for(int i=0;i<table_data.length();i++)
  {
    if(table_data[i] == '\n' || i==table_data.length())
    {
      if (i-r > 1)
      {
        //tmp[t] = table_data.substring(r,i);
        Serial.println("row: " + table_data.substring(r,i));
        String row = table_data.substring(r,i);
        t++;

        int r2=0,t2=0;
        for(int j=0;j<row.length();j++)
        {
          if(row[j] == ':' || row[j] == '\n')
          {
            if (j-r2 > 1)
            {
              int key = row.substring(r2,j).toInt();
              double val = row.substring(j+1).toDouble();
              arr[key] = val;
              t2++;
            }
            r2 = (j+1);
          }
        }
      }
      r = (i+1);
    }
      
  }
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
  Serial.println("baaand:" + band);
  config.putString(String("selected_band").c_str(), band);
  build_textareas();
  handleCONFIG();
}

// initialization routine
void setup()
{
  // DELETEME: Cleanup from old ver
  config.remove(String("show_fwd").c_str());
  config.remove(String("show_ref").c_str());
  config.remove(String("show_swr").c_str());

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
  server.on("/modcfg", handleMODCFG);
  server.on("/selectband", handleBAND);
  server.on("/modtranslation", handleMODTRANS);


  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    Serial.println("SPIFFS Mount Failed");
    return;
   }

  server.onNotFound(handleNotFound);
  server.begin();

  Serial.print(F("HTTP EthernetWebServer is @ IP : "));
  Serial.println(ETH.localIP());

  analogReadResolution(12);
  //translation_fwd.begin(band_fwd.c_str(), false);
  //translation_ref.begin(band_ref.c_str(), false);
  config.begin("config", false);

  band = config.getString(String("selected_band").c_str());
  if (band = ""){
    config.putString(String("selected_band").c_str(), default_band);
    band = default_band;
  }

  Serial.println("band: " + band);
  build_textareas();
  Serial.println("done");


}

void loop()
{
  server.handleClient();
}