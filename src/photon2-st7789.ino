SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions

#include "FreeSans12pt7b.h"
#include <FreeSans9pt7b.h>
#include <DSEG7Classic_BoldItalic16pt7b.h>
#include <DSEG7Classic_BoldItalic10pt7b.h> 
#include <Gotham_Medium10pt7b.h>
#include <Gotham_Medium8pt7b.h>
#include <Orbitron_Medium_40.h> //Orbitron Roboto Syncopate
#include <Orbitron_Bold_20.h>
#include <Roboto_Condensed_Light_24.h>
#include <Roboto_Condensed_Light_Italic_24.h>
#include <weather_icons.h>

#define GRAY  0x8c71
#define DARK_GRAY  0x1062

// Comment out the next line to load from SPI/QSPI flash instead of SD card:
#define USE_SD_CARD

// TFT display and SD card share the hardware SPI interface, using
// 'select' pins for each to identify the active device on the bus.

#define PHN2ST7789
//PARTICLE
#ifdef PHN2ST7789
//Photon 2
#define TFT_CS   D3  // Chip select control pin D8
#define TFT_DC   D5  // Data Command control pin
#define TFT_RST  D4  // Reset pin (could connect to NodeMCU RST, see next line)
#define SD_CS    D6

#define BTN_X -1
#define BTN_Y -1
//the pin that the interrupt is attached to
#define INT_PIN D10

#define TFT_WIDTH  170 // ST7789 240 x 240 and 240 x 320
#define TFT_HEIGHT 320 // ST7789 240 x 240

#define TFT_MISO MISO
#define TFT_MOSI MOSI
#define TFT_SCLK SCK

//#define SPI_FREQUENCY  50000000
#endif

#ifdef PHOTON2
//Photon 2
#define TFT_CS   -1  // Chip select control pin D8
#define TFT_DC   S4  // Data Command control pin
#define TFT_RST  S3  // Reset pin (could connect to NodeMCU RST, see next line)

#define TFT_WIDTH  240 // ST7789 240 x 240 and 240 x 320
#define TFT_HEIGHT 240 // ST7789 240 x 240

#define TFT_MISO D3
#define TFT_MOSI D2
#define TFT_SCLK D4

#define SPI_FREQUENCY  50000000
#endif

//P2
#ifdef P2
//#define SD_CS    S6 // SD card select pin
#define TFT_CS         S3 //for p2 display //-1 for Photon Lander 
#define TFT_RST        S5 //for P2 display //S3 for Photon Lander // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         S4

#define TFT_WIDTH  240 // ST7789 240 x 320
#define TFT_HEIGHT 320 

#define TFT_MISO MISO
#define TFT_MOSI MOSI
#define TFT_SCLK SCK

#define SPI_FREQUENCY  25000000
#endif

#define LED_BLUE D7
#define LED_GREEN D0
#define LED_RED RX



#define DEG2RAD 0.0174532925

#if defined(USE_SD_CARD)
  SdFat                SD;         // SD card filesystem
  Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys
#else
  // SPI or QSPI flash filesystem (i.e. CIRCUITPY drive)
  #if defined(__SAMD51__) || defined(NRF52840_XXAA)
    Adafruit_FlashTransport_QSPI flashTransport(PIN_QSPI_SCK, PIN_QSPI_CS,
      PIN_QSPI_IO0, PIN_QSPI_IO1, PIN_QSPI_IO2, PIN_QSPI_IO3);
  #else
    #if (SPI_INTERFACES_COUNT == 1)
      Adafruit_FlashTransport_SPI flashTransport(SS, &SPI);
    #else
      Adafruit_FlashTransport_SPI flashTransport(SS1, &SPI1);
    #endif
  #endif
  Adafruit_SPIFlash    flash(&flashTransport);
  FatFileSystem        filesys;
  Adafruit_ImageReader reader(filesys); // Image-reader, pass in flash filesys
#endif

Adafruit_ST7789      tft    = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_Image       img;        // An image loaded into RAM
int32_t              width  = 0, // BMP image dimensions
                     height = 0;
                 
// GFXcanvas16 canvas(100,20);
// GFXcanvas16 image_canvas(170,320);
// GFXcanvas16* bg_canvas;
// uint16_t* bg_buff;

//GFXcanvas16 temperatureCanvas(170,80);

GFXcanvas16 timeCanvas(112,30);
GFXcanvas16 worldtimeCanvas(72,25);
GFXcanvas16 windCanvas(70,70);
GFXcanvas16 indicatorCanvas(70,8);

int pattern = 0;
bool LED_STATE = 1;

