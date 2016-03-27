#ifndef pixels_HH
#define pixels_HH

#include <NeoPixelBus.h>

enum anim_mode {
    ANIM_STOP,
    ANIM_WAIT,
    ANIM_FLASH
};

class Animo {
    Ticker tick;
    RgbColor fcolor;
    RgbColor bcolor;
    NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod>* bus;
    anim_mode status;
    unsigned int frame;

  public:

    void Begin(NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod>* s){bus = s;status = ANIM_STOP;};
  
    void Animation();

    void SetAnimColor(uint8_t rf,uint8_t gf,uint8_t bf,uint8_t rb,uint8_t gb,uint8_t bb);
    void SetAnimColor(uint8_t rf,uint8_t gf,uint8_t bf);

    void AnimStop();
    void AnimStart(anim_mode m);

    bool CanShow();
    
} LedStrip;

void animation(){   //dirty hack for ticker
    LedStrip.Animation();
}


void Animo::SetAnimColor(uint8_t rf,uint8_t gf,uint8_t bf,uint8_t rb,uint8_t gb,uint8_t bb){
    fcolor = RgbColor(rf,gf,bf);
    bcolor = RgbColor(rb,gb,bb);
}

void Animo::SetAnimColor(uint8_t rf,uint8_t gf,uint8_t bf){
    SetAnimColor(rf,gf,bf,0,0,0);
}

void Animo::AnimStop(){
    tick.detach();
    status = ANIM_STOP;
    bus->ClearTo(RgbColor(0,0,0));
    bus->Show();
}

void Animo::AnimStart(anim_mode m){
    
    switch (m){
        case ANIM_STOP:
          AnimStop();
          break;

        case ANIM_WAIT:
          frame = 0;
          if(status!=ANIM_WAIT){tick.detach();tick.attach_ms(100,animation);}
          break;

        case ANIM_FLASH:
          frame = 0;
          if(status!=ANIM_FLASH){tick.detach();tick.attach_ms(500,animation);}
 
    }
    status = m;
}

void Animo::Animation(){

    switch (status){
      
        case ANIM_STOP:
          break;

        case ANIM_WAIT:
          bus->ClearTo(bcolor);
          bus->SetPixelColor(frame, fcolor);
          frame = (frame+1)%PIXELS;
          bus->SetPixelColor(frame, fcolor);
          bus->Show();
      
          break;

        case ANIM_FLASH:
          if (frame){
              frame = 0;
              bus->ClearTo(bcolor);
          }else{
              frame = 1;
              bus->ClearTo(fcolor);
          }
          bus->Show();
          break;
   }  
}

bool Animo::CanShow(){
    if (status == ANIM_STOP){
      return bus->CanShow();
    }else{
      return false;
    }
}

#endif

