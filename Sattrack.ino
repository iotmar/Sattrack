
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
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

  pinMode(buttonPin, INPUT);

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
      WiFi.softAP( config->host , config->ap_password);
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
  sat.setsunrise(config->sunoffset);

  //Start MDNS
  #ifndef USE_OTA
  if (MDNS.begin(config->host, WiFi.localIP())){
    #ifdef DEBUG
      Serial.printf("mDNS responder started for %s.local\n", config->host);
      Serial.println();
    #endif
  }
  #endif

  ///get tle and time
  UDPNTPClient.begin(2390);
  if (!dataError){
    bool timeFlag = updateTime();
    bool TleFlag = getTle(config->satnum);
    if (!(timeFlag && TleFlag)){
        #ifdef DEBUG
          Serial.println("Can't get data");
          Serial.println();
        #endif
        LedStrip.SetAnimColor(0x0,0x0,0x0,0x9f,0x0,0x0);
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

  state=IDLE;
}

//////////////////////////////////////
//           Main loop              //
//////////////////////////////////////
void ColorCalc(double jd,double satEl,int16_t satVis);

void loop() { 
  double jd = getJulianTime();
  
  if(!dataError){

    sat.findsat(jd);   //find satellite position on system time
  
    if ( sat.satEl > config->offset ){   ///aboven horizon => display
        if (LedStrip.CanShow()){
          ColorCalc(jd,sat.satEl,sat.satVis);
        }
    }else{  //below horizon
        if (LedStrip.CanShow()){   //clear pixels
          strip.ClearTo(RgbColor(0,0,0));
          strip.Show();
        }
        
        if (passPredictions[0].jdstop <= jd && !predError){  //pred next past
             predError = !updatePasses();
             uint8_t buf[]="n";webSocket.broadcastBIN(buf,1);  //notify clients that there is new data available
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

    if(jd > orbit.lastJd-orbit.step*(orbit_size/2-1)){
        updateOrbit();
        webSocketSendOrbit();
    }

    if (millis() - socketrate > 33)
    {
        socketrate = millis();
        webSocketSendData();
    }
    
  }
  else if(WiFi.getMode() != WIFI_AP){  ///retry when failed getting data
      if (updatejdtime < jd){
          if (!(updateTime() && getTle(config->satnum))){
              #ifdef DEBUG
                Serial.println("Can't get data");
                Serial.println();
              #endif
              updatejdtime = jd + 0.003472;   // retry update in 5 min
          }else{
              dataError = false;
              LedStrip.AnimStop();
          }
      }
  }
  
  ///calculate prediction requests///
  
  if (PredictRequest != NULL){   
      int params = PredictRequest->params();
      if (params > 0 && !predError){   ///check for extra arguments
          
          char *ptr;
          bool err;
          uint32_t unix = strtoul(PredictRequest->getParam(0)->value().c_str(),&ptr,10);  ///read unix time
          double jdC = getJulianFromUnix(unix);
    
          ///recalc new predictions
          LedStrip.AnimStart(ANIM_WAIT);
          
          passinfo Predictions[pred_size];

          #ifdef DEBUG
             Serial.println("Prediction request: " + PredictRequest->getParam(0)->name() + " " + PredictRequest->getParam(0)->value());
          #endif
          
          if (PredictRequest->getParam(0)->name() == "pre"){
              err = !predictPasses(Predictions,jdC, true);
          }else if (PredictRequest->getParam(0)->name() == "next"){
              err = !predictPasses(Predictions,jdC, false);
          }else{
              err = true;   //no correct arguments
          }
  
          LedStrip.AnimStop();
          senddata(PredictRequest,Predictions,err);  /// send new prediction
          
      }else{
        senddata(PredictRequest,passPredictions,false);  /// send normal prediction
      }

      PredictRequest = NULL;
  }
  
  ///statemachine///

  switch(state){
       case RECALC:
          LedStrip.SetAnimColor(0xff,0x66,0x0);
          LedStrip.AnimStart(ANIM_WAIT);
          sat.site(config->lat,config->lon,config->alt);  //set new coordinates
          sat.setsunrise(config->sunoffset);
          if(!getTle(config->satnum, true)){              //get new tle and recalculate overpasses
              LedStrip.SetAnimColor(0x9f,0x0,0x0);
              LedStrip.AnimStart(ANIM_FLASH);
              dataError = true;
              updatejdtime = getJulianTime() + 0.003472;   // retry update in 5 min
          }else{
              LedStrip.AnimStop();
              dataError = false;
          }
          state=IDLE;
          break;
      
      case RESTART:
          closeAllConnections();
          delay(5000);
          ESP.restart();
          state=IDLE;
          break;

       case RESEND:
          uint8_t buf[]="n";
          webSocket.broadcastBIN(buf,1);  //notify clients that there is new data available
          state=IDLE;
          break;
          
  }

  ///OTA and debug////

  #ifdef DEBUG_frame
    framerate +=1;
  #endif

  #ifdef USE_OTA
    ArduinoOTA.handle();
  #endif


  ///resetbutton///
  bool state = digitalRead(buttonPin);
  if (!state && prevButtonState){
    pressTime = millis();
  }
  if (millis() - pressTime > 5000 && !state){  //button pressed for 5 sec
    //save defaultconfigs
    setDefaultConfig();
    EEPROM.getDataPtr();
    EEPROM.commit();

    //small animation
    LedStrip.SetAnimColor(127,0,255);
    LedStrip.AnimStart(ANIM_WAIT);
    delay(2000);
    ESP.restart();
  }
  prevButtonState = state;

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
      Color = RgbColor::LinearBlend(config->ColorDayL, config->ColorDayH, (satEl-config->offset)/(90.0-config->offset));
  }else{   //dark
      RgbColor visible = RgbColor::LinearBlend(config->ColorVisL, config->ColorVisH, (satEl-config->offset)/(90.0-config->offset));
      RgbColor eclipsed = RgbColor::LinearBlend(config->ColorEclL, config->ColorEclH, (satEl-config->offset)/(90.0-config->offset));
      Color = RgbColor::LinearBlend(eclipsed, visible, satVis/1000.0);     
  }

  if (jd <= passPredictions[0].jdstop+0.0001 && jd >= passPredictions[0].jdstart-0.0001 && !predError){
    float brightness;
    if (jd <= passPredictions[0].jdmax){
      brightness = (jd-passPredictions[0].jdstart)/(passPredictions[0].jdmax-passPredictions[0].jdstart);
    }else{
      brightness = (passPredictions[0].jdstop-jd)/(passPredictions[0].jdstop-passPredictions[0].jdmax);
    }
    if (brightness < 0.0){brightness = 0.0;}
    if (brightness > 1.0){brightness = 1.0;}
    Color = RgbColor::LinearBlend(RgbColor(0,0,0), Color, brightness); 
  }

  strip.ClearTo(Color);
  strip.Show();
}

