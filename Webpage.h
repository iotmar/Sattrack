#ifndef webpage_h
#define webpage_h

#include "FS.h"

extern AsyncWebServer server;

//Concatinate char arrays and items
class Stringbuffer {
     size_t BUFFER_SIZE;
     char* buff;
  public:
    Stringbuffer (size_t size){BUFFER_SIZE = size; buff = new char[size]; buff[0] = '\0';buff[size-1] = '\0';}
    ~Stringbuffer(){delete[] buff;}
    void add(const char* array){ strncat(buff, array, BUFFER_SIZE - strlen(buff) - 1); }
    void add(int i){ char buffer [12];itoa (i,buffer,10);strncat(buff, buffer, BUFFER_SIZE - strlen(buff) - 1); }
    void add(unsigned long i){char buffer [11];ultoa(i,buffer,10);strncat(buff, buffer, BUFFER_SIZE - strlen(buff) - 1); }
    void addTime (int hr, int min, double sec){ if( hr > 99 && min > 99 && sec > 99.0){return;}
                                                char timebuff [9] = ""; char numberbuff [9] = "";
                                                itoa (hr,numberbuff,10);strcat(timebuff, numberbuff);
                                                strcat(timebuff, ":");
                                                itoa (min,numberbuff,10);if(numberbuff[1]=='\0'){strcat(timebuff, "0");};strcat(timebuff, numberbuff);
                                                strcat(timebuff, ":");
                                                int isec=(int)(sec+0.5);itoa (isec,numberbuff,10);if(numberbuff[1]=='\0'){strcat(timebuff, "0");};strcat(timebuff, numberbuff);
                                                strncat(buff, timebuff, BUFFER_SIZE - strlen(buff) - 1);
                                               }
    void add(double i){String cast = String(i);strncat(buff, cast.c_str(), BUFFER_SIZE - strlen(buff) - 1);}
    void add(double i,unsigned char prec){char buffer [11];dtostrf(i,10,prec,buffer);String cast = String(buffer);cast.trim();strncat(buff,cast.c_str(), BUFFER_SIZE - strlen(buff) - 1);}
    void addColor(RgbColor c){ String i="#";
                               i+=String(c.R>>4,HEX);i+=String(c.R & 0xf,HEX);
                               i+=String(c.G>>4,HEX);i+=String(c.G & 0xf,HEX);
                               i+=String(c.B>>4,HEX);i+=String(c.B & 0xf,HEX);
                               strncat(buff, i.c_str(),BUFFER_SIZE - strlen(buff) - 1);
                             }

    char* getPointer(){return buff;}
};

void closeAllConnections(){
   //server.close();
   webSocket.disconnect();
   WiFiUDP::stopAll();
   WiFiClient::stopAll();
}

/////////////////////////////////
//        Websocket            //
/////////////////////////////////

void webSocketSendData(){

    int year,mon,day,hr,min,secc;
    double sec;
    invjday(sat.satJd ,config->timezone,config->daylight , year, mon, day, hr, min, sec);
    secc = sec;

    uint8_t* buffer = new uint8_t[sizeof(double)*7+sizeof(int16_t)+sizeof(int)*3+1];
    
    buffer[0]='p';
    memcpy(&buffer[1],&sat.satLon,sizeof(double));
    memcpy(&buffer[sizeof(double)+1],&sat.satLat,sizeof(double));
    memcpy(&buffer[sizeof(double)*2+1],&sat.satAlt,sizeof(double));
    memcpy(&buffer[sizeof(double)*3+1],&sat.satAz,sizeof(double));
    memcpy(&buffer[sizeof(double)*4+1],&sat.satEl,sizeof(double));
    memcpy(&buffer[sizeof(double)*5+1],&sat.satDist,sizeof(double));
    memcpy(&buffer[sizeof(double)*6+1],&sat.satVis,sizeof(int16_t));
    memcpy(&buffer[sizeof(double)*6+sizeof(int16_t)+1],&hr,sizeof(int));
    memcpy(&buffer[sizeof(double)*6+sizeof(int16_t)+sizeof(int)+1],&min,sizeof(int));
    memcpy(&buffer[sizeof(double)*6+sizeof(int16_t)+2*sizeof(int)+1],&secc,sizeof(int));

    webSocket.broadcastBIN(buffer, sizeof(double)*7+sizeof(int16_t)+sizeof(int)*3+1);
    delete[] buffer;
}

