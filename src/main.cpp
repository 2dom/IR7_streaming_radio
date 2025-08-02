/*********************************************************************
Braun IR7 Web Radio & Alarm Clock Firmware

This firmware turns an ESP32-based device into a fully featured web radio and alarm clock with the following features:
  - Streams internet radio stations using a VS1053 audio decoder.
  - WiFi configuration and radio station management via a built-in web interface (AP mode).
  - Rotary encoder for intuitive control of volume, station, alarm time, and tone settings.
  - Color TFT display for time, menus, battery, WiFi status, and scrolling song/station info.
  - Real-time clock (RTC) with NTP synchronization and alarm functionality.
  - Battery voltage monitoring and automatic sleep for power saving.
  - Designed for the Braun IR7 hardware, but adaptable to similar ESP32 + VS1053 + TFT projects.

Written by Dominic Buchstaller.
BSD license, check license.txt for more information
*********************************************************************/


#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include <esp_task_wdt.h>
#include <VS1053_ext.h>              
#include "images.h"
#include "AiEsp32RotaryEncoder.h"
#include "driver/rtc_io.h"
#include <TFT_eSPI.h>
#include <ESP32Time.h>
#include "time.h"
#include "NotoSansBold15.h"
#include "Agnella-Bold50.h"
#include "Agnella-Bold30.h"
#include "RTClib.h"
#include "esp_sntp.h"

WiFiMulti wifiMulti;

// Select one backround image
//#define BACKROUND BACKROUND0   // Hex pattern backround
#define BACKROUND BACKROUND1   // Flower backround
//#define BACKROUND BACKROUND2   // Minimal backround


// Select one vendor logo
//#define VENDOR_LOGO VENDOR_LOGO0   // AI generated backround
#define VENDOR_LOGO VENDOR_LOGO1   // Braun backround

// Select one menu
#define MENU MENU0   // AI generated backround
//#define VENDOR_LOGO VENDOR_LOGO1   // Braun backround

#define SPI_MISO_PIN GPIO_NUM_12
#define SPI_MOSI_PIN GPIO_NUM_25
#define SPI_CLK_PIN GPIO_NUM_33
#define VS1053_DREQ GPIO_NUM_23
#define VS1053_CS GPIO_NUM_17
#define VS1053_DCS GPIO_NUM_16
#define VS1053_RST GPIO_NUM_18
#define LCD_DC GPIO_NUM_26
#define LCD_CS GPIO_NUM_27
#define LCD_BL GPIO_NUM_4
#define VCC_PIN 34
#define MOSFET_EN GPIO_NUM_32
#define ENC1_BUT GPIO_NUM_13
#define ENC1_A GPIO_NUM_15
#define ENC1_B GPIO_NUM_2
#define ENC1_STEPS 5
#define DS3231_CLK GPIO_NUM_0
#define DS3231_SDA GPIO_NUM_5

#define myabs(x) ((x)>0?(x):-(x))

WebServer server(80);
bool ap_mode = false;

const byte DNS_PORT = 53;
DNSServer dnsServer;

// Time keeping
#define uS_TO_SEC_FACTOR 1000000ULL
RTC_DS3231 rtc;

// This semapohore manages the SPI bus between LCD and MP3 board
// In theory this should work without but it is much more stable this way
SemaphoreHandle_t SPIsem = NULL;  

// This struct holds the configuration for volume, station, alarm and bass
#define num_configs 4
uint8_t active_config;
struct config_struct
{
  long high_limit=0;
  long low_limit=0;
  long value=0;
};

RTC_DATA_ATTR config_struct myconfig[num_configs];
RTC_DATA_ATTR bool alarm_on;
#define CONFIG_SIZE sizeof(myconfig)

// This is the y value on the LCD where the songle title and station is displayed
int title_pos=115;

unsigned long last_second_update=0; // This is used executing one second repeating tasks
unsigned long last_user_activity=0; // Last time the user did something
unsigned long auto_sleep_timeout=30*60*1000; // Radio goes to sleep after x ms after wakeup from alarm
bool auto_sleep=false;
int last_rssi_update=0; // Last Wifi signal strength
int rssi_limits[]={-80,-70,-67,-50,-30}; // This defines the quality of the wifi signal in five steps (from bad to good)
float last_vcc; // Last battery voltage
int last_minute_update=255; // Last minute of the current time
unsigned long last_station_change=0; // Last time the station was changed
bool change_station=false;  // The station has been changed

// // NTP stuff
String timezone = "CET-1CEST,M3.5.0,M10.5.0/3"; // Berlin
const char* ntp_server = "192.53.103.108"; // ptbtime1.ptb.de Braunschweig

