// ******************************************************************************
// *************        Chaos Blinki Bunti Uhr by Micha1986         *************
// *************       Zusammengesetzt aus den Beispielen aus       *************
// *************                      FastLED                       *************
// *************                 FastLED_NeoMatrix                  *************
// *************       sowie einigen Sketches aus dem Internet      *************
// ******************************************************************************
// *************    Es wird meinerseits keine großen Anpassungen    *************
// *************  oder Änderungen am Sketch geben, da sie mit einem *************
// *************  vernüfntigen Sketch schon Leute beschäftigen die  *************
// *************             Ahnung davon haben :D :D               *************
// ******************************************************************************

//Version TimeStamp: 16.06.2020  21:22
#define FW_Version  "0.17" 

#define FASTLED_FORCE_SOFTWARE_SPI
#include <FastLED.h>
#include <FastLED_NeoMatrix.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <time.h>
#include <Fonts/Picopixel.h>
#include <Fonts/TomThumb.h>
#include <pgmspace.h>  // Needed to store stuff in Flash using PROGMEM

#include "Icons.h"
#include "Qbert.h"
#include "DigiDug.h"
#include "Face.h"
#include "Face2.h"
#include "Face3.h"
#include "Feuer.h"
#include "Drink.h"

// Hier können entsprechende Anpassungen vorgenommen werden.
// Im restlichen Sketch bitte nichts ändern, außer du weißt 
// was du entsprechend änderst.
//-----------------------------------------------------------
//-----------------------------------------------------------
//------------------- BEGIN USER SETTINGS -------------------

//                  **********************
//                  * WLAN Zugangsdaten  *
//                  **********************

const char* ssid     = "WLAN Name";              // WLAN Name
const char* password = "WLAN Passwort";          // WLAN Passwort
const char* host     = "ChaosUhr";               // WLAN Name

//                  **********************
//                  * Uhr Einstellungen  *
//                  **********************

#define BRIGHTNESS             20                // Helligkeit Uhr

//                  *************************
//                  * FastLED Einstellungen *
//                  *************************

#define DATA_PIN               D6                // Output PIN für Matrix (D6 für Wemos, für andere Boards evtl. nur die 6)
#define LED_TYPE               WS2812B           // LED Stripe TYP
#define COLOR_ORDER            GRB               // RGB oder GRB Anordung, sofern Farben nicht passen, hier ändern
#define FRAMES_PER_SECOND      150               // Wiederholrate
#define mw                     20                // LEDs in Matrixbreite
#define mh                     15                // LEDs in Matrixhöhe
#define NUM_LEDS               (mw*mh)           // Anzahl LEDs der Matrix


//                  ***************************
//                  * Animation Einstellungen *
//                  ***************************

#define UhrAnimation           2                 // Anzahl durchläufe Datum/Uhrzeit 

#define FaceAnimation          4                 // Anzahl durchläufe der Animation "blaues Gesicht" (ca. 5300ms pro Durchgang)
#define Face2Animation         10                // Anzahl durchläufe der Animation "lila Gesicht" (ca. 140ms pro Durchgang)
#define Face3Animation         4                 // Anzahl durchläufe der Animation "rotes Gesicht" (ca. 1500ms pro Durchgang)
#define FiresAnimation         15                // Anzahl durchläufe der Animation (ca. 480ms pro Durchgang)
#define DrinkAnimation         3                 // Anzahl durchläufe der Animation (ca. 3200ms pro Durchgang)
#define DigiDugAnimation       8                 // Anzahl durchläufe der Animation (ca. 500ms pro Durchgang)
#define QbertAnimation         5                 // Anzahl durchläufe der Animation (ca. 1000ms pro Durchgang)

#define DemoReel1DugAnimation  1100              // Anzahl durchläufe der Animationen "rainbow, confetti, rainbowWithGlitter, sinelon" 
#define DemoReel2Animation     510               // Anzahl durchläufe der Animation "bpm, juggle" (ca. 10ms pro Durchgang)

