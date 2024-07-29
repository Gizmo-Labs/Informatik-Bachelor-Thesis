/********************************************************
  Bibliotheken
 ********************************************************/
#pragma once
#include <stdint.h>

/********************************************************
  Funktions-Prototypen Data-Collecting
********************************************************/
void print_emg_sample(int8_t *sample, size_t len);
void write_emg_sample(int8_t *sample, size_t len);
void emgCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
void batteryCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
void setupISR();

/********************************************************
  Globale Konstanten Data-Collecting

  - Timer Intervall in Millisekunden !!
********************************************************/
#define DEBUG_DATA_COLLECTING false
#define GESTURES 8

#define TIMER0_INTERVAL_MS 2000L
#define TIMER1_INTERVAL_MS 1000L
#define TIMER2_INTERVAL_MS 5000L

// Müssen vor nachfolgenden Includes stehen
#define TIMER_INTERRUPT_DEBUG 2
#define _TIMERINTERRUPT_LOGLEVEL_ 0

/********************************************************
  Spezielle Bibliotheken
********************************************************/
#include "ESP32TimerInterrupt.hpp"
#include "ESP32_ISR_Timer.hpp"

/********************************************************
  Deklaration ISR-Timer
********************************************************/
extern ESP32_ISR_Timer ISR_Timer;

/********************************************************
  Deklaration Struct Data-Collecting
********************************************************/
typedef struct 
{
  volatile bool flag_start_collecting;
  volatile bool flag_traffic_light;
  volatile bool flag_green_light;
  volatile bool flag_capture_old;
  volatile bool flag_capture_new;  
  uint8_t iLabel;
  uint8_t iRepetitions;
  uint8_t iRepetitions_done;
  uint8_t iSamples;
  uint32_t iDatapoints[GESTURES];
  int8_t iBluetoothData[64];  
} TINYML_DATA;


enum STATE_TIMER0
{
  BEGIN = 0,
  END = 1
};

extern enum STATE_TIMER0 state0;