const float Pi = 3.14159265359; 
unsigned long previousMillis = 0;  // will store last time LED was updated
// constants won't change:
const long interval = 60*1000;  // milliiseconds
const long pubinterval = 600; //interval at which to publish in seconds. this is done by counting number of intervals
unsigned long intervalcount = 0;
unsigned long screen_update_counter = 0;
uint8_t indicator_count = 0;

String todayHigh,todayLow,dayoneHigh,dayoneLow,daytwoHigh,daytwoLow,daythreeHigh,daythreeLow;
String todayForecast,dayoneForecast,daytwoForecast,daythreeForecast;

void threadFunction(void);

String numtoWeekday(int num)
{
  switch (num)
  {
  case 1:
    return ("SUN");
  case 2:
    return("MON");
  case 3:
    return("TUE");
  case 4:
    return("WED");
  case 5:
   return("THU");
  case 6:
    return("FRI");
  case 7:
    return("SAT");
  }
  return "NULL";
}

String numtoMonth(int numMonth)
{
  switch (numMonth)
  {
  case 1:
    return ("JAN");
    break;
  case 2:
    return ("FEB");
    break;
  case 3:
    return ("MAR");
    break;
  case 4:
    return ("APR");
    break;
  case 5:
    return ("MAY");
    break;
  case 6:
    return ("JUN");
    break;
  case 7:
    return ("JUL");
    break;
  case 8:
    return ("AUG");
    break;
  case 9:
    return ("SEPT");
    break;
  case 10:
    return ("OCT");
    break;
  case 11:
    return ("NOV");
    break;
  case 12:
    return ("DEC");
    break;
  }
  return "NULL";
}