//------------------- END USER SETTINGS ---------------------
//-----------------------------------------------------------
//-----------------------------------------------------------

//  **************************
//  *   Definiere WebSever   *
//  **************************

ESP8266WebServer server(80);
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

//  **************************
//  *   Definiere Matrix     *
//  **************************

CRGB leds[NUM_LEDS]; 
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, mw, mh, 1, 1, NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG); 

//  **************************
//  * Definitionen Zeitzone  *
//  **************************

struct tm lt;         // http://www.cplusplus.com/reference/ctime/tm/
const char* const PROGMEM ntpServer[] = {"fritz.box", "de.pool.ntp.org", "at.pool.ntp.org", "ch.pool.ntp.org", "ptbtime1.ptb.de", "europe.pool.ntp.org"};
const char* const PROGMEM dayShortNames[] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
const char* const PROGMEM dayNames[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};

//  **************************
//  *   Interne Variablen    *
//  **************************

int rounds = 0;
int dauer = 0;
int x    = matrix->width();
int pass = 0;
int pass1 = 0;
int color = 0;
int r = 255;
int g = 255;
int b = 255;
int r1 = 255;
int g1 = 255;
int b1 = 255;
int durchgang=0;
static char buf[15];                                    // je nach Format von "strftime" eventuell anpassen

//  **************************
//  *    Intitialisieren     *
//  **************************

void setup() { 

  Serial.begin(115200); 
  Serial.print("FW Version: "); 
  Serial.println(FW_Version);  
  FastLED.addLeds<NEOPIXEL,DATA_PIN>(leds, NUM_LEDS);   
  FastLED.setBrightness(BRIGHTNESS);
  
//  ***************************
//  *       Starte WLAN       *
//  ***************************

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.hostname(host);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); 
  delay(250);

//  ***************************
//  *    Starte WEBSERVER     *
//  ***************************  
    
  if (WiFi.status() == WL_CONNECTED) {
    MDNS.begin(host);
    server.on("/", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", serverIndex);
    });
    server.on("/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    }, []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      }
      yield();
    });
    server.begin();
    MDNS.addService("http", "tcp", 80);

    Serial.printf("Ready! Open http://%s.local in your browser\n", host);
  } else {
    Serial.println("WiFi Failed");
  }

//  ***************************
//  *      Starte Matrix      *
//  ***************************

  matrix->begin();
  matrix->setTextWrap(false);
  matrix->setBrightness(BRIGHTNESS);
  matrix->setFont(&TomThumb);
  //matrix->setFont(&Picopixel);  

//  ***************************
//  *       Startscreen       *
//  ***************************  

  matrix->fillScreen(0);       
  matrix->setTextColor(matrix->Color(255, 255, 255));
  matrix->setCursor(5, 6);
  matrix->print("FW: "); 
  matrix->setCursor(4, 13);
  matrix->print(FW_Version);
  matrix->show();
  delay(5000);
  
  matrix->fillScreen(0);       
  matrix->setTextColor(matrix->Color(255, 79, 0));
  matrix->setCursor(2, 6);
  matrix->print("LOAD");  
  matrix->setCursor(0, 13);
  matrix->print("CHAOS");
  matrix->show();
  delay(5000);
  matrix->fillScreen(0);
  
//  ***************************
//  *      Setup Uhrzeit      *
//  ***************************

setupTime();
}
  
void setupTime() {
  configTime(0, 0, ntpServer[1]);  // deinen NTP Server einstellen (von 0 - 5 aus obiger Liste) alternativ lassen sich durch Komma getrennt bis zu 3 Server angeben
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);   // Zeitzone einstellen https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
}

//  ***************************
//  *  Animationsdefinitonen  *
//  ***************************    

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {Uhrzeit, DigiDug, Qbert, Face, Face2, Face3, Fires, rainbow, confetti, rainbowWithGlitter, sinelon, bpm, juggle, Drink};
String Pattern;
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

//  ***************************
//  *  Animationen ausführen  *
//  ***************************    