// Wifi stuff
const char* SSID[] = {};//"Look mum...no wires!","FRITZ!BoxGastzugang"};
const char* PSK[] = {};//"huibuhdasnachtgespenst","diefischerinvombodensee"};
#define no_wifi_networks sizeof(SSID)/sizeof(SSID[0])
const uint32_t connectTimeoutMs = 3000;

bool sync_ntp_time=false; // Has NTP time been synced to external real time clock?

// Econder wheel - the last argument enables pull-up resistors for ENC1_A and ENC1_B
AiEsp32RotaryEncoder encoder1 = AiEsp32RotaryEncoder(ENC1_B, ENC1_A, -1, -1, ENC1_STEPS,0); 

// 2nd core task for handling wifi and MP3 stuff
TaskHandle_t Task0;

// Scrolling station name and text
unsigned long last_text_move=0; // Last time the text moved 
int display_text_len;
int display_text_pos;
String station_text, song_text, display_text;

// Display stuff - sprites for scrolling text and displaying current time
TFT_eSPI display = TFT_eSPI();
TFT_eSprite ScrollTextSprite = TFT_eSprite(&display); 
TFT_eSprite ClockSprite = TFT_eSprite(&display);
#define TFT_MY_GRAY 0x3186

// Position of graphical elements
int battery_y_pos=15;
int battery_x_pos=140;
int wifi_y_pos=25;
int wifi_x_pos=100;
int menu_y_pos=165;

String this_stream_url;
String this_stream_name;

VS1053 mp3(VS1053_CS, VS1053_DCS, VS1053_DREQ, VSPI, SPI_MOSI_PIN, SPI_MISO_PIN, SPI_CLK_PIN);


unsigned long start_time;

// NTP Sync successful
void timeSyncCallback(struct timeval *tv)
{
  sync_ntp_time=true;
}

// Encoder interrupt
void IRAM_ATTR readEncoderISR1()
{
  encoder1.readEncoder_ISR();
}

// Fade pins in and out
void pin_fade(bool in, gpio_num_t pin, uint8_t delay_value = 5)
{
   if (in)
   {
    for (int bright=0; bright<255; bright++)
    {
      analogWrite(pin,bright);
      delay(delay_value);
    }
   }
   else
   {
    for (int bright=255; bright>0; bright--)
    {
      analogWrite(pin,bright);
      delay(delay_value);
    }
   }
}

// Nap time
void got_to_sleep()
{
  pin_fade(0,LCD_BL);
  display.fillScreen(TFT_BLACK);
  display.writecommand(0x10); // Turn display off / power saving mode
  digitalWrite(LCD_BL,LOW);

  // Fade out volume
  for (int i=myconfig[0].value; i>0; i--)
  {
    mp3.setVolume(i);
    delay(10);
  }

  digitalWrite(MOSFET_EN,LOW); // Amp off
  pinMode(VS1053_RST,INPUT); // This will put the device in low power mode (on-board pullup for RST)
  esp_sleep_enable_ext0_wakeup(ENC1_BUT, LOW); 
  
  // Hold levels during sleep
  gpio_deep_sleep_hold_en();
  gpio_hold_en(MOSFET_EN);
  

  while (digitalRead(ENC1_BUT)==LOW)
    delay(10);

  // Set alarm clock
  if (alarm_on)
  {
    DateTime now = rtc.now();
    int hour = now.hour();
    int minute = now.minute();  
    Serial.println("RTC local time at sleep: " + String(hour) + ":" + String(minute));
    
    rtc.disableAlarm(1);
    rtc.clearAlarm(1);
    rtc.disableAlarm(2);
    rtc.clearAlarm(2);
    int alarm_hour = myconfig[2].value/60;
    int alarm_minute = myconfig[2].value%60;
    // We set the alarm for 20 secs before the alarm time to account for boot-up
    DateTime alarm_time;
    alarm_time  = DateTime(2020, 6, 25, alarm_hour, alarm_minute, 0) - TimeSpan(0, 0, 0, 20);
    rtc.setAlarm1(alarm_time, DS3231_A1_Hour);
    Serial.println("Alarm is set for "+ String (alarm_time.hour()) + ":" + String(alarm_time.minute()));
    
  }
  else
  {
    rtc.disableAlarm(1);
    rtc.clearAlarm(1);
    rtc.disableAlarm(2);
    rtc.clearAlarm(2);
  }
  delay(100);
  esp_deep_sleep_start(); // <--------------------------SLEEEEEP
}

// Draw the menu
void draw_menu()
{
  display.pushImage(35,menu_y_pos, MENU_WIDTH, MENU_HEIGHT, MENU, TFT_BLACK);
  int select_width=25;
  int select_y=205;
  for (int selector=0; selector<4; selector++ )

  {
    if (active_config==selector)
      display.fillRect(53+selector*46-select_width/2,select_y,select_width,3,TFT_WHITE);
    else
      display.fillRect(53+selector*46-select_width/2,select_y,select_width,3,0x4a69);
  }
}