void setup(void) 
{
  while(!Particle.connected());
  //Subscribe to the Particle webhook response
  Particle.subscribe("hook-response/get_night_weather", getNightWeather);
  Particle.subscribe("hook-response/get_day_weather", getDayWeather);
  Particle.subscribe("hook-response/get_suntime", getSunriseSunset);
  Particle.subscribe("hook-response/get_wind", getWindHumidity);
  Particle.subscribe("hook-response/get_moon", getMoonPhase);

  Time.zone(-8);//before day light savings it was -7

  pinMode(LED_RED,OUTPUT);
  pinMode(LED_GREEN,OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  digitalWrite(LED_BLUE,LOW);
  digitalWrite(LED_RED,LOW);
  digitalWrite(LED_GREEN,LOW);

    ImageReturnCode stat; // Status from image-reading functions

    Serial.begin(9600);
    //Serial.println("SHTC3 test");

    tft.init(170, 320); // Init ST7789 320x240

    // The Adafruit_ImageReader constructor call (above, before setup())
    // accepts an uninitialized SdFat or FatFileSystem object. This MUST
    // BE INITIALIZED before using any of the image reader functions!
    Serial.print(F("Initializing filesystem..."));
#if defined(USE_SD_CARD)
    // SD card is pretty straightforward, a single call...
    if (!SD.begin(SD_CS, SD_SCK_MHZ(10)))
    { // Breakouts require 10 MHz limit due to longer wires
        Serial.println(F("SD begin() failed"));
        for (;;)
            ; // Fatal error, do not continue
    }
#else
    // SPI or QSPI flash requires two steps, one to access the bare flash
    // memory itself, then the second to access the filesystem within...
    if (!flash.begin())
    {
        Serial.println(F("flash begin() failed"));
        for (;;)
            ;
    }
    if (!filesys.begin(&flash))
    {
        Serial.println(F("filesys begin() failed"));
        for (;;)
            ;
    }
#endif
    Serial.println(F("OK!"));

    tft.setRotation(0);
    tft.fillScreen(ST77XX_BLACK);

  reader.loadBMP("/background04.bmp", img);
  img.draw(tft,0,0);
  tft.drawLine(0,0,170,0,ST77XX_BLACK);
  tft.drawLine(0,1,170,1,ST77XX_BLACK);
  drawGauges();
  drawTimeConsole();
  //tft.drawRGBBitmap(145,28,FULL_MOON,21,21);

  String data = String(10);
  Particle.publish("get_night_weather", data, PRIVATE);
  Particle.publish("get_day_weather", data, PRIVATE);
  Particle.publish("get_suntime", data, PRIVATE);
  Particle.publish("get_wind", data, PRIVATE);
  Particle.publish("get_moon", data, PRIVATE);

  new Thread("testThread", threadFunction);


}

void loop() 
{
  char timestring[10];
  unsigned long currentMillis = millis();

  //update indicator every second
  if (currentMillis - previousMillis >= 1000) 
  {
    previousMillis = currentMillis;
    if(indicator_count > 5) indicator_count = 0;
    drawIndicator(indicator_count);
    indicator_count++;
    screen_update_counter++;
  }
  //update screen every minute
  if (screen_update_counter >= 59) 
  {
    screen_update_counter = 0;
    drawTimeConsole();
    drawGauges();
    intervalcount++;
  }
  if(intervalcount>pubinterval)
  {
    intervalcount = 0;
    String data = String(10);
    Particle.publish("get_day_weather", data, PRIVATE);
    Particle.publish("get_night_weather", data, PRIVATE);
    Particle.publish("get_suntime", data, PRIVATE);
    Particle.publish("get_wind", data, PRIVATE);
    Particle.publish("get_moon", data, PRIVATE);
  }
  
}

void getWindHumidity(const char *event, const char *data) 
{
  char wind_buffer[30];
  char wind_speed[4];
  strncpy(wind_buffer, data, sizeof(wind_buffer) - 1);
  wind_buffer[sizeof(wind_buffer) - 1] = '\0';  // Ensure null-termination

  //get humidity
  char* wind_token = strtok(wind_buffer, ",");  // Split the string by comma
  drawArcLinesTFT(10,290,50,10,15,270.0, 360.0,ST77XX_WHITE);
  fillArc2(10,290,0,30,65,65,8,0x0517);
  fillArc2(10,290,0,map((100.0-atoi(wind_token)),0.0,100.0,0.0,30.0),65,65,8,DARK_GRAY);
  tft.setFont(&Gotham_Medium10pt7b);

  tft.setTextColor(0x0517,ST77XX_BLACK);
  tft.setCursor(5,290);
  tft.print(atoi(wind_token));tft.print("%");

  //get wind speed
  windCanvas.fillScreen(ST77XX_BLACK);
  wind_token = strtok(NULL, ","); 
  windCanvas.setFont(&Gotham_Medium10pt7b);
  windCanvas.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  windCanvas.setCursor(24, 35);
  sprintf(wind_speed, "%.2f", atof(wind_token));
  //sprintf(wind_speed, "%.2f", 99.99); //test string
  windCanvas.setFont();
  windCanvas.print(wind_speed);
  Serial.print("Wind Speed: ");
  Serial.println(wind_token);
  
  //get wind direction
  wind_token = strtok(NULL, ",");
  //draw wind direction dial
  drawArcLines(35, 35, 28, 24, 2, 0.0, 360.0, 0x654d);
  drawArcLines(35, 35, 25, 12, 5, 0.0, 360.0, ST77XX_WHITE);
  drawSingleTriangle(35, 35, 22, atoi(wind_token)-90, 10, ST77XX_WHITE);
  Serial.print("Wind direction: ");
  Serial.println(atoi(wind_token));
  

  tft.drawRGBBitmap(92, 227, windCanvas.getBuffer(), windCanvas.width(), windCanvas.height());
}

//The webhook response data will contain the sunrise and sunset time together as a string
void getSunriseSunset(const char *event, const char *data) 
{
  Serial.println("getting sunrise sunset times");
  Serial.println(data);
  // Make a writable copy of the input string
  char suntimes_buffer[50];  // Adjust size as needed
  strncpy(suntimes_buffer, data, sizeof(suntimes_buffer) - 1);
  suntimes_buffer[sizeof(suntimes_buffer) - 1] = '\0';  // Ensure null-termination

  //get sunrise time
  char* token = strtok(suntimes_buffer, " ");  // Split the string by spaces

  tft.setFont(&Gotham_Medium8pt7b);

  tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
  tft.setCursor(12,180);
  tft.print(token);

  token = strtok(NULL, " "); //get sunset time
  tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
  tft.setCursor(105,180);
  tft.print(token);

  token = strtok(NULL, " "); //get moon phase

  Serial.print("moon phase is: ");
  Serial.println(token);

  if(!strcmp(token,"NEW_MOON"))
  tft.drawRGBBitmap(145,28,NEW_MOON,21,21);
  else if(!strcmp(token,"WAXING_CRESCENT"))
  tft.drawRGBBitmap(145,28,WAXING_CRESCENT,21,21);
  else if(!strcmp(token,"FIRST_QUARTER"))
  tft.drawRGBBitmap(145,28,FIRST_QUARTER,21,21);
  else if(!strcmp(token,"WAXING_GIBBOUS"))
  tft.drawRGBBitmap(145,28,WAXING_GIBBOUS,21,21);
  else if(!strcmp(token,"FULL_MOON"))
  tft.drawRGBBitmap(145,28,FULL_MOON,21,21);
  else if(!strcmp(token,"WANING_GIBBOUS"))
  tft.drawRGBBitmap(145,28,WANING_GIBBOUS,21,21);
  else if(!strcmp(token,"LAST_QUARTER"))
  tft.drawRGBBitmap(145,28,LAST_QUARTER,21,21);
  else if(!strcmp(token,"WANING_CRESCENT"))
  tft.drawRGBBitmap(145,28,WANING_CRESCENT,21,21);
}

void getMoonPhase(const char *event, const char *data)
{
  Serial.println("getting moon phase");
  // char moonPhase[20];
  // strncpy(moonPhase, data, sizeof(moonPhase) - 1);
  // moonPhase[sizeof(moonPhase) - 1] = '\0';  // Ensure null-termination
  if(!strcmp(data,"NEW_MOON"))
  tft.drawRGBBitmap(145,28,NEW_MOON,21,21);
  else if(!strcmp(data,"WAXING_CRESCENT"))
  tft.drawRGBBitmap(145,28,WAXING_CRESCENT,21,21);
  else if(!strcmp(data,"FIRST_QUARTER"))
  tft.drawRGBBitmap(145,28,FIRST_QUARTER,21,21);
  else if(!strcmp(data,"WAXING_GIBBOUS"))
  tft.drawRGBBitmap(145,28,WAXING_GIBBOUS,21,21);
  else if(!strcmp(data,"FULL_MOON"))
  tft.drawRGBBitmap(145,28,FULL_MOON,21,21);
  else if(!strcmp(data,"WANING_GIBBOUS"))
  tft.drawRGBBitmap(145,28,WANING_GIBBOUS,21,21);
  else if(!strcmp(data,"LAST_QUARTER"))
  tft.drawRGBBitmap(145,28,LAST_QUARTER,21,21);
  else if(!strcmp(data,"WANING_CRESCENT"))
  tft.drawRGBBitmap(145,28,WANING_CRESCENT,21,21);

  Serial.println(data);
}

void drawTimeConsole(void)
{
  char daystring[4];
    uint8_t hour = Time.hourFormat12();
    uint8_t indiahour = (hour + 13 + 12) % 12;
    bool indiaPM = indiahour >= 12;
    if (indiahour == 0) indiahour = 12;
    

    uint8_t minute = Time.minute();
    uint8_t indiaminute = (minute + 30) % 60;
    uint8_t second = Time.second();
    uint8_t weekday = Time.weekday();
    uint8_t day = Time.day();
    sprintf(daystring, "%02d", day);
    uint16_t year = Time.year();
    uint8_t month = Time.month();
    char timestring[10];

    timeCanvas.fillScreen(0xfb20);
    timeCanvas.setFont(&DSEG7Classic_BoldItalic16pt7b);
    timeCanvas.setTextColor(0xe2e0,0xfb20);//unlit orange
    timeCanvas.setCursor(0,30);
    timeCanvas.print("88:88");

    sprintf(timestring, "%02d:%02d ", hour, minute);
    timeCanvas.setCursor(0,30);
    timeCanvas.setTextColor(ST77XX_BLACK,0xfb20);//orange
    timeCanvas.print(timestring);
    

    worldtimeCanvas.setFont(&DSEG7Classic_BoldItalic10pt7b);
    worldtimeCanvas.fillScreen(0xfb20);
    sprintf(timestring, "%02d:%02d ", indiahour, indiaminute);
    worldtimeCanvas.setCursor(0,24);
    worldtimeCanvas.setTextColor(ST77XX_BLACK,0xfb20);//orange
    worldtimeCanvas.print(timestring);

    tft.drawRGBBitmap(4, 58, timeCanvas.getBuffer(), timeCanvas.width(), timeCanvas.height());
    tft.drawRGBBitmap(92, 188, worldtimeCanvas.getBuffer(), worldtimeCanvas.width(), worldtimeCanvas.height());

    tft.setFont(&Gotham_Medium8pt7b);

    tft.setTextColor(ST77XX_BLACK,0xfb20);//orange
    tft.fillRect(4,95,28,16,0xfb20);
    tft.setCursor(4,110);
    if (Time.isPM())  tft.print("PM");
    else  tft.print("AM");

    tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
    tft.fillRect(16,10,50,16,ST77XX_BLACK);
    tft.setCursor(16,24);
    tft.print(numtoWeekday(weekday));

    tft.setTextColor(ST77XX_BLACK,0xfb20);
    tft.fillRect(16,35,58,18,0xfb20);
    tft.setCursor(16,48);
    tft.print(daystring);
    //tft.print(" ");
    switch (month)
  {
  case 1:
    tft.print("JAN");
    break;
  case 2:
    tft.print("FEB");
    break;
  case 3:
    tft.print("MAR");
    break;
  case 4:
    tft.print("APR");
    break;
  case 5:
    tft.print("MAY");
    break;
  case 6:
    tft.print("JUN");
    break;
  case 7:
    tft.print("JUL");
    break;
  case 8:
    tft.print("AUG");
    break;
  case 9:
    tft.print("SEPT");
    break;
  case 10:
    tft.print("OCT");
    break;
  case 11:
    tft.print("NOV");
    break;
  case 12:
    tft.print("DEC");
    break;
  }

    tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
    tft.fillRect(88,20,50,18,ST77XX_BLACK);
    tft.setCursor(88,34);
    tft.print(year);

  tft.setFont();

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_BLACK,0xfb20);//orange
  //tft.fillRect(140,218,20,12,ST77XX_RED);
  tft.setCursor(150,218);
  if (indiaPM)  tft.print("PM");
  else  tft.print("AM");

  tft.setFont(&Gotham_Medium10pt7b);

  tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
  tft.setCursor(135,134);

  if (todayHigh == "  ") 
  {
    tft.setCursor(135,134);
    tft.print(todayLow);
  }
  else
  {
    tft.setCursor(135,134);
    tft.print(todayHigh); //timeCanvas.print("|");timeCanvas.print(todayLow);
  }
}

