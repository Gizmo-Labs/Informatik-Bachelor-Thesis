/********************************************************
  Bibliotheken
********************************************************/
#include "Prototypes.h"


/********************************************************
  Neopixel
********************************************************/
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


/********************************************************
  Extern deklarierte Instanzen
********************************************************/
extern TINYML_DATA *data_collecting_t;
extern EVALUATION_DATA *evaluation_data_t;


/********************************************************
  Farbe von RGB-LED einstellen
********************************************************/
void setNeoColor(uint8_t red, uint8_t green, uint8_t blue)
{
    pixels.setPixelColor(0, pixels.Color(red, green, blue)); 
    pixels.show();                                           
}


/********************************************************
  Initialisierung der RGB-LED
********************************************************/
void initNeo()
{
    pixels.begin();
    pixels.show();             
    pixels.setBrightness(64); 
}


/********************************************************
  Startampel zum Datensammeln
********************************************************/
void trafficLight()
{
  data_collecting_t->flag_traffic_light = true;
  sendStatusText("Datenaufnahme wenn Ampel grün!");  
  setNeoColor(255, 0, 0); // Rot
  delay(PRE_TRAFFIC_LIGHT);
  setNeoColor(255, 127, 0); // Dunkelorange
  delay(PRE_TRAFFIC_LIGHT);
  setNeoColor(0, 255, 0); // Grün
  delay(PRE_TRAFFIC_LIGHT);
  data_collecting_t->flag_green_light = true;
}


/********************************************************
  Blinken beim Klassifizieren
********************************************************/
void classifyingLight()
{  
  sendStatusText("Klassifizierung aktiv!");  
  delay(PRE_CLASSIFYING_LIGHT);
  setNeoColor(255, 0, 255); // Magenta
  delay(PRE_CLASSIFYING_LIGHT);
  setNeoColor(0, 0, 0); // Magenta  
}


/********************************************************
  Zyklisches Update für serielle Ausgabe
********************************************************/
bool updateSerialOutput()
{
  static long currentMillis = millis();
  if (millis() - currentMillis >= SERIAL_UPDATE)
  {
    currentMillis = millis();
    return true;
  }
  else
  {
    return false;
  }
}


/********************************************************
  Zyklisches Update Stati
********************************************************/
bool updateStatus()
{
  static long currentMillis = millis();
  if (millis() - currentMillis >= STATUS_UPDATE)
  {
    currentMillis = millis();
    return true;
  }
  else
  {
    return false;
  }
}

/********************************************************
  Zyklisches Update Speicherauslastung
********************************************************/
bool updateMemory()
{
  static long currentMillis = millis();
  if (millis() - currentMillis >= MEMORY_UPDATE)
  {
    currentMillis = millis();
    return true;
  }
  else
  {
    return false;
  }
}