// Draw wifi reception indicator
void draw_wifi()
{ 
  int sig_db = WiFi.RSSI();
  if (sig_db!= last_rssi_update)
  {
    last_rssi_update= sig_db;
    Serial.println("Wifi signal (db): " + String(sig_db));

    if (sig_db>rssi_limits[0])
      display.fillSmoothCircle(wifi_x_pos,wifi_y_pos,3,TFT_WHITE);
    else
      display.fillSmoothCircle(wifi_x_pos,wifi_y_pos,3,TFT_DARKGREY);
    for (int i=0; i<4; i++)
    {
      if (sig_db>rssi_limits[i+1])
        display.drawSmoothArc(wifi_x_pos,wifi_y_pos,6+3*i,7+3*i,140,220,TFT_WHITE,TFT_BLACK,true);
      else
        display.drawSmoothArc(wifi_x_pos,wifi_y_pos,6+3*i,7+3*i,140,220,TFT_DARKGREY,TFT_BLACK,true);
    }
  }
}

void bat_empty()
{
  display.pushImage(0,0,240,240,BAT_EMPTY_LOGO);
  delay(2000);
  got_to_sleep();
}

// Draw battery voltage indicator
void draw_battery()
{
  float current_vcc=analogRead(VCC_PIN)/565.0;
  if (current_vcc<3.2)
    bat_empty();
  float delta_vcc = abs(current_vcc-last_vcc);
  if (delta_vcc > 0.1)
  {
    Serial.println("VCC battery: " + String(current_vcc) + ", delta_vcc" + String(delta_vcc));
    last_vcc=current_vcc;
    int battery_width=30;
    int battery_height=10;
    int battery_knob_height=6;
    int battery_knob_width=2;
    
    display.fillRect(battery_x_pos-battery_width/2,battery_y_pos,battery_width,battery_height,TFT_BLACK);
    display.drawRect(battery_x_pos-battery_width/2,battery_y_pos,battery_width,battery_height,TFT_WHITE);
    display.fillRect(battery_x_pos+battery_width/2,battery_y_pos+battery_height/2-battery_knob_height/2,battery_knob_width,battery_knob_height,TFT_WHITE);
  
    if (current_vcc<3.2) current_vcc=3.2;
    if (current_vcc>4.2) current_vcc=4.2;
    
    float current_vcc_PC=(current_vcc-3.2); // between 0.0 and 1.0
    
    display.fillRect(battery_x_pos-battery_width/2+1,battery_y_pos+1,(int)((float)battery_width-2.0)*current_vcc_PC,battery_height-2,TFT_WHITE);
    
  }
}

// Draw alarm time
void draw_alarm()
{
  int hour = myconfig[2].value/60;
  int minute = myconfig[2].value%60;
  int alarm_y_pos=80;
  String time_string = "AL";
  if (hour<10) time_string+="0";
  time_string +=String(hour)+":";
  if (minute<10) time_string+="0";
  time_string +=String(minute);
  
  display.loadFont(NotoSansBold15);
  int time_string_len=display.textWidth(time_string);  
  display.setCursor(120-time_string_len/2, alarm_y_pos);
  if (alarm_on)
    display.setTextColor(TFT_RED, TFT_DARKGREY); 
  else  
    display.setTextColor(TFT_WHITE, TFT_DARKGREY);
  display.pushImage(0,alarm_y_pos,240,17,BACKROUND+240*alarm_y_pos);
  display.println(time_string);	
}

// Draws the current time
void draw_time()
{
  DateTime now = rtc.now();
  int hour = now.hour();
  int minute = now.minute();  

  if (minute!= last_minute_update)
  {
    String time_string;
    if (hour<10) time_string+="0";
    time_string +=String(hour)+":";
    if (minute<10) time_string+="0";
    time_string +=String(minute);

    Serial.println("Drawing time: " + time_string);
    
    int time_string_len=ClockSprite.textWidth(time_string);
    ClockSprite.setCursor(120-time_string_len/2, 0);
    ClockSprite.setTextColor(TFT_WHITE, TFT_DARKGREY); 
    ClockSprite.pushImage(0,0,240,40,BACKROUND+240*40);
    ClockSprite.println(time_string);	
    ClockSprite.pushSprite(0,40);
    display.drawSmoothArc(120,120,119,120,0,360,TFT_MY_GRAY,TFT_BLACK,false);
    
    last_minute_update=minute;
  }
  
}

// Start the mp3 decoder board
void start_VS1003 (){
  Serial.println("Starting start_VS1003...");
  mp3.begin();
  mp3.setVolumeSteps(100);
  Serial.println("MP3 Object created...");
  mp3.setVolume(myconfig[0].value);
  Serial.println("start_VS1003 complete");
   
}