void drawIndicator(int pattern)
{
    indicatorCanvas.fillScreen(ST77XX_BLACK);
    drawFilledParallelogram(0,8,6,8,45,DARK_GRAY);
    drawFilledParallelogram(10,8,6,8,45,DARK_GRAY);
    drawFilledParallelogram(20,8,6,8,45,DARK_GRAY);
    drawFilledParallelogram(30,8,6,8,45,DARK_GRAY);
    drawFilledParallelogram(40,8,6,8,45,DARK_GRAY);

  switch(pattern)
  {
    case 0:
    drawFilledParallelogram(0,8,6,8,45,ST77XX_WHITE);
    drawFilledParallelogram(10,8,6,8,45,DARK_GRAY);
    drawFilledParallelogram(20,8,6,8,45,DARK_GRAY);
    drawFilledParallelogram(30,8,6,8,45,DARK_GRAY);
    drawFilledParallelogram(40,8,6,8,45,DARK_GRAY);
    break;
    
    case 1:
    drawFilledParallelogram(0,8,6,8,45,ST77XX_WHITE);
    drawFilledParallelogram(10,8,6,8,45,ST77XX_WHITE);
    drawFilledParallelogram(20,8,6,8,45,DARK_GRAY);
    drawFilledParallelogram(30,8,6,8,45,DARK_GRAY);
    drawFilledParallelogram(40,8,6,8,45,DARK_GRAY);
    break;

    case 2:
    drawFilledParallelogram(0,8,6,8,45,ST77XX_WHITE);
    drawFilledParallelogram(10,8,6,8,45,ST77XX_WHITE);
    drawFilledParallelogram(20,8,6,8,45,ST77XX_WHITE);
    drawFilledParallelogram(30,8,6,8,45,DARK_GRAY);
    drawFilledParallelogram(40,8,6,8,45,DARK_GRAY);
    break;

    case 3:
    drawFilledParallelogram(0,8,6,8,45,ST77XX_WHITE);
    drawFilledParallelogram(10,8,6,8,45,ST77XX_WHITE);
    drawFilledParallelogram(20,8,6,8,45,ST77XX_WHITE);
    drawFilledParallelogram(30,8,6,8,45,ST77XX_WHITE);
    drawFilledParallelogram(40,8,6,8,45,DARK_GRAY);
    break;

    case 4:
    drawFilledParallelogram(0,8,6,8,45,ST77XX_WHITE);
    drawFilledParallelogram(10,8,6,8,45,ST77XX_WHITE);
    drawFilledParallelogram(20,8,6,8,45,ST77XX_WHITE);
    drawFilledParallelogram(30,8,6,8,45,ST77XX_WHITE);
    drawFilledParallelogram(40,8,6,8,45,ST77XX_WHITE);
    break;

    // case 5:
    // drawFilledParallelogram(0,8,6,8,45,DARK_GRAY);
    // drawFilledParallelogram(10,8,6,8,45,DARK_GRAY);
    // drawFilledParallelogram(20,8,6,8,45,DARK_GRAY);
    // drawFilledParallelogram(30,8,6,8,45,DARK_GRAY);
    // drawFilledParallelogram(40,8,6,8,45,DARK_GRAY);
    // break;

  }
  
  tft.drawRGBBitmap(4, 212, indicatorCanvas.getBuffer(), indicatorCanvas.width(), indicatorCanvas.height());
  tft.drawRGBBitmap(46, 106, indicatorCanvas.getBuffer(), indicatorCanvas.width(), indicatorCanvas.height());
}

