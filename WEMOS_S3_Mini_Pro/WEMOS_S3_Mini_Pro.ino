/////////////////////////////////////////////////////////////////

#include "FS.h"
#include <SPI.h>
#include "FFat.h"
#include "SPIFFS.h"
// #include <Wire.h>  

/////////////////////////////////////////////////////////////////

#include <Arduino_GFX_Library.h>
#include <Adafruit_NeoPixel.h>
#include <WiFiManager.h> 
#include <ArduinoJson.h>    

#include "Button2.h"

/////////////////////////////////////////////////////////////////

#define FORMAT_FFAT false
#define FORMAT_SPIFFS_IF_FAILED true

/////////////////////////////////////////////////////////////////

 #define LOLIN_S3_Mini_Pro_ST7789   //  ST7789   128x128 0.85 4/2
// #define LOLIN_S3_Mini_Pro_GC9107   //  GC9107   128x128 0.85 4/2
// #define LOLIN_S3_Mini_Pro_GC9A01   //  GC9A01   128x128 0.85 4/2
// #define LOLIN_S3_Mini_Pro_ILI9341    //  GC9A01   128x128 0.85 4/2


#if defined(LOLIN_S3_Mini_Pro_ST7789) || defined(LOLIN_S3_Mini_Pro_GC9107) || defined(LOLIN_S3_Mini_Pro_GC9A01) || defined(LOLIN_S3_Mini_Pro_ILI9341)
  #define BOARD "LOLIN S3 Mini Pro"
  #define LCD_WIDTH 128
  #define LCD_HEIGHT 128
  #define LCD_ROTATION 0
  #define pclk_hz 40000000
  #define reset_gpio_num GFX_NOT_DEFINED 
  #define HAS_IR 
  #define HAS_IMU  

  #include "FastIMU.h"

  #define IMU_ADDRESS 0x18    //Change to the address of the IMU
  #define PERFORM_CALIBRATION //Comment to disable startup calibration
  QMI8658 IMU;               //Change to the name of any supported IMU! 

  calData calib = { 0 };  //Calibration data
  AccelData accelData;    //Sensor data
  GyroData gyroData;
  MagData magData;

  Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC,TFT_CS,SCK,MOSI,MISO);

#endif 

  #define LCD_USES "UNKNOWN"

#if defined(LOLIN_S3_Mini_Pro_ST7789)
  #define LCD_USES "ST7789"
  Arduino_GFX *gfx = new Arduino_ST7789(bus,TFT_RST,LCD_ROTATION,true,LCD_WIDTH,LCD_HEIGHT,2,1);
#elif defined(LOLIN_S3_Mini_Pro_GC9107)
  #define LCD_USES "GC9107"
  Arduino_GFX *gfx = new Arduino_GC9107(bus,TFT_RST,LCD_ROTATION,true);
#elif defined(LOLIN_S3_Mini_Pro_GC9A01)
  #define LCD_USES "GC9A01"
  Arduino_GFX *gfx = new Arduino_GC9A01(bus,TFT_RST,LCD_ROTATION,true);
#elif defined(LOLIN_S3_Mini_Pro_ILI9341)
  #define LCD_USES "ILI9341"
  Arduino_GFX *gfx = new Arduino_ILI9341(bus,TFT_RST,LCD_ROTATION,true);

#endif 

  #define RGB_DATA 8
  #define NUM_LEDS 1
  Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, RGB_DATA, NEO_GRB + NEO_KHZ800);

/////////////////////////////////////////////////////////////////

struct s_licenses  
{
  bool firebase = false;
  bool telegram = false;
  bool whatsapp = false;
  bool facebook = false;
  bool patterns = false;
  bool webinterface = true;
  bool multiconfig = false;
  bool demomode = true;
};