// Connect to Wi-Fi
void connect_wifi(){

  for (uint8_t this_network=0; this_network< no_wifi_networks; this_network++)
    wifiMulti.addAP(SSID[this_network], PSK[this_network]);
  
  display.fillScreen(TFT_BLACK);
  display.pushImage(25,65,VENDOR_LOGO_WIDTH,VENDOR_LOGO_HEIGHT, VENDOR_LOGO, TFT_BLACK);
  pin_fade(1,LCD_BL);
  display.loadFont(AgnellaBold30);

  String value_string = "IR7";
  int value_string_width = display.textWidth(value_string,1);
   
  display.setTextColor(TFT_WHITE, TFT_BLACK); // Set the font colour AND the background colour
  display.setCursor(120-value_string_width/2, 35);
  display.println(value_string);

  display.loadFont(NotoSansBold15);

  value_string = "Connecting to wifi ...";
  value_string_width = display.textWidth(value_string,1);
  display.fillRect(0,160,240,16,TFT_BLACK);
  display.setTextColor(TFT_WHITE, TFT_BLACK); 
  display.setCursor(120-value_string_width/2, 160);
  display.println(value_string);

  unsigned long start_time = millis();
  while(1)
  {
      if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED) {
        String value_string = String(WiFi.SSID());
        int value_string_width = display.textWidth(value_string,1);
        display.fillRect(0,160,240,16,TFT_BLACK);
        display.setTextColor(TFT_WHITE, TFT_BLACK); 
        display.setCursor(120-value_string_width/2, 160);
        display.println(value_string);
        Serial.print("WiFi connected: ");
        Serial.print(WiFi.SSID());
        Serial.print(" ");
        Serial.println(WiFi.RSSI());
        break;
      }
      if ((millis()-start_time)>60000)
        got_to_sleep();
      if (digitalRead(ENC1_BUT)==LOW)
        got_to_sleep();
    }   
  pin_fade(0,LCD_BL);
}

// Starting radio straam
void start_stream()
{
  mp3.connecttohost(this_stream_url);
  Serial.println("start_stream complete");
} 

// Initialize display
void init_display(){
  display.init();
  display.fillScreen(TFT_BLACK);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.setTextWrap(false, false);
  display.setTextSize(2);
  display.setSwapBytes(true);
}

// Set-up I/O
void init_io()
{
  

  pinMode(VS1053_RST, OUTPUT); 
  pinMode(VS1053_CS,OUTPUT);
	pinMode(VS1053_DCS,OUTPUT);
  digitalWrite(VS1053_DCS,HIGH);
  digitalWrite(VS1053_CS,HIGH);
 
  gpio_hold_dis(VS1053_RST);
  digitalWrite(VS1053_RST,LOW); // Reset MP3 decoder board on boot
  delay(10);
  digitalWrite(VS1053_RST,HIGH); // Enable MP3 decoder

  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL,LOW); // Disable LCD backlight for now - will fade in later
  
  gpio_hold_dis(MOSFET_EN);
  pinMode(MOSFET_EN, OUTPUT);
  digitalWrite(MOSFET_EN,HIGH); // Enable amp
  
  analogWriteFrequency(50000);

  

  pinMode(VCC_PIN,INPUT);
}

// Start encoders
void start_rot_encoder(){

  active_config=0;
  encoder1.begin();
  encoder1.setup(readEncoderISR1);
  encoder1.setBoundaries(myconfig[active_config].low_limit, myconfig[active_config].high_limit, false); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)    
  encoder1.setAcceleration(250);
  encoder1.setEncoderValue(myconfig[active_config].value);
}

// Use seperate core to do WIFI & mp3 decode stuff
void Task0code( void * pvParameters ){

  for(;;){
      while (xSemaphoreTake(SPIsem, 0) != pdTRUE) {
        delay(1);
      }; 
      mp3.loop();
      xSemaphoreGive(SPIsem);  
      
    delay(1);
    esp_task_wdt_reset();
  } 
}

// Equalizer
void setTone ()
{
        // toneha       = <0..15>        // Setting treble gain (0 off, 1.5dB steps)
        // tonehf       = <0..15>        // Setting treble frequency lower limit x 1000 Hz
        // tonela       = <0..15>        // Setting bass gain (0 = off, 1dB steps)
        // tonelf       = <0..15>        // Setting bass frequency lower limit x 10 Hz

  uint8_t rtone[4]  = {0,0, myconfig[3].value, 15};
  mp3.setTone(rtone);     
}


// Set Time-Zone
void setTimezone(){
  Serial.printf("  Setting Timezone to %s\n",timezone.c_str());
  setenv("TZ",timezone.c_str(),1);  
  tzset();
}