void drawGauges(void)
{

  //tft.fillCircle(155,38,10,0xef5b);//moon

  //fillArc2(40,265,0,120,30,30,8,DARK_GRAY);
  //fillArc2(40,265,180,map(82.0,0.0,100.0,0.0,120.0),30,30,8,0x0517); //82% humididy

  tft.setFont();
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
  tft.setCursor(130,5);
  float voltage = analogRead(A6) / 819.2;
  tft.print(voltage);tft.print("v");

  tft.setFont(&Gotham_Medium10pt7b);

  // tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
  // tft.setCursor(120,270);
  // tft.print("12");

  //tft.drawRGBBitmap(125,77,rain,40,40);
  drawWeatherBitmap(125,77,todayForecast);

}

void getNightWeather(const char *event, const char *night_data)
{
  int index0,index1,index2,index3,index4,index6,index5,index7,index8,index9,index10,index11,index12;
  String name;
  String weatherString = String(night_data);
  Serial.println(night_data);

  index0 = weatherString.indexOf('~');
  name = weatherString.substring(0,index0);
  Serial.println(name);
  if(name == "Tonight") 
  {
    Serial.println(night_data);
    todayHigh = "  ";
    index1 = weatherString.indexOf('~',index0+1);
    todayLow = weatherString.substring(index0+1,index1);

    index2 = weatherString.indexOf('~',index1+1);
    todayForecast = weatherString.substring(index1+1,index2);

    index3 = weatherString.indexOf('~',index2+1);
    dayoneHigh = weatherString.substring(index2+1,index3);

    index4 = weatherString.indexOf('~',index3+1);
    dayoneForecast = weatherString.substring(index3+1,index4);

    index5 = weatherString.indexOf('~',index4+1);
    dayoneLow = weatherString.substring(index4+1,index5);

    index6 = weatherString.indexOf('~',index5+1);
    daytwoHigh = weatherString.substring(index5+1,index6);

    index7 = weatherString.indexOf('~',index6+1);
    daytwoForecast = weatherString.substring(index6+1,index7);

    index8 = weatherString.indexOf('~',index7+1);
    daytwoLow = weatherString.substring(index7+1,index8);

    index9 = weatherString.indexOf('~',index8+1);
    daythreeHigh = weatherString.substring(index8+1,index9);

    index10 = weatherString.indexOf('~',index9+1);
    daythreeForecast = weatherString.substring(index9+1,index10);

    index11 = weatherString.indexOf('~',index10+1);
    daythreeLow = weatherString.substring(index10+1,index11);
  }
  else Serial.println("Not tonight");

 
}