void webSocketSendOrbit(uint8_t num){
    uint8_t* buffer = new uint8_t[sizeof(double)*orbit_size*2+2];
    buffer[0] = 'o';
    buffer[1] = orbit_size;
    memcpy(buffer+2,&orbit,sizeof(double)*orbit_size*2);
    if (num == 255){
      webSocket.broadcastBIN(buffer,sizeof(double)*orbit_size*2+2);
    }else{
      webSocket.sendBIN(num,buffer,sizeof(double)*orbit_size*2+2);
    }
    delete[] buffer;
}

void webSocketSendOrbit(){
  webSocketSendOrbit(255);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
            #ifdef DEBUG
                Serial.printf("[%u] Disconnected!\n", num);
            #endif
            break;
            
        case WStype_CONNECTED:
            #ifdef DEBUG
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
                if (dataError){
                    webSocket.disconnect();
                }else{
                    webSocketSendOrbit(num);
                }
            #endif
            break;
            
    }

}

/////////////////////////////////
//        Data handlers        //
/////////////////////////////////

////send data///

void senddata(AsyncWebServerRequest *request,passinfo* Predictions, bool err){

    int year,mon,day,hr,min;
    double sec;
    
    Stringbuffer buf( 12 + 128*pred_size + 2*(80+12) + (12+18) + (9+25) + (2*23) + 3*15);
    
    if (!(predError || err)){
        buf.add("tab|pass|18|");
        buf.add(pred_size);
        
        for (int i = 0; i < pred_size; i++){
          
          invjday(Predictions[i].jdstart ,config->timezone,config->daylight , year, mon, day, hr, min, sec);
          buf.add("|");buf.add(day);buf.add(" ");buf.add(monstr[mon]);                                            //day
          buf.add("|");buf.addTime(hr,min,sec);                                                                   //starttime
          buf.add("|");buf.add(Predictions[i].azstart);                                                           //start azimuth
          
          invjday(Predictions[i].jdmax ,config->timezone,config->daylight , year, mon, day, hr, min, sec);
          buf.add("°|");buf.addTime(hr,min,sec);                                                                  //maxtime
          buf.add("|");buf.add(Predictions[i].maxelevation);                                                      //max elevation
          
          invjday(Predictions[i].jdstop ,config->timezone,config->daylight , year, mon, day, hr, min, sec);
          buf.add("°|");buf.addTime(hr,min,sec);                                                                  //stoptime
          buf.add("|");buf.add(Predictions[i].azstop);                                                            //stop azimuth
          
          switch(Predictions[i].sight){                                                                           //visibility
                case lighted:
                    buf.add("°|Visible");
                    break;
                case eclipsed:
                    buf.add("°|Eclipsed");
                    break;
                case daylight:
                    buf.add("°|Daylight");
                    break;
          }

          buf.add("|");buf.add((int)Predictions[i].transit);                                                      //transit type
          if ( Predictions[i].transit != none){
            invjday(Predictions[i].jdtransit ,config->timezone,config->daylight , year, mon, day, hr, min, sec);
            buf.add("|");buf.addTime(hr,min,sec);                                                                 //transistion time
            buf.add("|");buf.add(Predictions[i].aztransit);                                                       //trans azimuth
            buf.add("|");buf.add(Predictions[i].transitelevation);                                                //trans elevation
            if (Predictions[i].jdtransit<Predictions[i].jdmax){                                                   //sorting
              buf.add("|0|");
            }else{
              buf.add("|1|");
            }
          }else{
            buf.add("|-|-|-|-|");                                                                                                             
          }

          buf.add(Predictions[i].azmax);                                                                          //maxiumum azimuth
          buf.add("|");buf.add((int)Predictions[i].visstart);                                                     //visibility start
          buf.add("|");buf.add((int)Predictions[i].visstop);                                                      //visibility stop
          buf.add("|");buf.add((int)Predictions[i].vismax);                                                       //visibility max
          buf.add("|");buf.add((int)Predictions[i].vistransit);                                                   //visibility transit
          
          
        }
        buf.add("\ninput|pre|");buf.add(getUnixFromJulian(Predictions[0].jdmax));
        buf.add("\ninput|next|");buf.add(getUnixFromJulian(Predictions[pred_size-1].jdmax));
    }else{
        buf.add("tab|pass|1|1|Error");
        buf.add("\ninput|pre|0");
        buf.add("\ninput|next|0");
    }
    buf.add("\ndiv|line1|");buf.add(sat.line1);
    buf.add("\ndiv|line2|");buf.add(sat.line2);
    invjday(sat.satrec.jdsatepoch ,config->timezone,config->daylight , year, mon, day, hr, min, sec);
    buf.add("\ndiv|epoch|");buf.add(day);buf.add("/");buf.add(mon);buf.add("/");buf.add(year);buf.add(" ");buf.addTime(hr,min,sec);
    buf.add("\ndiv|sat|");buf.add(sat.satName);
    
    buf.add("\ndiv|off|");buf.add(config->offset);
    buf.add("\ndiv|clat|");buf.add(config->lat);
    buf.add("\ndiv|clon|");buf.add(config->lon);
    
    request->send ( 200, "text/plain", buf.getPointer());

    #ifdef DEBUG
      Serial.println("Send /data");
    #endif
   
}

