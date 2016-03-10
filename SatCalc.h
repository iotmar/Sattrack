#ifndef satcalc_H
#define satcalc_H

#define MAX_ITTER 100

double getJulianTime();

///update all predictions
bool predictPasses(){

    LedStrip.SetAnimColor(0x0,0x9f,0x0);
    double jdC = getJulianTime();
    
    #ifdef DEBUG
      int year,mon,day,hr,min;
      double sec;
      
      invjday(jdC,config->timezone,config->daylight, year, mon, day, hr, min, sec);
      Serial.println("Predict time: " + String(day) + '/' + String(mon) + '/' + String(year) + ' ' + String(hr) + ':' + String(min) + ':' + String(sec));
      Serial.println();
    #endif
    
    if ( !sat.initpredpoint( jdC , 0.0 )){
        #ifdef DEBUG
            Serial.println("Can't find initial predpoint");
         #endif
         return false;
    }
    
    bool error;
    for (int i = 0; i<pred_size ; i++){
 
      if ( ! sat.nextpass(&passPredictions[i],MAX_ITTER)){
         #ifdef DEBUG
            Serial.println("Prediction ERROR");
         #endif
         return false;
      }else{
        #ifdef DEBUG
            int year,mon,day,hr,min;
            double sec;
            invjday(passPredictions[i].jdstart ,config->timezone,config->daylight , year, mon, day, hr, min, sec);
            Serial.println("Overpass " + String(day) + ' ' + String(mon) + ' ' + String(year));
            Serial.println("  Start: az=" + String(passPredictions[i].azstart) + "° " + String(hr) + ':' + String(min) + ':' + String(sec));
            invjday(passPredictions[i].jdmax ,config->timezone,config->daylight , year, mon, day, hr, min, sec);
            Serial.println("  Max: elev=" + String(passPredictions[i].maxelevation) + "° " + String(hr) + ':' + String(min) + ':' + String(sec));
            invjday(passPredictions[i].jdstop ,config->timezone,config->daylight , year, mon, day, hr, min, sec);
            Serial.println("  Stop: az=" + String(passPredictions[i].azstop) + "° " + String(hr) + ':' + String(min) + ':' + String(sec));
            switch(passPredictions[i].sight){
                case lighted:
                    Serial.println("  Visible");
                    break;
                case eclipsed:
                    Serial.println("  Eclipsed");
                    break;
                case daylight:
                    Serial.println("  Daylight");
                    break;
            }
        #endif
      }

    }
    return true;
}

bool updatePasses(){

    for (int i = 1; i<pred_size; i++){
        passPredictions[i-1] = passPredictions[i];
    }
    return sat.nextpass(&passPredictions[pred_size-1],MAX_ITTER);
}

bool predictPasses(passinfo passPredictions[pred_size],double jdC, bool direction){

    LedStrip.SetAnimColor(0x0,0x9f,0x0);
    double jdSave = sat.getpredpoint();

    sat.setpredpoint(jdC);
    
    bool error;
    for (int i = 0; i<pred_size ; i++){
      int k;
      
      if (direction){      /////order to fill array
        k = pred_size-1-i;
      }else{
        k = i;
      }
      
      if ( ! sat.nextpass(&passPredictions[k],MAX_ITTER,direction)){
         #ifdef DEBUG
            Serial.println("Prediction ERROR");
         #endif
         sat.setpredpoint(jdSave);
         return false;
      }

    }

    sat.setpredpoint(jdSave);
    return true;
}


#endif
