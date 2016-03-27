#ifndef config_h
#define config_h

#include "Pixels.h"

#define CONFIGVERSION 6

void closeAllConnections();
void sendconfig(AsyncWebServerRequest *request);
boolean checkRange(String Value);
void setDefaultConfig();
bool getTle(int ide, bool forceupdate);


//////////////////////////
//  Config definition   //
//////////////////////////

struct ConfigStruct {
    boolean daylight;
    boolean dhcp;
    
    uint8_t  IP[4];
    uint8_t  Netmask[4];
    uint8_t Gateway[4];
    
    double lon;
    double lat;
    double alt;
  
    int8_t timezone;
    uint8_t version;
    uint16_t satnum;

    RgbColor ColorDayL;
    RgbColor ColorDayH;
    RgbColor ColorVisL;
    RgbColor ColorVisH;
    RgbColor ColorEclL;
    RgbColor ColorEclH;
    
    char ntpServerName[128];
    char password[64];
    char ssid[32]; 
};

ConfigStruct* config;

void setDefaultConfig(){
    config->ssid[0] = '\0';
    config->password[0] = '\0';

    strcpy(config->ntpServerName, "0.be.pool.ntp.org");
    config->daylight = true;
    config->timezone = 1;
    
    config->dhcp = false;
    config->IP[0]=192;config->IP[1]=168;config->IP[2]=1;config->IP[3]=100;
    config->Netmask[0]=255;config->Netmask[1]=255;config->Netmask[2]=255;config->Netmask[3]=0;
    config->Gateway[0]=192;config->Gateway[1]=168;config->Gateway[2]=1;config->Gateway[3]=1;

    config->lat = 0.0; 
    config->lon = 0.0;
    config->alt = 0.0;
    config->satnum = 25544;   //ISS

    config->ColorDayL = RgbColor(0,255,0);  //green
    config->ColorDayH = RgbColor(0,0,255);  //blue
    config->ColorVisL = RgbColor(255,255,0);  //yellow
    config->ColorVisH = RgbColor(255,255,255);  //white
    config->ColorEclL = RgbColor(0,0,255);  //blue
    config->ColorEclH = RgbColor(255,0,0);  //red

    config->version = CONFIGVERSION;
}

void initConfig(){

  EEPROM.begin(sizeof(ConfigStruct));

  config = reinterpret_cast<ConfigStruct*>(EEPROM.getDataPtr());
  
  if(config->version != CONFIGVERSION){
      setDefaultConfig();
  }
  
}

////////////////////////////////////
//         Post handlers          //
////////////////////////////////////

RgbColor HexColorToRgb(String hexstring){
  
  long number = strtol( &hexstring[1], NULL, 16);

  // Split them up into r, g, b values
  uint8_t r = number >> 16;
  uint8_t g = number >> 8 & 0xFF;
  uint8_t b = number & 0xFF;

  return RgbColor(r,g,b);
}