///Handle request for data///

void checkdata(AsyncWebServerRequest *request){
    if (dataError){
        request->send ( 400, "text/html", "Page not Found" );
    }else{
        int params = request->params();
        if (params > 0 && !predError){   ///check for extra arguments => request prediction
          
            if (PredictRequest == NULL){
                PredictRequest = request;    //calculation will be handeld in the main loop
            }else{   //already an calculation busy
                request->send(503);
            }
            
        }else{
            senddata(request, passPredictions, false); //send normal passPredictions
        }
    }
}

///Send config settings///

void sendconfig(AsyncWebServerRequest *request){

    if ( WiFi.getMode() != WIFI_AP){
      if ( !request->authenticate(config->host,config->ap_password)){
        request->requestAuthentication();
        return;
      }
    }
    
    Stringbuffer buf(43*3 + 75 + 15*12 + 23*2 + 16*4 + 19*6 + 138 + 13 + 16*3 +  20);
    
    buf.add("input|SSID|");buf.add(config->ssid);
    buf.add("\ninput|HOST|");buf.add(config->host);
    buf.add("\ninput|PASS|");buf.add(config->ap_password);
    buf.add("\ninput|PSK|");buf.add(config->password);

    buf.add("\ninput|ip_0|");buf.add(config->IP[0]);
    buf.add("\ninput|ip_1|");buf.add(config->IP[1]);
    buf.add("\ninput|ip_2|");buf.add(config->IP[2]);
    buf.add("\ninput|ip_3|");buf.add(config->IP[3]);
    buf.add("\ninput|nm_0|");buf.add(config->Netmask[0]);
    buf.add("\ninput|nm_1|");buf.add(config->Netmask[1]);
    buf.add("\ninput|nm_2|");buf.add(config->Netmask[2]);
    buf.add("\ninput|nm_3|");buf.add(config->Netmask[3]);
    buf.add("\ninput|gw_0|");buf.add(config->Gateway[0]);
    buf.add("\ninput|gw_1|");buf.add(config->Gateway[1]);
    buf.add("\ninput|gw_2|");buf.add(config->Gateway[2]);
    buf.add("\ninput|gw_3|");buf.add(config->Gateway[3]);

    buf.add("\ninput|lat|");buf.add(config->lat,4);
    buf.add("\ninput|lon|");buf.add(config->lon,4);
    buf.add("\ninput|alt|");buf.add(config->alt);
    buf.add("\ninput|sat|");buf.add(config->satnum);
    buf.add("\ninput|off|");buf.add(config->offset);
    buf.add("\ninput|sun|");buf.add(config->sunoffset);

    buf.add("\ninput|VisL|");buf.addColor(config->ColorVisL);
    buf.add("\ninput|VisH|");buf.addColor(config->ColorVisH);
    buf.add("\ninput|EclL|");buf.addColor(config->ColorEclL);
    buf.add("\ninput|EclH|");buf.addColor(config->ColorEclH);
    buf.add("\ninput|DayL|");buf.addColor(config->ColorDayL);
    buf.add("\ninput|DayH|");buf.addColor(config->ColorDayH);

    buf.add("\ninput|ts|");buf.add(config->ntpServerName);
    buf.add("\ninput|tz|");buf.add(config->timezone);
    
    buf.add("\nchk|ds|");buf.add((config->daylight ? "checked" : ""));
    buf.add("\nchk|DHCP|");buf.add((config->dhcp ? "checked" : ""));
    
    request->send ( 200, "text/plain", buf.getPointer() );

    #ifdef DEBUG
      Serial.println("Send /config");
    #endif
}