void getDayWeather(const char *event, const char *day_data)
{
  int index0,index1,index2,index3,index4,index6,index5,index7,index8,index9,index10,index11,index12;
  String name;
  String weatherString = String(day_data);
  Serial.println(day_data);

  index0 = weatherString.indexOf('~');
  name = weatherString.substring(0,index0);
  Serial.println(name);
  if((name == "Today") || (name == "This Afternoon"))
  {
    Serial.println(day_data);
    index1 = weatherString.indexOf('~',index0+1);
    todayHigh = weatherString.substring(index0+1,index1);

    index2 = weatherString.indexOf('~',index1+1);
    todayForecast = weatherString.substring(index1+1,index2);

    index3 = weatherString.indexOf('~',index2+1);
    todayLow = weatherString.substring(index2+1,index3);

    index4 = weatherString.indexOf('~',index3+1);
    dayoneHigh = weatherString.substring(index3+1,index4);

    index5 = weatherString.indexOf('~',index4+1);
    dayoneForecast = weatherString.substring(index4+1,index5);

    index6 = weatherString.indexOf('~',index5+1);
    dayoneLow = weatherString.substring(index5+1,index6);

    index7 = weatherString.indexOf('~',index6+1);
    daytwoHigh = weatherString.substring(index6+1,index7);

    index8 = weatherString.indexOf('~',index7+1);
    daytwoForecast= weatherString.substring(index7+1,index8);

    index9 = weatherString.indexOf('~',index8+1);
    daytwoLow  = weatherString.substring(index8+1,index9);

    index10 = weatherString.indexOf('~',index9+1);
    daythreeHigh = weatherString.substring(index9+1,index10);

    index11 = weatherString.indexOf('~',index10+1);
    daythreeForecast = weatherString.substring(index10+1,index11);

    index12 = weatherString.indexOf('~',index11+1);
    daythreeLow = weatherString.substring(index11+1,index12);
  }
  else Serial.println("Not today");

 
}

