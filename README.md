# Remote Power/SWR Meter

This WT32/ESP32 based project, combined with a directional coupler setup, allows you to remotely monitor the output power and SWR of your station via a web browser.

To achieve this, it reads two voltages which are supplied by the drectional couplers. From these, the respective power is calculated with the help of a table to be created by the user.

# Warning / Disclaimer

This software contains no security mechanisms at all. There is no input/output sanitization/validation in place. Furthermore there is no authentication or authorization mechanism implemented. Any person/system inside the network is able to access the application, read and modify configuration items, gather information about monitored devices.

DO NOT MAKE THE APPLICATION PUBLICLY AVAILABLE! DO NOT EXPOSE THIS TO THE INTERNET.

If you feel uncomfortable using the software as iz is, you are more than welcome to contribute, improve, submit PRs etc.

# Documentation

You can find the documentation here: [https://dk1mi.radio/remote-power-amplifier-monitor/](https://dk1mi.radio/remote-power-amplifier-monitor/)