void loop() { 

  server.handleClient();  
  MDNS.update();  
  
  if (gCurrentPatternNumber == 0){         
      Uhrzeit();     
      if(durchgang >= UhrAnimation){          
        durchgang = 0; 
        nextPattern();            
      }
   }
      
   else {
      Serial.print("Pattern: ");
      Serial.println(gCurrentPatternNumber);
      Serial.println();      
      gPatterns[gCurrentPatternNumber]();
      gHue++;
      if (gCurrentPatternNumber == 7 || gCurrentPatternNumber == 8 || gCurrentPatternNumber == 9 || gCurrentPatternNumber == 10){ 
        if (rounds >= DemoReel1DugAnimation){
             rounds = 0;
             FastLED.clear();
             nextPattern();       
        }
      }      
      else if (gCurrentPatternNumber == 11 || gCurrentPatternNumber == 12){
        if (rounds >= DemoReel2Animation){
             rounds = 0;
             FastLED.clear();
             nextPattern();       
        }
      }      
   }  
}

//  ***************************
//  *     Uhrzeit anzeigen    *
//  *************************** 

void Uhrzeit() {   
  FastLED.setBrightness(BRIGHTNESS);  
  static time_t lastsek {0};
  time_t now = time(&now);
  localtime_r(&now, &lt);  
  if (lt.tm_sec != lastsek) {
    lastsek = lt.tm_sec;
    if (!(time(&now))) {                         // einmal am Tag die Zeit vom NTP Server holen o. jede Stunde "% 3600" aller zwei "% 7200"      
      setupTime();      
    }
    strftime (buf, sizeof(buf), "%d.%m.%Y", &lt);      // http://www.cplusplus.com/reference/ctime/strftime/  
  } 
  //matrix->fillScreen(0);   
  matrix->clear();    
  matrix->setTextColor(matrix->Color(r1,g1,b1));
  matrix->setCursor(2, 6);
  matrix->printf("%.2d:%.2d", lt.tm_hour, lt.tm_min); 
  Datum();
}

//  ***************************
//  *      Scroll Datum       *
//  ***************************
 
void Datum() {     
  char datum[30]; 
  sprintf(datum, "%s   %s", dayNames[lt.tm_wday], buf);
  int scroll = (strlen(datum) * -3) - ((strlen(dayNames[lt.tm_wday]))*2) +3 ; //Anzahl Pixel + Leerzeichen   
  matrix->setTextColor(matrix->Color(r,g,b));  
  matrix->setCursor(x, 13);
  matrix->printf(datum);
  if(--x < scroll) {    
    x = matrix->width();        
    DatumFarbWechsel();  
    durchgang++;   
  }     
  matrix->show();
  delay(125);  
}

//  ***************************
//  *    Nächste Animation    *
//  ***************************

void nextPattern(){ 
  int lastpattern = gCurrentPatternNumber;    
  //int pattern;
  if (lastpattern == 0){
    gCurrentPatternNumber = random(ARRAY_SIZE( gPatterns));    
    while (lastpattern == gCurrentPatternNumber){
      gCurrentPatternNumber = random(ARRAY_SIZE( gPatterns));       
      }     
  }  
  else{    
    UhrzeitFarbWechsel();    
    gCurrentPatternNumber = 0;  
  }  
  FastLED.setBrightness(BRIGHTNESS);    
}


//  ***************************
//  *     Textfarbe Datum     *
//  ***************************
void DatumFarbWechsel(){ 
    if(pass == 0) {
      //Datum
      r=random(100,255);
      g=random(100,255);
      b=255;      
      pass = 1;
    }
    else if(pass == 1) {
      //Datum
      r=random(100,255);
      g=255;
      b=random(100,255);
      pass = 2;     
    }
    else if(pass == 2) {
      //Datum
      r=255;
      g=random(100,255);
      b=random(100,255);      
      pass = 0;      
    }
    else {
      pass=0;
      }
}

