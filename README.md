# Sattrack
 Sattrack is a small device that sits on your desk, and warn you when the Iss is overflying. It does this by using the Simplified perturbations model (SGP4) for calculating the current position of the satellite. This code was original made for the ESP8266 and use of the [arduino-core](https://github.com/esp8266/Arduino). Later it was moved to PlatformIO. 
 
 The projects original information can be found here:
 Follow this project on [hackaday.io](https://hackaday.io/project/12607-sattrack-iss-indicator).

 In the mids of 2019 I started my coding adventure, I wanted to learn to write code for Microcontrollers / IOT devices. After some blinking tutorials, who don't teach you much, I thought it would be nice to track the International Space Station. While searching
 for information / tutorials I stumbled on Sattrack and I really liked it. 
 So I was really dissapointed when it would not compile at all. The logs showed that some code was depricated and more fun stuff.
 Since I did like this project more than others, I started slowly working on it in the hope to get it running again. At the moment
 it works, but there is more work to be done. There are still some warning messages during compiling who I want to clean up.
 For now I have a unit working and running and I have to wait until the ISS passes over... :-)
 -iotmar-

# Hardware
- ESP8266
- Neopixel ring connected to GPIO2


# External libraries
- [Sgp4-Library](https://github.com/Hopperpop/Sgp4-Library)
- [arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets) by Links2004
- [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) by Makuna
- [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP) by me-no-dev
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) by me-no-dev

# Installation instructions
- Install platformIO.
- Compile and upload the code to your ESP.
- Upload the data to SPIFFS.
- Wait until it opens an AP (indicated by a blue spinning wheel), and connect to "Sattrack" with password "123456789". (This can be   changed in the settings.)
- Go to [sattrack.local/settings.html](http://sattrack.local/settings.html) and change network settings. Save and restart.
- Connect to your own AP and go back to the setting page. It will ask you to login.
- Use again "Sattrack" with password "123456789", and change your site coordinates and other settings.
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

# Credits
- [Sunlight World Map](https://github.com/GChristensen/sunligth-world-map-gadget) by GChristensen
- Worldmap by NASA Earth Observatory
- Hopperpop