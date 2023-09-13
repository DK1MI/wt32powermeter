# Remote Power/SWR Meter

This WT32/ESP32 based project, combined with a directional coupler setup, allows you to remotely monitor the output power and SWR of your station via a web browser.

To achieve this, it reads two voltages which are supplied by the drectional couplers. From these, the respective power is calculated with the help of a table to be created by the user.

# Changes

## v1.0.2

  * added rudimentary PEP implementation for testing
    * can be enabled/disabled via config page
  * added temperature reading 
    * requires DS18B20 sensor on pin IO14
    * display of temperature can be enabled/disabled via config page
    * display in Celsius or Fahrenheit can be toggled via config page

# Warning / Disclaimer

This software contains no security mechanisms at all. There is no input/output sanitization/validation in place. Furthermore there is no authentication or authorization mechanism implemented. Any person/system inside the network is able to access the application, read and modify configuration items, gather information about monitored devices.

DO NOT MAKE THE APPLICATION PUBLICLY AVAILABLE! DO NOT EXPOSE THIS TO THE INTERNET.

If you feel uncomfortable using the software as iz is, you are more than welcome to contribute, improve, submit PRs etc.

# Download

[https://codeberg.org/mclemens/wt32powermeter/archive/main.zip](https://codeberg.org/mclemens/wt32powermeter/archive/main.zip)

# Documentation

You can find the documentation here: [https://dk1mi.radio/remote-power-amplifier-monitor/](https://dk1mi.radio/remote-power-amplifier-monitor/)