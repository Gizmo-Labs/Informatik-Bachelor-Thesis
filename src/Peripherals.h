/********************************************************
  Bibliotheken
********************************************************/
#pragma once
#include <Adafruit_NeoPixel.h>


/********************************************************
  Funktions-Prototypen 
********************************************************/
void setNeoColor(uint8_t red, uint8_t green, uint8_t blue);
void initNeo();
void trafficLight();
void classifyingLight();
bool updateStatus();
bool updateMemory();
bool updateSerialOutput();


/********************************************************
  Globale Konstanten 
********************************************************/
#define PIN 48 
#define NUMPIXELS 1

#define SERIAL_UPDATE 500 
#define STATUS_UPDATE 2000
#define MEMORY_UPDATE 4000

#define PRE_TRAFFIC_LIGHT 1000
#define PRE_CLASSIFYING_LIGHT 200


/********************************************************
  Deklaration Globale Variablen
********************************************************/
extern long previousStatus;
extern long previousMemory;
extern long previousIdle;