////////////////////////////////////
//      Init functions            //
////////////////////////////////////


void initServer(){

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/home.html","text/html");});
    server.on("/home.html", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/home.html","text/html");});
    server.on("/links.html", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/links.html","text/html");});
    server.on("/about.html", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/about.html","text/html");});
    server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/w3.css","text/css");});
    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/favicon.ico","image/ico");});
    server.on("/map.jpg", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/map.jpg","image/jpg");});
    server.on("/microajax.js", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/microajax.js","text/javascript");});
    server.on("/sunlight.js", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/sunlight.js","text/javascript");});  
    server.on("/config", HTTP_ANY, sendconfig);
    server.on("/data", HTTP_ANY, checkdata);
    server.on("/site.appcache", HTTP_GET,[](AsyncWebServerRequest *request)
        { AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/site.appcache","text/cache-manifest");
          response->addHeader("Cache-Control","max-age=86400");
          request->send(response);
        });
 
    server.on("/settings.html", [](AsyncWebServerRequest *request){

        if ( WiFi.getMode() != WIFI_AP){
          if ( !request->authenticate(config->host,config->ap_password)){
            request->requestAuthentication();
            return;
          }
        }

        saveNetworkSettings(request);
        request->send(SPIFFS, "/settings.html","text/html");
 
        
    });
    server.onNotFound([](AsyncWebServerRequest *request){request->send(404);});
   
    server.begin();
    
}

void initWebsocket(){
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void initMonths(){
 
    strcpy(monstr[1], "Jan");
    strcpy(monstr[2], "Feb");
    strcpy(monstr[3], "Mar");
    strcpy(monstr[4], "Apr");
    strcpy(monstr[5], "May");
    strcpy(monstr[6], "Jun");
    strcpy(monstr[7], "Jul");
    strcpy(monstr[8], "Aug");
    strcpy(monstr[9], "Sep");
    strcpy(monstr[10], "Oct");
    strcpy(monstr[11], "Nov");
    strcpy(monstr[12], "Dec");

}

void AnimStop();

void initOTA(){
  
    ArduinoOTA.onStart([]() {
      #ifdef DEBUG
        Serial.println("OTA Start");
      #endif
      #ifdef DEBUG_frame
        ticker.detach();
      #endif
      
      closeAllConnections();
      
      LedStrip.AnimStop();
      
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
       int pix = (progress / (total / PIXELS));
       RgbColor color = RgbColor(127,0,255);
       strip.SetPixelColor(pix, color);
       strip.Show();
    });
    
    
    
    ArduinoOTA.onError([](ota_error_t error) {
      #ifdef DEBUG
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
      #endif

      LedStrip.SetAnimColor(0x9f,0x0,0x0);
      LedStrip.AnimStart(ANIM_FLASH);

      delay(3000);
      ESP.restart();
      
    });

    #ifdef DEBUG
      ArduinoOTA.onEnd([]() {
        Serial.println("OTA End");
      });
    #endif
    
    ArduinoOTA.setHostname(config->host);
    ArduinoOTA.begin();
}

#endif