// Setup NTP and time zone
void init_ntp()
{ 
  sntp_set_time_sync_notification_cb(timeSyncCallback);
  sntp_set_sync_interval(5000);
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, ntp_server);
  sntp_init();
  setTimezone(); 
}

// Sync NTP time to external real time clock
void sync_ntp()
{
  // This is set by the NTP timeSyncCallback (NTP)
  if (sync_ntp_time)
  {
    // Don't really need another sync so let's delay/disable that
    sntp_set_sync_interval(3600*1000);
    sntp_stop();

    // Get and set time from ntp
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    rtc.adjust(DateTime(timeinfo.tm_year+1900,timeinfo.tm_mon+1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));  // Set RTC time using NTP epoch time
    Serial.println("RTC synced to NTP time");

    sync_ntp_time=false;
  }
}

void load_wifi_credentials() {
  File f = SPIFFS.open("/wifi.txt", "r");
  if (!f) {
    Serial.println("No wifi.txt found.");
    return;
  }

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    int sep = line.indexOf(';');
    if (sep > 0) {
      String ssid = line.substring(0, sep);
      String pass = line.substring(sep + 1);
      Serial.println("Adding SSID: " + ssid);
      wifiMulti.addAP(ssid.c_str(), pass.c_str());
    }
  }
  f.close();
}

std::vector<String> stationList;

uint8_t numb_of_stations=0;

void load_station_list() {
  stationList.clear();
  
  File f = SPIFFS.open("/stations.txt", "r");
  if (!f) {
    Serial.println("No stations.txt found.");
    return;
  }

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    Serial.println("Station: " + line);
    if (line.length() > 0) stationList.push_back(line);
    numb_of_stations++;
  }
  myconfig[1].high_limit=numb_of_stations-1;
  f.close();
}

void full_init()
{
  if (myconfig[3].high_limit!=15) //No valid config - initialize
  {
    // Volume 
    myconfig[0].low_limit=1;
    myconfig[0].high_limit=50;
    myconfig[0].value=10;

    // Stationƒ
    myconfig[1].low_limit=0;
    myconfig[1].high_limit=0;
    myconfig[1].value=0;

    // Time 
    myconfig[2].low_limit=0;
    myconfig[2].high_limit=(24*60)-1;
    myconfig[2].value=8*60;

    // Tone
    myconfig[3].low_limit=0;
    myconfig[3].high_limit=15;
    myconfig[3].value=15;

    alarm_on=false;

  }

  for (int i=0; i<4; i++)
    Serial.println(String(myconfig[i].low_limit)+ " " + String(myconfig[i].value) + " " + String(myconfig[i].high_limit));
  
  init_io();
  
  SPIsem = xSemaphoreCreateBinary();
  xSemaphoreGive(SPIsem); 

  init_display();

  ScrollTextSprite.setColorDepth(16); 
  ScrollTextSprite.createSprite(1000, 15); 
  ScrollTextSprite.setTextColor(TFT_WHITE,TFT_BLACK); 
  ScrollTextSprite.setTextSize(2);
  ScrollTextSprite.setSwapBytes(true);

  ClockSprite.setColorDepth(16); 
  ClockSprite.createSprite(240, 40); 
  ClockSprite.setTextColor(TFT_WHITE,TFT_BLACK); 
  ClockSprite.setTextSize(2);
  ClockSprite.setSwapBytes(true);
  ClockSprite.loadFont(AgnellaBold50); 

  // Mount SPIFFS (ensure this happens outside of AP mode too)
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed in full_init");
  }

  load_wifi_credentials();
  connect_wifi();
    
  display.pushImage(0,0,240,240,BACKROUND);
  draw_time();
  draw_menu();
  draw_alarm();
  draw_wifi();
  draw_battery();
  
  pin_fade(1,LCD_BL);
  
  start_VS1003();
  Serial.println("\n\nEncoder started\n");

  load_station_list();
    
  String station_string = stationList[myconfig[1].value];
  int del_pos = station_string.indexOf('@');
  this_stream_url=station_string.substring(del_pos+1);
  this_stream_name=station_string.substring(0,del_pos);
  Serial.println(this_stream_url);
  setTone();

  start_stream();
  //digitalWrite(MOSFET_EN,HIGH);
    
 

  start_rot_encoder();
  
  start_time=millis();

  // Use seperate core to do WIFI & mp3 decode stuff
  xTaskCreatePinnedToCore(
      Task0code,   /* Task function. */
      "Task0",     /* name of task. */
      10000,       /* Stack size of task */
      NULL,        /* parameter of the task */
      5,           /* priority of the task */
      &Task0,      /* Task handle to keep track of created task */
      0);          /* pin task to core 0 */                  

  Serial.println("Init complete");
}

