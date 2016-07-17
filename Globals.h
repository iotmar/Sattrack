//#define DEBUG                 //Outputs some information in the serial port
//#define DEBUG_frame           //Outputs every second the heap and looprate
//#define USE_OTA               //Use OTA (note: It's unprotected, use with care.)

#define pred_size 10 
#define orbit_size 100         
#define PIXELS 12             //Numbers of neopixels that is connected to the device
#define buttonPin 0           //Pinnumber for factoryreset button, default GPIO0. Button to ground + 10k pull up resistor


NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> strip(PIXELS, 2);   //Connect the neopixels to GPIO2, change NeoGrbFeature if needed
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
    RESEND,
    IDLE
}state;

struct orbit_path {
    double lon[orbit_size];
    double lat[orbit_size];
    double lastJd;
    double step;
}orbit;

/////Flags//////

bool predError = false;
bool dataError = false;