//  ***************************
//  *    Textfarbe Uhrzeit    *
//  ***************************
void UhrzeitFarbWechsel(){ 
      if(pass1 == 0) {          
        //Uhrzeit
        r1=255;
        g1=255;
        b1=random(100,255);      
        pass1=1;
      }
      else if(pass1 == 1) {        
        r1=255;
        g1=random(100,255);
        b1=255;    
        pass1=2;  
      }
      else if(pass1 == 2) {         
        r1=random(100,255);
        g1=255;
        b1=255;  
        pass1=0;      
      }
      else {
        r1=255;
        g1=255;
        b1=255; 
        pass1=0;     
      }
}
//  ***************************
//  *    Figur Animationen    *
//  ***************************

void Drink() {
  int rounds = 0;
  while (rounds <= DrinkAnimation){
//    1st Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Drink01[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(200);         

//    2nd Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Drink02[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(200); 
      
//    3rd Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Drink03[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(200);

//    4th Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Drink04[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(200);        

//    5th Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Drink05[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(200); 

//    6th Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Drink06[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(200);      

//    7th Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Drink07[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(200);     

//    8th Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Drink08[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(200);     

//    9th Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Drink09[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(200);         

//    10th Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Drink010[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(200);            

//    11th Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Drink011[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(200);            

//    12th Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Drink012[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(200);         

//    13th Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Drink013[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(200);            

//    14th Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Drink014[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(200);       

//    15th Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Drink015[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(200);            

//    16th Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Drink016[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(200);   
      rounds++;                                                                                       
  }
  nextPattern();    
}


void Qbert() {
  int rounds = 0;
  while (rounds <= QbertAnimation){
//    1st Frame    
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Qbert01[i]));  // Read array from Flash
        }
      FastLED.show();
      delay(500);
      
      
//    2nd Frame 
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Qbert02[i]));
        }
      FastLED.show();
      delay(500);
      rounds++;  
  }
  nextPattern();    
}

void DigiDug() {   
  int rounds = 0;
  while (rounds <= DigiDugAnimation){ 
//    1st Frame 
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(DigDug01[i]));
        }
      FastLED.show();
      delay(250);
      
//    2nd Frame 
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(DigDug02[i]));
      }
      FastLED.show();
      delay(250);    
      rounds++;
    }  
    nextPattern();
}

void Face() {   
  int rounds = 0;  
  while (rounds < FaceAnimation){
//    1st Frame 
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Face001[i]));
        }
      FastLED.show();
      delay(2000);
      
//    2nd Frame 
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Face002[i]));
      }
      FastLED.show();
      delay(1000);  
        
//    3rd Frame
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Face003[i]));
      }
      FastLED.show();
      delay(1000);
      
//    4th Frame
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Face001[i]));
      }
      FastLED.show();
      delay(1000);  
           
//    5th Frame
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS ; i++) {
          leds[i] = pgm_read_dword(&(Face004[i]));
      }
      FastLED.show();
      delay(40);
      
//    6th Frame
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS+1; i++) {
          leds[i] = pgm_read_dword(&(Face005[i]));
      }
      FastLED.show();
      delay(40);
      
//    7th Frame
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS+1; i++) {
          leds[i] = pgm_read_dword(&(Face007[i]));
      }
      FastLED.show();
      delay(40);
      
//    8th Frame
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS+1; i++) {
          leds[i] = pgm_read_dword(&(Face008[i]));
      }
      FastLED.show();
      delay(60);
      
//    9th Frame
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS+1; i++) {
          leds[i] = pgm_read_dword(&(Face007[i]));
      }
      FastLED.show();
      delay(40);
      
//    10th Frame
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS+1; i++) {
          leds[i] = pgm_read_dword(&(Face005[i]));
      }
      FastLED.show();
      delay(40);
      
//    11th Frame
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS+1; i++) {
          leds[i] = pgm_read_dword(&(Face004[i]));
      }
      FastLED.show();
      delay(40);
      rounds++; 
  }  
  nextPattern();
}

void Face2() {   
  int rounds = 0;  
  while (rounds < Face2Animation){    
//    1st Frame
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Face202[i]));
      }
      FastLED.show();
      delay(700);
          