void check_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     Serial.println("Wakeup caused by external signal EXT0");break;
    case ESP_SLEEP_WAKEUP_EXT1:     Serial.println("Wakeup caused by external signal EXT1"); break;
    case ESP_SLEEP_WAKEUP_TIMER:    Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP:      Serial.println("Wakeup caused by ULP program"); break;
    default:                        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}



void start_ap_mode() {

  init_io();

  init_display();

  ScrollTextSprite.setColorDepth(16); 
  ScrollTextSprite.createSprite(1000, 15); 
  ScrollTextSprite.setTextColor(TFT_WHITE,TFT_BLACK); 
  ScrollTextSprite.setTextSize(2);
  ScrollTextSprite.setSwapBytes(true);

  ClockSprite.setColorDepth(16); 
  ClockSprite.createSprite(240, 40); 
  ClockSprite.setTextColor(TFT_WHITE,TFT_BLACK); 
  ClockSprite.setTextSize(2);
  ClockSprite.setSwapBytes(true);
  ClockSprite.loadFont(AgnellaBold50); 
    
  display.fillScreen(TFT_BLACK);
  display.pushImage(25,65,VENDOR_LOGO_WIDTH,VENDOR_LOGO_HEIGHT, VENDOR_LOGO, TFT_BLACK);
  pin_fade(1,LCD_BL);
  display.loadFont(NotoSansBold15);
  String value_string = "Connect to Wifi:";
  int value_string_width = display.textWidth(value_string,1);
  display.setTextColor(TFT_WHITE, TFT_BLACK); // Set the font colour AND the background colour
  display.setCursor(120-value_string_width/2, 160);
  display.println(value_string);
  value_string = "Braun IR7 Config";
  value_string_width = display.textWidth(value_string,1);
  display.setTextColor(TFT_WHITE, TFT_BLACK); // Set the font colour AND the background colour
  display.setCursor(120-value_string_width/2, 180);
  display.println(value_string);
  

  display.loadFont(NotoSansBold15);

  WiFi.mode(WIFI_AP);
  WiFi.softAP("Braun IR7 Config", "");
  delay(100);

  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(apIP);

  // Start DNS server to redirect all domains to ESP IP
  dnsServer.start(DNS_PORT, "*", apIP);

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  // Landing page with prepopulated textarea fields
  server.on("/", HTTP_GET, []() {
  String wifi_data = "";
  String station_data = "";

  if (SPIFFS.exists("/wifi.txt")) {
    File wf = SPIFFS.open("/wifi.txt", "r");
    while (wf.available()) {
      wifi_data += wf.readStringUntil('\n');
      wifi_data += "\n";
    }
    wf.close();
  }

  if (SPIFFS.exists("/stations.txt")) {
    File sf = SPIFFS.open("/stations.txt", "r");
    while (sf.available()) {
      station_data += sf.readStringUntil('\n');
      station_data += "\n";
    }
    sf.close();
  }

  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset='utf-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <style>
      body {
        font-family: Arial, sans-serif;
        font-size: 18px;
        background: #f4f4f4;
        margin: 0;
        padding: 0;
      }
      .container {
        max-width: 700px;
        margin: 40px auto;
        background: white;
        padding: 30px;
        border-radius: 12px;
        box-shadow: 0 6px 12px rgba(0,0,0,0.1);
      }
      h2 {
        text-align: center;
        margin-top: 0;
        font-size: 26px;
      }
      label {
        font-weight: bold;
        display: block;
        margin: 20px 0 10px;
        font-size: 20px;
      }
      textarea {
        width: 100%;
        padding: 12px;
        font-family: monospace;
        font-size: 16px;
        resize: vertical;
        border: 1px solid #ccc;
        border-radius: 6px;
      }
      input[type="submit"] {
        margin-top: 30px;
        width: 100%;
        padding: 14px;
        background-color: #007BFF;
        color: white;
        font-size: 20px;
        border: none;
        border-radius: 6px;
        cursor: pointer;
      }
      input[type="submit"]:hover {
        background-color: #0056b3;
      }
    </style>
  </head>
  <body>
    <div class="container">
      <h2>Braun IR7 Web Radio Configuration</h2>
      <form method="POST" action="/save">
        <label for="wifi">Wi-Fi Credentials (SSID;password per line)</label>
        <textarea name="wifi" rows="6">)rawliteral" + wifi_data + R"rawliteral(</textarea>

        <label for="stations">Radio Stations (name@url per line)</label>
        <textarea name="stations" rows="10">)rawliteral" + station_data + R"rawliteral(</textarea>

        <input type="submit" value="Save & Reboot">
      </form>
    </div>
  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", html);
  });

  // Save config and restart
  server.on("/save", HTTP_POST, []() {
    String wifi = server.arg("wifi");
    String stations = server.arg("stations");

    File wf = SPIFFS.open("/wifi.txt", "w");
    wf.print(wifi);
    wf.close();

    File sf = SPIFFS.open("/stations.txt", "w");
    sf.print(stations);
    sf.close();

    server.send(200, "text/html", "<html><body><h2>Saved. Rebooting...</h2></body></html>");
    delay(2000);
    got_to_sleep();
  });

  // Catch-all route: redirect all unknown URLs to /
  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });


  server.begin();
  Serial.println("HTTP server started");

  // Wait for button release
  while (digitalRead(ENC1_BUT)==LOW)
    delay(100);

  // Keep serving the page
  while (true) {
    server.handleClient();
    dnsServer.processNextRequest();
    if (digitalRead(ENC1_BUT)==LOW)
      ESP.restart();
    delay(10);
  }
}

