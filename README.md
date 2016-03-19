This is an experimental branch that use asynchronic TCP.

# Sattrack
Sattrack is a small device that sits on your desk, and warn you when the Iss is overflying. It does this by using the Simplified perturbations model (SGP4) for calculating the current position of the satellite. This code is made for the ESP8266 and use the [arduino-core](https://github.com/esp8266/Arduino).

# Hardware
- ESP8266-01
- Voltage regulator: LM1117 3.3V
- Logic level converter (perhaps unnecessary)
- Neopixel ring connected to GPIO2
- Resistors,capacitors,...

# External libraries
- [Sgp4-Library](https://github.com/Hopperpop/Sgp4-Library)
- [arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets) by Links2004 (note: Enable AsyncTCP)
- [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) by Makuna
- [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP) by me-no-dev
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) by me-no-dev

# Installation instructions
- Install the arduino-core for the ESP8266 (recommended version: staging 2.1.0-rc1).
- Install the external libraries.
- Change CPU frequency to 160 MHz and use minimum 64k SPIFFS.
- Compile and upload the code to your ESP.
- Upload the data to SPIFFS (see [here](https://github.com/esp8266/Arduino/blob/master/doc/filesystem.md) for more information).
- Wait until it opens an AP (indicated by a blue spinning wheel), and connect to "Sattrack" with password "12345". (This can be changed in "Globals.h".)
- Go to [sattrack.local/settings.html](http://sattrack.local/settings.html) and change network settings. Save and restart.
- Connect to your own AP and go back to the setting page. It will ask you to login.
- Use again "Sattrack" with password "12345", and change your site coordinates and other settings.
- Wait until the ISS is overhead.
- Wave at the astronauts!

# Status indicators
This device use the neopixels to show some status information.
- On startup
  - Two red turning pixels: Trying to connect to acces point.
  - Two orange turning pixels: Trying to get time and TLE.
  - Two green turning pixels: Calculating overpasses.
- When something fails at startup
  - Blue turning wheel: Couldn't connect to network and opens an AP.
  - Red turning wheel: Error getting time or TLE. It will retry in some minutes.