// configuration
struct s_config
{
   bool SPIFFS = false;
   bool FFat = false;
   bool PSRAM = false;
   bool GFX = false;
   bool LED = false;
   bool Serial = true;
   bool IR = false;
   bool IMU = false;
   int16_t GFX_width = -1;
   int16_t GFX_height = -1;
   int32_t chipId = 0;
   char SSID[64] = "";
   char psk[40] = "";
};

const char _magicid[9] = "12345678";

struct s_psram{
  char magicid_1[9];
  char magicid_2[9];
};
s_psram *_psram;

struct s_ffat{
};

/////////////////////////////////////////////////////////////////

s_licenses _licenses;
s_config _config;

Button2 button;
WiFiManager wm;

/////////////////////////////////////////////////////////////////

void IR_Init() { 
  #ifdef HAS_IR
     _config.IR = true;
  #endif   
}

void IMU_Init() {
  Serial.print("IMU init");
  #ifdef HAS_IMU
     _config.IMU = true;
  #endif   
  if (_config.IMU) {
    int err = IMU.init(calib, IMU_ADDRESS);
    if (err != 0) {
      Serial.print("Error initializing IMU: ");
      Serial.println(err);
      _config.IMU = false; }    
  }
}

void FFat_init() {
  Serial.println("FFat Mount");
  if(!FFat.begin()){
    FFat.format();
    if(!FFat.begin()){
      Serial.println("FFat Format & Mount Failed");
      _config.FFat = false;
    }
    else {
      _config.FFat = true;
    }
  }
  else {
    _config.FFat = true;
  } }

void SPIFFS_init() {
  Serial.println("SPIFFS Mount");
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
      Serial.println("SPIFFS Mount Failed");
  }
  else {
    _config.SPIFFS = true;
  } 
}

void PSRAM_init() {
  Serial.println("PSRAM Init");
  if (psramInit())
  {
    _config.PSRAM = true;
  }
  else {
    Serial.println("PSRAM Init Failed");
    _config.PSRAM = false;
  } }

bool PSRAMRead_init(u_int32_t _free)
{
  bool _res = false;
  _psram = (struct s_psram*)malloc(sizeof(struct s_psram));
  if ((strcmp(_psram->magicid_1,_magicid)==0) && (strcmp(_psram->magicid_1,_magicid)==0)){
    Serial.println("PSRAM malloc found");
    _res = true;
  }
  else {
    Serial.println("PSRAM malloc init");
    strcpy(_psram->magicid_1,_magicid);
    strcpy(_psram->magicid_2,_magicid);
  }
  return _res;
}

void Wifi_Init() {
  WiFiManager wm;
  bool res;
  wm.setConfigPortalTimeout(30); 
  res = wm.autoConnect("AutoConnectAP","password"); // password protected ap
  if(!res) {
      Serial.println("Failed to connect");
  } 
  else {
      //if you get here you have connected to the WiFi    
      Serial.println("connected...yeey :)");
  }
}

void LED_init() {
  pinMode(RGB_POWER , OUTPUT);      // Set the RGB_PWR pin to OUTPUT
  digitalWrite(RGB_POWER , HIGH);   // Turn on the RGB_PWR (LDO2)

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(RGB_BRIGHTNESS); 

  strip.setPixelColor(0, strip.Color(255, 0, 0)); // Red
  strip.show();
  delay(200);

  // Green
  strip.setPixelColor(0, strip.Color(0, 255, 0)); // Green
  strip.show();
  delay(200);

  // Blue
  strip.setPixelColor(0, strip.Color(0, 0, 255)); // Blue
  strip.show();
  delay(200);

  strip.setPixelColor(0, strip.Color(0, 0, 0)); 
  strip.show(); }

