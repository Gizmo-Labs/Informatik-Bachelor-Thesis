/********************************************************
  Bibliotheken
 ********************************************************/
#pragma once
#include <WiFi.h>
#include <ArduinoJson.h>
#include "esp_heap_caps.h"


/********************************************************
  Funktions-Prototypen WiFi
********************************************************/
void WiFiStart();
void connectToWifi();


/********************************************************
  Funktions-Prototypen Kommunikation
********************************************************/
void initMqtt();
void publishMqtt(const char* topic, String payload);
void sendStatusText(String payload);
void sendSomewhat(String prefix, const char *topic, String payload);
void sendStatusMemory();
void sendStatusChip();
void sendStatusMyo();
void sendStatusData();
void printStatusInternalRAM();
void printStatusExternalRAM();


/********************************************************
  Funktions-Prototypen Allgemein
********************************************************/
String ConvertResetReasonToString(esp_reset_reason_t reason);


/********************************************************
  Globale Konstanten Kommunikation
********************************************************/
#define DEBUG_COMMUNICATION false
#define MQTT_HOST IPAddress(192, 168, 178, 200)
#define MQTT_PORT 1883

static const String PREFIX_MYO = "TinyML_ControlMyo";
static const String PREFIX_DATA = "TinyML_DataCollect";
static const String PREFIX_EVAL = "TinyML_Evaluation";
static const String PREFIX_MEMORY = "TinyML_Memory";
static const String PREFIX_GENERAL = "TinyML_General";


/********************************************************
  Deklaration Globale Variablen Kommunikation
********************************************************/
extern String IP;
extern const char* host;

// Subscribe Topics
extern const char *Classify_Topic;
extern const char *Data_Topic;
extern const char *Evaluation_Topic;
extern const char *General_Topic;
extern const char *Train_Topic;
extern const char *Metrics_Topic;
extern const char *Memory_Topic;
extern const char *Myo_Topic;


/********************************************************
  Deklaration Struct Myo-Armband + Bluetooth
********************************************************/
typedef struct
{
    volatile bool flag_connect_bluetooth;          
    volatile bool flag_myo_connected;
    volatile bool flag_monitor_external;
    volatile bool flag_monitor_internal;    
} MYO_DATA;


/********************************************************
  Memory Allocator für Arduino JSON auf externem PSRAM
********************************************************/
struct SpiRamAllocator : ArduinoJson::Allocator
{
  void *allocate(size_t size) override
  {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
  }

  void deallocate(void *pointer) override
  {
    heap_caps_free(pointer);
  }

  void *reallocate(void *ptr, size_t new_size) override
{
    return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
  }
};