void drawWeatherBitmap(int x,int y,String forecast)
{
  if(!strcmp(forecast,"Sunny"))
  tft.drawRGBBitmap(x,y,sunny,40,40);

  if(!strcmp(forecast,"Mostly Sunny"))
  tft.drawRGBBitmap(x,y,partial_cloudy,40,40);

  else if(!strcmp(forecast,"Clear"))
  tft.drawRGBBitmap(x,y,moon,40,40);

  else if(!strcmp(forecast,"Mostly Clear"))
  tft.drawRGBBitmap(x,y,moon,40,40);

  else if(!strcmp(forecast,"Partly Cloudy"))
  tft.drawRGBBitmap(x,y,cloudy,40,40);

  else if(forecast.indexOf("Cloudy") > 0)
  tft.drawRGBBitmap(x,y,cloud,40,40);

  else if(!strcmp(forecast,"Showers"))
  tft.drawRGBBitmap(x,y,rain,40,40);

  else if(!strcmp(forecast,"Windy"))
  tft.drawRGBBitmap(x,y,windy,40,40);

  else if(forecast.indexOf("Rain") > 0)
  tft.drawRGBBitmap(x,y,rain,40,40);

  else if(forecast.indexOf("thunder") > 0)
  tft.drawRGBBitmap(x,y,rain_thunder,40,40);

  else if(forecast.indexOf("Snow") > 0)
  tft.drawRGBBitmap(x,y,snow,40,40);

  else if(forecast.indexOf("Overcast") > 0)
  tft.drawRGBBitmap(x,y,cloud,40,40);

}

// #########################################################################
// Draw a circular or elliptical arc with a defined thickness
// #########################################################################

// x,y == coords of centre of arc
// start_angle = 0 - 359
// seg_count = number of 3 degree segments to draw (120 => 360 degree arc)
// rx = x axis radius
// yx = y axis radius
// w  = width (thickness) of arc in pixels
// colour = 16 bit colour value
// Note if rx and ry are the same then an arc of a circle is drawn

void fillArc2(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour)
{

  byte seg = 3; // Segments are 3 degrees wide = 120 segments for 360 degrees
  byte inc = 3; // Draw segments every 3 degrees, increase to 6 for segmented ring

    // Calculate first pair of coordinates for segment start
    float sx = cos((start_angle - 90) * DEG2RAD);
    float sy = sin((start_angle - 90) * DEG2RAD);
    uint16_t x0 = sx * (rx - w) + x;
    uint16_t y0 = sy * (ry - w) + y;
    uint16_t x1 = sx * rx + x;
    uint16_t y1 = sy * ry + y;

  // Draw colour blocks every inc degrees
  for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * DEG2RAD);
    float sy2 = sin((i + seg - 90) * DEG2RAD);
    int x2 = sx2 * (rx - w) + x;
    int y2 = sy2 * (ry - w) + y;
    int x3 = sx2 * rx + x;
    int y3 = sy2 * ry + y;

    tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);

    // Copy segment end to sgement start for next segment
    x0 = x2;
    y0 = y2;
    x1 = x3;
    y1 = y3;
  }
}

/**
 * Draw lines along an arc.
 * 
 * centerX and centerY define the coordinates of the center of the arc.
 * radius is the radius of the arc.
 * numSegments is the number of line segments to draw along the arc.
 * segmentLength defines the length of each line segment.
 * startAngleDeg and stopAngleDeg specify the start and stop angles of the arc in degrees.
 * color defines the color of the lines.
 * 
 * This function divides the specified arc into a number of segments and draws small lines along it.
 */
void drawArcLines(int centerX, int centerY, int radius, int numSegments, int segmentLength, float startAngleDeg, float stopAngleDeg, uint16_t color) {
    // Convert angles from degrees to radians
    float startAngle = startAngleDeg * M_PI / 180.0;
    float stopAngle = stopAngleDeg * M_PI / 180.0;

    // Calculate the angular increment between segments
    float angleIncrement = (stopAngle - startAngle) / numSegments; // Span of the arc divided by numSegments

    for (int i = 0; i < numSegments; i++) {
        // Calculate start angle for the segment
        float angle = startAngle + i * angleIncrement;

        // Calculate the starting point of the line
        int startX = centerX + radius * cos(angle);
        int startY = centerY + radius * sin(angle);

        // Calculate the ending point of the line (offset by segment length along the arc)
        int endX = centerX + (radius + segmentLength) * cos(angle);
        int endY = centerY + (radius + segmentLength) * sin(angle);

        // Draw the line segment
        windCanvas.drawLine(startX, startY, endX, endY, color);
    }
}

