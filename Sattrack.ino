#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <Sgp4.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <NeoPixelBus.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include "FS.h"
#include "Globals.h"
#include "Config.h"

#include "SatCalc.h"
#include "Webpage.h"
#include "Clients.h"
#include "Pixels.h"



///////////////////////////////////
//            Setup              //
///////////////////////////////////

void setup() {

  initConfig();
  
  strip.Begin();
  LedStrip.Begin(&strip);
  
  LedStrip.SetAnimColor(0x9f,0x0,0x0);
  LedStrip.AnimStart(ANIM_WAIT);
  
  SPIFFS.begin();

  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println();
  #endif

  ///Start wifi
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(config->ssid,config->password);
  if (!config->dhcp){
    WiFi.config(IPAddress(config->IP),IPAddress(config->Gateway),IPAddress(config->Netmask));
  }

  ///Connect to wifi
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout <= 15 ) {
      delay(500);
      timeout += 1;
      #ifdef DEBUG
        Serial.print(".");
      #endif
  }

  ///Start AP on timeout
  if (timeout > 15){
      WiFi.mode(WIFI_AP);
      WiFi.softAP( host , ap_password);
      LedStrip.SetAnimColor(0,0,0,0x0,0x0,0x9f);
      LedStrip.AnimStart(ANIM_WAIT);
      dataError = true;
  }else{
      LedStrip.SetAnimColor(0xff,0x66,0x0);
  }
  
  #ifdef DEBUG
    Serial.println();
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();
  #endif

  ///Set site coordinates
  sat.site(config->lat,config->lon,config->alt);

  //Start MDNS
  #ifndef USE_OTA
  if (MDNS.begin(host, WiFi.localIP())){
    #ifdef DEBUG
      Serial.printf("mDNS responder started for %s.local\n", host);
      Serial.println();
    #endif
  }
  #endif
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);

  ///get tle and time
  UDPNTPClient.begin(2390);
  if (!dataError){
    if (!(updateTime() && getTle(config->satnum))){
        #ifdef DEBUG
          Serial.println("Can't get data");
          Serial.println();
        #endif
        LedStrip.SetAnimColor(0x9f,0x0,0x0, 0x0,0x0,0x0);
        LedStrip.AnimStart(ANIM_WAIT);
        dataError = true;
        updatejdtime = getJulianTime() + 0.000694;   // retry update in 1 min
    }
  }

  ///initilize server,websocket,ota
  initServer();
  initWebsocket();
  initMonths();
  #ifdef USE_OTA
    initOTA();
  #endif

  if (!dataError) {LedStrip.AnimStop();}


  ///Start framerate ticker
  #ifdef DEBUG_frame
    ticker.attach(1,Second_Tick);
    framerate = 0;
  #endif
 
}

//////////////////////////////////////
//           Main loop              //
//////////////////////////////////////
void ColorCalc(double jd,double satEl,int16_t satVis);

void loop() { 
  double jd = getJulianTime();
  
  if(!dataError){
    webSocket.loop();
    sat.findsat(jd);   //find satellite position on system time
  
    if ( sat.satEl > 0.0 ){   ///aboven horizon => display
        if (LedStrip.CanShow()){
          ColorCalc(jd,sat.satEl,sat.satVis);
        }
    }else{  //below horizon
        if (LedStrip.CanShow()){   //clear pixels
          strip.ClearTo(RgbColor(0,0,0));
          strip.Show();
        }
        
        if (passPredictions[0].jdstop <= jd && predError){  //pred next past
             predError = updatePasses();
             uint8_t buf[]="n";webSocket.broadcastBIN(buf,1);
        }
  
        if ( updatejdtime < jd){  ///update tle and time
            if (passPredictions[0].jdstart - 0.5/sat.revpday > jd || passPredictions[0].jdstop + 0.1/sat.revpday < jd){
                if (millis() > 2592000000){
                    ESP.restart();
                }
                updateTime();
                getTle(config->satnum); 
            }
        }
    }
    #ifndef FREERUN
    if (millis() - socketrate > 33)
    #endif
    {
        socketrate = millis();
        webSocketSendData();
    }
    webSocket.loop();
    
  }
  else if(WiFi.getMode() != WIFI_AP){  ///retry when failed getting data
      if (updatejdtime < jd){
          if (!(updateTime() && getTle(config->satnum))){
              #ifdef DEBUG
                Serial.println("Can't get data");
                Serial.println();
              #endif
              LedStrip.SetAnimColor(0x9f,0x0,0x0);
              LedStrip.AnimStart(ANIM_WAIT);
              dataError = true;
              updatejdtime = jd + 0.000694;   // retry update in 1 min
          }else{
              dataError = false;
              LedStrip.AnimStop();
          }
      }
  }
  
  server.handleClient();
  

  #ifdef DEBUG_frame
    framerate +=1;
  #endif
  #ifdef DEBUG
    Serial.flush();
    if (Serial.available()){
      Serial.read();
      uint8_t buf[]="n";webSocket.broadcastBIN(buf,1);
    }
  #endif
  #ifdef USE_OTA
    ArduinoOTA.handle();
  #endif

}

//////////////////////////////////////
//           Functions              //
//////////////////////////////////////


double getJulianTime(){ 
    return jdtime + (millis()-timemillis)/86400000.0;
}


void ColorCalc(double jd,double satEl,int16_t satVis){
  
  RgbColor Color;
  if (satVis == -1){    ///daylight
      Color = RgbColor::LinearBlend(config->ColorDayL, config->ColorDayH, satEl/90.0);
  }else{   //dark
      RgbColor visible = RgbColor::LinearBlend(config->ColorVisL, config->ColorVisH, satEl/90.0);
      RgbColor eclipsed = RgbColor::LinearBlend(config->ColorEclL, config->ColorEclH, satEl/90.0);
      Color = RgbColor::LinearBlend(eclipsed, visible, satVis/1000.0);     
  }

  if (jd <= passPredictions[0].jdstop+0.0001 && jd >= passPredictions[0].jdstart-0.0001 && predError){
    float brightness;
    if (jd <= passPredictions[0].jdmax){
      brightness = (jd-passPredictions[0].jdstart)/(passPredictions[0].jdmax-passPredictions[0].jdstart);
    }else{
      brightness = (passPredictions[0].jdstop-jd)/(passPredictions[0].jdstop-passPredictions[0].jdmax);
    }
    Color = RgbColor::LinearBlend(RgbColor(0,0,0), Color, brightness); 
  }

  strip.ClearTo(Color);
  strip.Show();
}