void setup() {

  pinMode(ENC1_BUT, INPUT_PULLUP);

  Serial.begin(115200);
  while (!Serial)
    delay(10);
  Serial.println("Woken up");

  float current_vcc=analogRead(VCC_PIN)/565.0;
  if (current_vcc<3.2)
  {
    Serial.println("Battery Voltage: " + String(current_vcc));
    
    Serial.println("Low battery - going to sleep");
    got_to_sleep();

  }

  delay(1000);
    
  check_wakeup_reason();


  init_ntp();

  // Real time clock
  Wire.setPins(DS3231_SDA, DS3231_CLK);

  if (! rtc.begin()) {
    Serial.println("RTC module is NOT found");
    Serial.flush();
    while (1);
  }

  // BUT pin low at boot - since the encoder button and the real time clock are both using it the 
  // wakeup is most liekly triggered by the external real time clock
  if (digitalRead(ENC1_BUT)==LOW)
  {
    auto_sleep=true;
    Serial.println("Alarm wakeup - enabling autosleep - going to sleeo after s: " + String(auto_sleep_timeout/1000));
  }

  // This resets the alarm and the external interrupt from real time clock
  rtc.disable32K();  // stop signal at the 32K pin
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);
  rtc.disableAlarm(1);
  rtc.disableAlarm(2);
  rtc.writeSqwPinMode(DS3231_OFF); // stop signal at the SQW pin

  delay(100);
  DateTime now = rtc.now();
  int hour = now.hour();
  int minute = now.minute();  
  Serial.println("RTC local time at boot: " + String(hour) + ":" + String(minute));

  // Check if BUT pin is still low after clearing the RTC alarm - since the encoder button and the real time clock are both using, if it the 
  // user is still pressing the button -> enter setup
  if (digitalRead(ENC1_BUT)==LOW)
  {
    Serial.println("Entering user setup");
    ap_mode = true;
    start_ap_mode();
    
  }


  full_init();
  
}

// Show feee memory
void showFreeMem() {
  Serial.print("Free mem:");
  Serial.print(heap_caps_get_free_size(MALLOC_CAP_8BIT));
  Serial.print(" (");
  Serial.print(heap_caps_get_free_size(MALLOC_CAP_8BIT) / sizeof(float));
  Serial.println(" floats)");
}