void drawArcLinesTFT(int centerX, int centerY, int radius, int numSegments, int segmentLength, float startAngleDeg, float stopAngleDeg, uint16_t color) {
    // Convert angles from degrees to radians
    float startAngle = startAngleDeg * M_PI / 180.0;
    float stopAngle = stopAngleDeg * M_PI / 180.0;

    // Calculate the angular increment between segments
    float angleIncrement = (stopAngle - startAngle) / numSegments; // Span of the arc divided by numSegments

    for (int i = 0; i < numSegments; i++) {
        // Calculate start angle for the segment
        float angle = startAngle + i * angleIncrement;

        // Calculate the starting point of the line
        int startX = centerX + radius * cos(angle);
        int startY = centerY + radius * sin(angle);

        // Calculate the ending point of the line (offset by segment length along the arc)
        int endX = centerX + (radius + segmentLength) * cos(angle);
        int endY = centerY + (radius + segmentLength) * sin(angle);

        // Draw the line segment
        tft.drawLine(startX, startY, endX, endY, color);
    }
}

/**
 * Draw a single filled equilateral triangle along an arc.
 * 
 * centerX and centerY define the coordinates of the center of the arc.
 * radius is the radius of the arc.
 * angleDeg specifies the angle (in degrees) where the triangle is centered.
 * triangleLength defines the length of the triangle's sides.
 * color defines the color of the triangle.
 * 
 * This function draws a filled equilateral triangle at a specified angle along an arc, 
 * with the triangle pointing outwards from the center.
 */
void drawSingleTriangle(int centerX, int centerY, int radius, float angleDeg, int triangleLength, uint16_t color) {
    // Convert angle from degrees to radians
    float angle = angleDeg * M_PI / 180.0;

    // Calculate the triangle's apex point (pointing outwards)
    int apexX = centerX + (radius + triangleLength) * cos(angle);
    int apexY = centerY + (radius + triangleLength) * sin(angle);

    // Calculate the two base corners of the triangle
    float offsetAngle = atan2(triangleLength / 2.0, radius); // Half of the base angle
    int corner1X = centerX + radius * cos(angle - offsetAngle);
    int corner1Y = centerY + radius * sin(angle - offsetAngle);
    int corner2X = centerX + radius * cos(angle + offsetAngle);
    int corner2Y = centerY + radius * sin(angle + offsetAngle);

    // Draw the filled triangle
    windCanvas.fillTriangle(apexX, apexY, corner1X, corner1Y, corner2X, corner2Y, color);
}

/**
 * Draw a filled parallelogram.
 * 
 * x and y define the coordinates of the bottom-left corner of the parallelogram.
 * baseLength specifies the length of the bottom side.
 * height specifies the vertical height of the parallelogram.
 * angleDeg specifies the angle (in degrees) of the left side relative to the base.
 * color defines the color of the parallelogram.
 * 
 * This function draws a filled parallelogram with the given dimensions and orientation.
 */
void drawFilledParallelogram(int x, int y, int baseLength, int height, float angleDeg, uint16_t color) {
    // Convert angle from degrees to radians
    float angleRad = angleDeg * M_PI / 180.0;

    // Calculate the coordinates of the four corners of the parallelogram
    int x1 = x;
    int y1 = y;

    int x2 = x1 + baseLength;
    int y2 = y1;

    int x3 = x2 + height * tan(angleRad);
    int y3 = y2 - height; // Ensure height is applied upwards

    int x4 = x1 + height * tan(angleRad);
    int y4 = y1 - height; // Ensure height is applied upwards

    // Draw the filled parallelogram
    indicatorCanvas.fillTriangle(x1, y1, x2, y2, x3, y3, color);
    indicatorCanvas.fillTriangle(x1, y1, x3, y3, x4, y4, color);
}

void threadFunction(void) {
    system_tick_t lastThreadTime = 0;

    while(true) {
      for(int r=0;r<4;r++)
  {
    digitalWrite(LED_RED,HIGH);
    delay(30);
    digitalWrite(LED_RED,LOW);
    delay(30);
  }
  for(int g=0;g<4;g++)
  {
    digitalWrite(LED_GREEN,HIGH);
    delay(30);
    digitalWrite(LED_GREEN,LOW);
    delay(30);
  }
        os_thread_delay_until(&lastThreadTime, 2000);
    }
    // You must not return from the thread function
}