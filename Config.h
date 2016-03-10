#ifndef config_h
#define config_h

#include "Pixels.h"

#define CONFIGVERSION 6

void closeAllConnections();
void sendconfig();
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


bool saveNetworkSettings(){

    if (server.args() > 0 ){

      bool restart = false;
      bool recalc = false;
      
      int part = server.arg(0).toInt();
      
      if (part ==  1){
          config->dhcp = false;
      }else if (part == 3){
          config->daylight = false;
      }
      
      for ( uint8_t i = 1; i < server.args(); i++ ) {

          if (part == 1){
                   if (server.argName(i) == "SSID") {strncpy(config->ssid,server.arg(i).c_str(),sizeof(ConfigStruct::ssid));restart = true;}
              else if (server.argName(i) == "PSK")  {strncpy(config->password,server.arg(i).c_str(),sizeof(ConfigStruct::password));}
              else if (server.argName(i) == "ip_0") {if (checkRange(server.arg(i)))   config->IP[0] =  (uint8_t)server.arg(i).toInt();}
              else if (server.argName(i) == "ip_1") {if (checkRange(server.arg(i)))   config->IP[1] =  (uint8_t)server.arg(i).toInt();}
              else if (server.argName(i) == "ip_2") {if (checkRange(server.arg(i)))   config->IP[2] =  (uint8_t)server.arg(i).toInt();}
              else if (server.argName(i) == "ip_3") {if (checkRange(server.arg(i)))   config->IP[3] =  (uint8_t)server.arg(i).toInt();}
              else if (server.argName(i) == "nm_0") {if (checkRange(server.arg(i)))   config->Netmask[0] =  (uint8_t)server.arg(i).toInt();}
              else if (server.argName(i) == "nm_1") {if (checkRange(server.arg(i)))   config->Netmask[1] =  (uint8_t)server.arg(i).toInt();}
              else if (server.argName(i) == "nm_2") {if (checkRange(server.arg(i)))   config->Netmask[2] =  (uint8_t)server.arg(i).toInt();}
              else if (server.argName(i) == "nm_3") {if (checkRange(server.arg(i)))   config->Netmask[3] =  (uint8_t)server.arg(i).toInt();}
              else if (server.argName(i) == "gw_0") {if (checkRange(server.arg(i)))   config->Gateway[0] =  (uint8_t)server.arg(i).toInt();}
              else if (server.argName(i) == "gw_1") {if (checkRange(server.arg(i)))   config->Gateway[1] =  (uint8_t)server.arg(i).toInt();}
              else if (server.argName(i) == "gw_2") {if (checkRange(server.arg(i)))   config->Gateway[2] =  (uint8_t)server.arg(i).toInt();}
              else if (server.argName(i) == "gw_3") {if (checkRange(server.arg(i)))   config->Gateway[3] =  (uint8_t)server.arg(i).toInt();}
              else if (server.argName(i) == "DHCP") {config->dhcp = true;}
          }

          if (part == 2){
                   if (server.argName(i) == "lon") {config->lon = atof(server.arg(i).c_str());recalc = true;}
              else if (server.argName(i) == "lat") {config->lat = atof(server.arg(i).c_str());}
              else if (server.argName(i) == "alt") {config->alt = atof(server.arg(i).c_str());}
              else if (server.argName(i) == "sat") {config->satnum = atoi(server.arg(i).c_str());}
          }

          if (part == 3){
                   if (server.argName(i) == "VisL") {config->ColorVisL = HexColorToRgb(server.arg(i));}
              else if (server.argName(i) == "VisH") {config->ColorVisH = HexColorToRgb(server.arg(i));}
              else if (server.argName(i) == "DayL") {config->ColorDayL = HexColorToRgb(server.arg(i));}
              else if (server.argName(i) == "DayH") {config->ColorDayH = HexColorToRgb(server.arg(i));}
              else if (server.argName(i) == "EclL") {config->ColorEclL = HexColorToRgb(server.arg(i));}
              else if (server.argName(i) == "EclH") {config->ColorEclH = HexColorToRgb(server.arg(i));}
              else if (server.argName(i) == "ds") {config->daylight = true;}
              else if (server.argName(i) == "ts") {strncpy(config->ntpServerName,server.arg(i).c_str(),sizeof(ConfigStruct::ntpServerName));}
              else if (server.argName(i) == "tz") {config->timezone =  (int8_t)server.arg(i).toInt();}
          }
      }
      
      if(EEPROM.commit()){
          #ifdef DEBUG
            Serial.println("Config saved to EEPROM");
          #endif
      }

      if(recalc){
          LedStrip.SetAnimColor(0xff,0x66,0x0);
          LedStrip.AnimStart(ANIM_WAIT);
          
          sat.site(config->lat,config->lon,config->alt);  //set new coordinates
          getTle(config->satnum, true);                   //get new tle and recalculate overpasses

          LedStrip.AnimStop();
      }
      
      if(restart){
          closeAllConnections();
          ESP.restart();
      }
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
