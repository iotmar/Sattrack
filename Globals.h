#define DEBUG                 //Outputs some information in the serial port
//#define DEBUG_frame         //Outputs every second the heap and looprate
#define USE_OTA               //Use OTA (note: It's unprotected, use with care.)

#define pred_size 10          
#define PIXELS 12             //Numbers of neopixels that is connected to the device
#define buttonPin 0           //Pinnumber for factoryreset button


NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> strip(PIXELS, 2);   //Connect the neopixels to GPIO2
AsyncWebServer server(80);
WebSocketsServer webSocket(81);
Sgp4 sat;
WiFiUDP UDPNTPClient;
WiFiClient client;


unsigned long unixtime;
double jdtime; 
double updatejdtime;
unsigned long timemillis;
unsigned long socketrate = 0;
bool prevButtonState = true;
unsigned long pressTime;

typedef char str3[4];
str3 monstr[13];

passinfo passPredictions[pred_size];

AsyncWebServerRequest *PredictRequest = NULL;

#ifdef DEBUG
  
  Ticker ticker;
  int framerate;

  void Second_Tick()
  {
    Serial.printf("Looprate:%d FreeMem:%d \n",framerate,ESP.getFreeHeap());
    framerate=0;  
  }
#endif

enum state_mode {
    RECALC,
    RESTART,
    IDLE
}state;

/////Flags//////

bool predError = false;
bool dataError = false;