// Check and update encoder limits
void update_encoder_limits()
{
  if (myconfig[active_config].value>myconfig[active_config].high_limit)
  myconfig[active_config].value=myconfig[active_config].high_limit;

  if (myconfig[active_config].low_limit>myconfig[active_config].value)
  myconfig[active_config].value=myconfig[active_config].low_limit;
  Serial.println("Active: " + String(active_config) + ", Value: " + String(myconfig[active_config].value)+ ", Low: " + String(myconfig[active_config].low_limit)+ ", High: " + String(myconfig[active_config].high_limit)); 
  encoder1.setBoundaries(myconfig[active_config].low_limit, myconfig[active_config].high_limit, false); 
  encoder1.setEncoderValue(myconfig[active_config].value);
}
// Main task
void loop() {

    yield();
    esp_task_wdt_reset();
    delay(10);
    wifiMulti.run();  
    if (encoder1.encoderChanged())
    {
      last_user_activity=millis();
      showFreeMem();
      uint16_t enc_val = encoder1.readEncoder();
      myconfig[active_config].value=enc_val;
      Serial.println("ENC1 VAL: " + String(enc_val));
      if (active_config==0) mp3.setVolume(enc_val);
      if (active_config==1)
      {
        last_station_change=millis();
        change_station=true;
        String station_string=stationList[enc_val];
        int del_pos = station_string.indexOf('@');
        this_stream_url=station_string.substring(del_pos+1);
        this_stream_name=String(enc_val) + " " + station_string.substring(0,del_pos);
        Serial.println(this_stream_url);
        ScrollTextSprite.loadFont(NotoSansBold15);
        int station_name_len=ScrollTextSprite.textWidth(this_stream_name);
        // This generates a sprite with sation title and song name that we will move over the screen
        ScrollTextSprite.pushImage(0,0,240,16,BACKROUND+240*title_pos);
        ScrollTextSprite.setCursor(128-station_name_len/2, 0);
        ScrollTextSprite.setTextColor(TFT_WHITE); 
        ScrollTextSprite.println(this_stream_name);	
        ScrollTextSprite.pushSprite(0, title_pos+1);
        display.drawSmoothArc(120,120,119,120,0,360,TFT_MY_GRAY,TFT_BLACK,false);
      }
      if (active_config==3)
        setTone();
    

      if (active_config==2)
        draw_alarm();
      
    }

    // New station selected
    if ((change_station)&&((millis()-last_station_change)>2000))
    {
      display_text_pos=260;
      yield();
      esp_task_wdt_reset();
      start_stream();
      change_station=false;
      last_text_move=millis()+3000;

    }

    // Encoder button pressed
    if ((digitalRead(ENC1_BUT)==0))
      {
        bool long_press=false;
        unsigned long press_time=millis();

        // Check for long press
        while (digitalRead(ENC1_BUT)==0)
        {
          unsigned long press_duration=millis()-press_time;
          if (press_duration>1000)
          {
            long_press=true; 
            break;
          }
           delay(1);
        }

        if (long_press)
        {
          Serial.println("Button long press");
          if (active_config==0)
            got_to_sleep();
          if (active_config==2)
          {
            alarm_on=!alarm_on;
            draw_alarm();
          }
        }
        else
        {
          Serial.println("Button short press");
          
          myconfig[active_config].value=encoder1.readEncoder();
          active_config++;
          if (active_config >=num_configs)
            active_config=0;
          draw_menu();
          update_encoder_limits();
        }
      // Wait for button to return to 0
      while (digitalRead(ENC1_BUT)==0)
        delay(1);
      last_user_activity=millis();
    }

    // Timeout on menu selection - revert to default menu item (volume)
    if ((millis()-last_user_activity)>15000)
    {
      active_config=0;
      draw_menu();
      update_encoder_limits();
      last_user_activity=millis();
    }

    // 1 second update loop 
    if ((millis()-last_second_update)>1000UL)
    {
      last_second_update = millis();
      sync_ntp();
      draw_time();
      draw_wifi();
      draw_battery();
        
    }

    // 
    if ((auto_sleep) && (millis()>auto_sleep_timeout))
      got_to_sleep();



    // Move station and song name every 20ms
    if (((millis()-last_text_move)>20)&&(mp3.isRunning()) && !change_station)
    {
      last_text_move=millis();
      if(display_text_pos==260)
        display.pushImage(0,title_pos,240,16,BACKROUND+240*title_pos);
      display_text_pos-=1;

      // End of line - start scrolling text from beginning
      if (display_text_pos < -display_text_len-10)
        display_text_pos=240;

      while (xSemaphoreTake(SPIsem, 0) != pdTRUE) {delay(1);}; // Lock with semaphore to prevent interference with SPI trafic for mp3 decoder
      ScrollTextSprite.pushImage(-display_text_pos,0,240,15,BACKROUND+240*title_pos);
      ScrollTextSprite.setCursor(3, 0);
      ScrollTextSprite.setTextColor(TFT_WHITE);
      ScrollTextSprite.println(display_text);	
      ScrollTextSprite.pushSprite(display_text_pos, title_pos+1);
      display.drawSmoothArc(120,120,119,120,0,360,TFT_MY_GRAY,TFT_BLACK,false);
      xSemaphoreGive(SPIsem);  
      
    }
}


void vs1053_showstation(const char* info) {
   station_text=String(info);    
}

void vs1053_showstreamtitle(const char* info) {

  song_text=String(info);
  display_text=String(myconfig[1].value) + " " + station_text+" xxx " + song_text;
  ScrollTextSprite.loadFont(NotoSansBold15);
  display_text_len=ScrollTextSprite.textWidth(display_text);
  display_text_pos=260;
     
}

void audio_eof_stream(const char* info) {
    Serial.printf("End of stream: %s\n", info);
    start_stream();
}

void vs1053_bitrate(const char *br){               
  Serial.print("BITRATE:      ");
  Serial.println(String(br)+"kBit/s");            
  String bitrate = String(br);

  int value_pos=215;
  display.loadFont(NotoSansBold15);
  String value_string = bitrate.substring(0,bitrate.length()-3)+"kBit/s";
  int value_string_len=display.textWidth(value_string);
  display.setTextColor(TFT_WHITE, TFT_DARKGREY); // Set the font colour AND the background colour
  display.pushImage(0,value_pos,240,16,BACKROUND+240*value_pos);
  display.setCursor(120-value_string_len/2, value_pos);
  display.println(value_string); 
}