void GFX_init() {
  if (!gfx->begin(pclk_hz))
  {
    Serial.println("gfx->begin() failed!");
    return;
  }
  else
  {
    Serial.println("gfx->begin() ok!");
    _config.GFX = true;
  }

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  _config.GFX_width = gfx->width();
  _config.GFX_height = gfx->height();

  Serial.println(gfx->width());
  Serial.println(gfx->height());

  gfx->fillScreen(WHITE);
  delay(200);
  gfx->fillScreen(RED);
  delay(200);
  gfx->fillScreen(GREEN);
  delay(200);
  gfx->fillScreen(BLUE);
  delay(200);
  gfx->fillScreen(BLACK);

  gfx->setRotation(0);
  gfx->setTextSize(1);
  gfx->setTextColor(WHITE, BLACK); }

/////////////////////////////////////////////////////////////////

void println(char *text)
{
  if (_config.Serial) {Serial.println(text); }
  if (_config.GFX) {gfx->println(text); }
}

void status(void)
{
  char buffer[80];

  sprintf(buffer,"CPU     : %s",ESP.getChipModel());
  println(buffer);
  sprintf(buffer,"Cores   : %ux%uMHz",ESP.getChipCores(),ESP.getCpuFreqMHz());
  println(buffer);
  sprintf(buffer,"Chip ID : %i",_config.chipId);
  println(buffer);
  sprintf(buffer,"APP     : %0.0fKB",1.0*ESP.getFreeSketchSpace()/1024.0);
  println(buffer);
  sprintf(buffer,"Flash   : %0.0fKB",1.0*ESP.getFlashChipSize()/1024.0);
  println(buffer);
  if (_config.PSRAM) {
  sprintf(buffer,"PSRAM   : %0.0fKB",1.0*ESP.getPsramSize()/1024.0);
  println(buffer); }
  if (_config.SPIFFS) {
  sprintf(buffer,"SPIFFS  : %0.0f/%0.0fKB",1.0*SPIFFS.usedBytes()/1024.0,1.0*SPIFFS.totalBytes()/1024.0);
  println(buffer); }
  if (_config.FFat) {
    sprintf(buffer,"FFat    : %0.0f/%0.0fKB",1.0*FFat.freeBytes()/1024.0,1.0*FFat.totalBytes()/1024.0);
    println(buffer); }
  sprintf(buffer,"IR/IMU  : %d %d",_config.IR,_config.IMU);
  println(buffer);
  sprintf(buffer,"SSID    : %s",wm.getWiFiSSID(false));
  println(buffer);
//  snprintf(buffer, sizeof(buffer), "SSID: %s", &WiFi.localIP()[0]);
//  println(buffer);
  gfx->println(WiFi.localIP());
}

/////////////////////////////////////////////////////////////////

void setup() {

  Serial.begin(9600);
  _config.Serial = true;
    
  Serial.println("\n\nS.H.O.C.K");

  Serial.println("ESP.getEfuseMac");
	for(int i=0; i<17; i=i+8) {
	  _config.chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}

  LED_init();
  GFX_init();  
  FFat_init();
  SPIFFS_init();
  PSRAM_init();
  IR_Init();
  IMU_Init();

  bool res = false;

  if (wm.getWiFiIsSaved())
  {
    strip.setPixelColor(0, strip.Color(0, 255, 0)); 
    strip.show(); 
    wm.setConfigPortalTimeout(30);
    res = wm.autoConnect();
  } 
  if (res)
  {
  }
  else
  {
    strip.setPixelColor(0, strip.Color(0, 0, 255)); 
    strip.show(); 
    wm.disconnect();
    wm.setConfigPortalTimeout(120); 
    res = wm.autoConnect();
  }

  strip.setPixelColor(0, strip.Color(0, 0, 0)); 
  strip.show(); 

  status();
  delay(2000);


}

/////////////////////////////////////////////////////////////////

int _blink = 0;

void loop() {

  if (_blink==0) { _blink=255; } else { _blink=0; }
  strip.setPixelColor(0, strip.Color(_blink, 0, 0)); 
  strip.show(); 
  Serial.print(".");
  delay(200);
}
/////////////////////////////////////////////////////////////////