//    2nd Frame
      FastLED.clear();
      for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = pgm_read_dword(&(Face203[i]));
      }
      FastLED.show();
      delay(700);
      rounds++;  
  }
  nextPattern();
}

void Face3() {   
  int rounds = 0;  
  while (rounds < Face3Animation){
//  1st Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Face301[i]));
      }
    FastLED.show();
    delay(100);  
    
//  2nd Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Face302[i]));
    }
    FastLED.show();
    delay(100);   
    
//  3rd Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Face303[i]));
    }
    FastLED.show();
    delay(100);
    
//  4th Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Face304[i]));
      }
    FastLED.show();
    delay(100);       

//  5th Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Face305[i]));
    }
    FastLED.show();
    delay(100);

//  6th Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Face306[i]));
    }
    FastLED.show();
    delay(100);

//  7th Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Face307[i]));
    }
    FastLED.show();
    delay(100);

//  8th Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Face308[i]));
    }
    FastLED.show();
    delay(100);

//  9th Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Face309[i]));
    }
    FastLED.show();
    delay(100);

//  10th Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Face310[i]));
    }
    FastLED.show();
    delay(100);

//  11th Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Face311[i]));
    }
    FastLED.show();
    delay(100);
    
//  12th Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Face312[i]));
    }
    FastLED.show();
    delay(100);

//  13th Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Face313[i]));
    }
    FastLED.show();
    delay(100);

//  14th Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Face314[i]));
    }
    FastLED.show();
    delay(100);

//  15th Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Face315[i]));
    }
    FastLED.show();
    delay(100);
    rounds++;      
  }  
  nextPattern();
}

void Fires() {   
  int rounds = 0;  
  while (rounds < FiresAnimation){  
//  1st Frame      
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Feuer1[i]));
      }
    FastLED.show();
    delay(80);
  
//  2nd Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Feuer2[i]));
    }
    FastLED.show();
    delay(80);    

//  3rd Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Feuer3[i]));
    }
    FastLED.show();
    delay(80);

//  4th Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Feuer4[i]));
    }
    FastLED.show();
    delay(80);

//  5th Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Feuer5[i]));
    }
    FastLED.show();
    delay(80);

//  6th Frame
    FastLED.clear();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = pgm_read_dword(&(Feuer6[i]));
    }
    FastLED.show();
    delay(80);    
    rounds++;
  }
  nextPattern();
}

//  ***************************
//  *   DEMO Reel Animation   *
//  ***************************

void rainbow() {   
  FastLED.setBrightness(BRIGHTNESS);
  fill_rainbow( leds, NUM_LEDS, gHue*2, 7);
  FastLED.show();
  rounds++;
}

void rainbowWithGlitter() {  
  FastLED.setBrightness(BRIGHTNESS);    
  fill_rainbow( leds, NUM_LEDS, gHue*2, 7);
  addGlitter(50);  
  FastLED.show();   
  rounds++;
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }        
}

void confetti() {
  FastLED.setBrightness(BRIGHTNESS*2);   
  fadeToBlackBy( leds, NUM_LEDS, 50);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue*2 + random16(150), 210, 255);
  FastLED.show(); 
  rounds++;
}

void sinelon(){    
  FastLED.setBrightness(BRIGHTNESS*2);  
  fadeToBlackBy( leds, NUM_LEDS,10);
  int pos = beatsin16( 25, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 210);
  FastLED.show();  
  rounds++;
}

void bpm(){  
  FastLED.setBrightness(BRIGHTNESS);
  uint8_t BeatsPerMinute = 45;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
  FastLED.show();   
  FastLED.delay(10);
  rounds++;
  
}

void juggle() {  
  FastLED.setBrightness(BRIGHTNESS);
  fadeToBlackBy( leds, NUM_LEDS, 60);
  byte dothue = 0;
  for( int i = 0; i < 9; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 210, 255);
    dothue += 28;
  }
  FastLED.show();   
  FastLED.delay(10);
  rounds++;
  
}