bool saveNetworkSettings(AsyncWebServerRequest *request){

    int params = request->params();
    if (params > 0 ){

      bool restart = false;
      bool recalc = false;
      
      int part = request->getParam(0)->value().toInt();
      
      if (part ==  1){
          config->dhcp = false;
      }else if (part == 3){
          config->daylight = false;
      }
      
      for ( uint8_t i = 1; i < params; i++ ) {

          if (part == 1){
                   if (request->getParam(i)->name() == "SSID") {strncpy(config->ssid,request->getParam(i)->value().c_str(),sizeof(ConfigStruct::ssid));state = RESTART;}
              else if (request->getParam(i)->name() == "PSK")  {strncpy(config->password,request->getParam(i)->value().c_str(),sizeof(ConfigStruct::password));}
              else if (request->getParam(i)->name() == "ip_0") {if (checkRange(request->getParam(i)->value()))   config->IP[0] =  (uint8_t)request->getParam(i)->value().toInt();}
              else if (request->getParam(i)->name() == "ip_1") {if (checkRange(request->getParam(i)->value()))   config->IP[1] =  (uint8_t)request->getParam(i)->value().toInt();}
              else if (request->getParam(i)->name() == "ip_2") {if (checkRange(request->getParam(i)->value()))   config->IP[2] =  (uint8_t)request->getParam(i)->value().toInt();}
              else if (request->getParam(i)->name() == "ip_3") {if (checkRange(request->getParam(i)->value()))   config->IP[3] =  (uint8_t)request->getParam(i)->value().toInt();}
              else if (request->getParam(i)->name() == "nm_0") {if (checkRange(request->getParam(i)->value()))   config->Netmask[0] =  (uint8_t)request->getParam(i)->value().toInt();}
              else if (request->getParam(i)->name() == "nm_1") {if (checkRange(request->getParam(i)->value()))   config->Netmask[1] =  (uint8_t)request->getParam(i)->value().toInt();}
              else if (request->getParam(i)->name() == "nm_2") {if (checkRange(request->getParam(i)->value()))   config->Netmask[2] =  (uint8_t)request->getParam(i)->value().toInt();}
              else if (request->getParam(i)->name() == "nm_3") {if (checkRange(request->getParam(i)->value()))   config->Netmask[3] =  (uint8_t)request->getParam(i)->value().toInt();}
              else if (request->getParam(i)->name() == "gw_0") {if (checkRange(request->getParam(i)->value()))   config->Gateway[0] =  (uint8_t)request->getParam(i)->value().toInt();}
              else if (request->getParam(i)->name() == "gw_1") {if (checkRange(request->getParam(i)->value()))   config->Gateway[1] =  (uint8_t)request->getParam(i)->value().toInt();}
              else if (request->getParam(i)->name() == "gw_2") {if (checkRange(request->getParam(i)->value()))   config->Gateway[2] =  (uint8_t)request->getParam(i)->value().toInt();}
              else if (request->getParam(i)->name() == "gw_3") {if (checkRange(request->getParam(i)->value()))   config->Gateway[3] =  (uint8_t)request->getParam(i)->value().toInt();}
              else if (request->getParam(i)->name() == "DHCP") {config->dhcp = true;}
          }

          if (part == 2){
                   if (request->getParam(i)->name() == "lon") {config->lon = atof(request->getParam(i)->value().c_str());state = RECALC;}
              else if (request->getParam(i)->name() == "lat") {config->lat = atof(request->getParam(i)->value().c_str());}
              else if (request->getParam(i)->name() == "alt") {config->alt = atof(request->getParam(i)->value().c_str());}
              else if (request->getParam(i)->name() == "sat") {config->satnum = atoi(request->getParam(i)->value().c_str());}
          }

          if (part == 3){
                   if (request->getParam(i)->name() == "VisL") {config->ColorVisL = HexColorToRgb(request->getParam(i)->value());}
              else if (request->getParam(i)->name() == "VisH") {config->ColorVisH = HexColorToRgb(request->getParam(i)->value());}
              else if (request->getParam(i)->name() == "DayL") {config->ColorDayL = HexColorToRgb(request->getParam(i)->value());}
              else if (request->getParam(i)->name() == "DayH") {config->ColorDayH = HexColorToRgb(request->getParam(i)->value());}
              else if (request->getParam(i)->name() == "EclL") {config->ColorEclL = HexColorToRgb(request->getParam(i)->value());}
              else if (request->getParam(i)->name() == "EclH") {config->ColorEclH = HexColorToRgb(request->getParam(i)->value());}
              else if (request->getParam(i)->name() == "ds") {config->daylight = true;}
              else if (request->getParam(i)->name() == "ts") {strncpy(config->ntpServerName,request->getParam(i)->value().c_str(),sizeof(ConfigStruct::ntpServerName));}
              else if (request->getParam(i)->name() == "tz") {config->timezone =  (int8_t)request->getParam(i)->value().toInt();}
          }
      }

      EEPROM.getDataPtr();
      EEPROM.commit();

      return true;
      
    }else{
       return false;
    }
    
}



boolean checkRange(String Value)
{
   if (Value.toInt() < 0 || Value.toInt() > 255)
   {
     return false;
   }
   else
   {
     return true;
   }
}



#endif
