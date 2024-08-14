/********************************************************
  Bibliotheken
********************************************************/
#include "Prototypes.h"

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

#include <AsyncMQTT_ESP32.h>
#include <WebServer.h>

/********************************************************
  Definition Globale Variablen Kommunikation
********************************************************/
const char *host = "Embedded-AI";

// MQTT-Topics 
const char *General_Topic = "TinyML/General";
const char *Memory_Topic = "TinyML/Memory";
const char *Data_Topic = "TinyML/DataCollect";
const char *Evaluation_Topic = "TinyML/Evaluation";
const char *Myo_Topic = "TinyML/ControlMyo";

const char *cLabel = "";
const char *cRepetitions = "";
const char *cSamples = "";

String sLabel;
String sRepetitions;
String sSamples;

SpiRamAllocator allocator;

MYO_DATA *myo_control_t = (MYO_DATA *)heap_caps_malloc(sizeof(MYO_DATA), MALLOC_CAP_SPIRAM);
extern TINYML_DATA *data_collecting_t;
extern EVALUATION_DATA *evaluation_data_t;


/********************************************************
  MQTT-Handler
********************************************************/
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

/********************************************************
  Starte WiFi-Verbindung
********************************************************/
void WiFiStart()
{
  /************* Vor Verbinden --> Trennen *************/
  WiFi.mode(WIFI_MODE_NULL);
  /*****************************************************/

  WiFi.setAutoReconnect(false);
  WiFi.setHostname(host);

  /****************** Jetzt Verbinden ******************/
  WiFi.mode(WIFI_STA);
  WiFi.begin("FURZ!Box 7362 SL", "301180mst");

  /****************** SSID anzeigen ********************/
  Serial.println("SSID: " + WiFi.SSID());
  /*****************************************************/

  /******************* IP anzeigen *********************/
  String ipaddress = WiFi.localIP().toString();
  Serial.println("IP-ADDRESSE: " + ipaddress);
  /*****************************************************/

  /******************* MAC anzeigen ********************/
  Serial.println("MAC-ADDRESSE: " + WiFi.macAddress());
  /*****************************************************/
}

/********************************************************
  Starte WiFi-Verbindung [Kurzform]
********************************************************/
void connectToWifi()
{
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin("FURZ!Box 7362 SL", "301180mst");
}

/********************************************************
  Konvertiere ESP32 Reset-Grund zu String
********************************************************/
String ConvertResetReasonToString(esp_reset_reason_t reason)
{
  switch (reason)
  {
  case ESP_RST_UNKNOWN:
    return "Resetgrund unbekannt";
    break;

  case ESP_RST_POWERON:
    return "Reset durch Poweron";
    break;

  case ESP_RST_EXT:
    return "Reset von Extern";
    break;

  case ESP_RST_SW:
    return "Reset durch Software";
    break;

  case ESP_RST_PANIC:
    return "Reset durch Exception";
    break;

  case ESP_RST_INT_WDT:
    return "Reset durch Interupt Watchdog";
    break;

  case ESP_RST_DEEPSLEEP:
    return "Reset nach Deep Sleep";
    break;

  case ESP_RST_BROWNOUT:
    return "Brownout Reset";
    break;

  case ESP_RST_SDIO:
    return "Reset durch SDIO";
    break;

  default:
    return "Resetgrund unbekannt";
    break;
  }
}

/********************************************************
  Starte Mqtt-Verbindung
********************************************************/
void connectToMqtt()
{
  if (DEBUG_COMMUNICATION)
    Serial.println("Verbinde mit MQTT-Broker...");
  mqttClient.connect();
}

/********************************************************
  WiFi-Eventhandling
********************************************************/
void WiFiEvent(WiFiEvent_t event)
{
  if (DEBUG_COMMUNICATION)
    Serial.printf("[WiFi-event] event: %d\n", event);

  switch (event)
  {
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.println(WiFi.localIP());
    connectToMqtt();
    break;
  }
}

/********************************************************
  Trennlinie auf serieller Schnittstelle
********************************************************/
void printSeparationLine()
{
  if (DEBUG_COMMUNICATION)
    Serial.println("************************************************");
}

/********************************************************
  Event für MQTT-Verbindung erfolgreich aufgebaut
********************************************************/
void onMqttConnect(bool sessionPresent)
{
  if (DEBUG_COMMUNICATION)
  {
    Serial.print("Verbunden mit MQTT-Broker: ");
    Serial.print(MQTT_HOST);
    Serial.print(", Port: ");
    Serial.println(MQTT_PORT);
  }

  printSeparationLine();

  if (DEBUG_COMMUNICATION)
  {
    Serial.print("Session aktiv: ");
    Serial.println(sessionPresent);
  }

  printSeparationLine();

  uint16_t packetIdSub1 = mqttClient.subscribe(Data_Topic, 0);
  if (DEBUG_COMMUNICATION)
  {
    Serial.print("Subscribing at QoS 0, packetId: ");
    Serial.println(packetIdSub1);
  }

  printSeparationLine();

  uint16_t packetIdSub2 = mqttClient.subscribe(Evaluation_Topic, 0);
  if (DEBUG_COMMUNICATION)
  {
    Serial.print("Subscribing at QoS 0, packetId: ");
    Serial.println(packetIdSub2);
  }

  printSeparationLine();

  uint16_t packetIdSub3 = mqttClient.subscribe(Myo_Topic, 0);
  if (DEBUG_COMMUNICATION)
  {
    Serial.print("Subscribing at QoS 0, packetId: ");
    Serial.println(packetIdSub3);
  }

  printSeparationLine();

  uint16_t packetIdSub4 = mqttClient.subscribe(General_Topic, 0);
  if (DEBUG_COMMUNICATION)
  {
    Serial.print("Subscribing at QoS 0, packetId: ");
    Serial.println(packetIdSub4);
  }

  printSeparationLine();

  uint16_t packetIdSub5 = mqttClient.subscribe(Memory_Topic, 0);
  if (DEBUG_COMMUNICATION)
  {
    Serial.print("Subscribing at QoS 0, packetId: ");
    Serial.println(packetIdSub5);
  }

  printSeparationLine();

  sendSomewhat(PREFIX_MYO, Myo_Topic, "Topic TinyML_MyoControl ist bereit!");
  sendSomewhat(PREFIX_DATA, Data_Topic, "Topic TinyML_DataCollect ist bereit!");
  sendSomewhat(PREFIX_EVAL, Evaluation_Topic, "Topic TinyML_Evaluation ist bereit!");
  sendSomewhat(PREFIX_GENERAL, General_Topic, "Topic TinyML_General ist bereit!");
  sendSomewhat(PREFIX_MEMORY, Memory_Topic, "Topic TinyML_Memory ist bereit!");
}

/********************************************************
  Event für MQTT-Verbindung getrennt
********************************************************/
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  (void)reason;

  if (DEBUG_COMMUNICATION)
    Serial.println("Getrennt von MQTT.");

  if (WiFi.isConnected())
  {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

/********************************************************
  Event für MQTT-Subscribe
********************************************************/
void onMqttSubscribe(const uint16_t &packetId, const uint8_t &qos)
{
  if (DEBUG_COMMUNICATION)
  {
    Serial.println("Subscribe quittiert.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
    Serial.print("  qos: ");
    Serial.println(qos);
  }
}

/********************************************************
  Event für MQTT-Unsubscribe
********************************************************/
void onMqttUnsubscribe(const uint16_t &packetId)
{
  if (DEBUG_COMMUNICATION)
  {
    Serial.println("Unsubscribe quittiert.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
  }
}

/********************************************************
  Event für MQTT-Nachricht erhalten
********************************************************/
void onMqttMessage(char *topic, char *payload, const AsyncMqttClientMessageProperties &properties,
                   const size_t &len, const size_t &index, const size_t &total)
{
  (void)payload;

  if (DEBUG_COMMUNICATION)
  {
    Serial.println("Publish empfangen!");
    Serial.print("Topic: ");
    Serial.println(topic);
    Serial.print("Payload: ");
    Serial.println(payload);
  }

  JsonDocument doc(&allocator);

  // Deserialisieren vom JSON-Payload
  DeserializationError error = deserializeJson(doc, payload);

  if (error)
  {
    if (DEBUG_COMMUNICATION)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    }
    return;
  }

  // Bei Message an Topic "TinyML/DataCollect" ...
  if (String(topic).indexOf("TinyML/DataCollect") >= 0)
  {
    // Lese Label aus UI
    String sLabel = doc[PREFIX_DATA + "_Label"];
    data_collecting_t->iLabel = sLabel.toInt();

    // Lese Anzahl der Wiederholungen aus UI
    String sRepetitions = doc[PREFIX_DATA + "_Repetitions"];
    data_collecting_t->iRepetitions = sRepetitions.toInt();

    // Lese Anzahl der Samples aus UI
    String sSamples = doc[PREFIX_DATA + "_Samples"];
    data_collecting_t->iSamples = sSamples.toInt();

    // Lese Start-Button aus UI
    String sStart = doc[PREFIX_DATA + "_Start_Collecting"];

    if (sStart.indexOf("true") >= 0)
    {
      data_collecting_t->flag_start_collecting = true;
    }
    else
    {
      data_collecting_t->flag_start_collecting = false;
    }
  }

  // Bei Message an Topic "TinyML/ControlMyo" ...
  if (String(topic).indexOf("TinyML/ControlMyo") >= 0)
  {
    // Lese Button Bluetooth-->verbinden aus UI
    String sBLEConnect = doc[PREFIX_MYO + "_Connect_Bluetooth"];
    if (sBLEConnect.indexOf("true") >= 0)
    {
      myo_control_t->flag_connect_bluetooth = true;
    }
    else
    {
      myo_control_t->flag_connect_bluetooth = false;
    }

    // Lese Button Speicher aufzeichnen aus UI
    String sMonitorInternal = doc[PREFIX_MYO + "_Monitor_IntRAM"];
    if (sMonitorInternal.indexOf("true") >= 0)
    {      
      myo_control_t->flag_monitor_internal = true;
    }
    else
    {     
      myo_control_t->flag_monitor_internal = false;
    }

    // Lese Button EMG-Signal aufzeichnen aus UI
    String sMonitorExternal = doc[PREFIX_MYO + "_Monitor_ExtRAM"];
    if (sMonitorExternal.indexOf("true") >= 0)
    {     
      myo_control_t->flag_monitor_external = true;
    }
    else
    {     
      myo_control_t->flag_monitor_external = false;
    }
  }

  // Bei Message an Topic "TinyML/Evaluation" ...
  if (String(topic).indexOf("TinyML/Evaluation") >= 0)
  {
    // Lese Button "Lade Testdaten" aus UI
    String sLoadTestData = doc[PREFIX_EVAL + "_Load_Testdata"];
    if (sLoadTestData.indexOf("true") >= 0)
    {      
      evaluation_data_t->flag_load_testdata = true;    
      evaluation_data_t->flag_loaded_testdata = false;    
      sendSomewhat(PREFIX_EVAL + "_Load_Testdata", Evaluation_Topic, "false");
      sendSomewhat(PREFIX_EVAL + "_State_Testdata", Evaluation_Topic, "0");  
    }
    else
    {
      evaluation_data_t->flag_load_testdata = false;
    }

  // Lese Button "Lade Validierungsdaten" aus UI
    String sLoadValidationData = doc[PREFIX_EVAL + "_Load_Valdata"];
    if (sLoadValidationData.indexOf("true") >= 0)
    {      
      evaluation_data_t->flag_load_validationdata = true;    
      evaluation_data_t->flag_loaded_validationdata = false;    
      sendSomewhat(PREFIX_EVAL + "_Load_Valdata", Evaluation_Topic, "false");
      sendSomewhat(PREFIX_EVAL + "_State_Valdata", Evaluation_Topic, "0");  
    }
    else
    {
      evaluation_data_t->flag_load_validationdata = false;
    }  

   // Lese Button "Starte Evaluierung" aus UI
    String sStartEvaluation = doc[PREFIX_EVAL + "_Start_Evaluation"];
    if (sStartEvaluation.indexOf("true") >= 0)
    {      
      evaluation_data_t->flag_start_evaluation = true;    
    }
    else
    {
      evaluation_data_t->flag_start_evaluation = false;
    }

    // Lese Button "Starte Klassifizierung" aus UI
    String sStartClassifying = doc[PREFIX_EVAL + "_Start_Classifying"];
    if (sStartClassifying.indexOf("true") >= 0)
    {      
      evaluation_data_t->flag_start_classifying = true;    
      evaluation_data_t->flag_classifying_light = true;
    }
    else
    {
      evaluation_data_t->flag_start_classifying = false;
    }     
  }
}

/********************************************************
  Event für MQTT-Publish erfolgt
********************************************************/
void onMqttPublish(const uint16_t &packetId)
{
  if (DEBUG_COMMUNICATION)
  {
    Serial.println("Publish erledigt.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
  }
}

/********************************************************
  MQTT-Verbindung initialisieren
********************************************************/
void initMqtt()
{
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0,
                                    reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0,
                                    reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
}

/********************************************************
  MQTT-Nachricht publishen
********************************************************/
void publishMqtt(const char *topic, String payload)
{
  mqttClient.publish(topic, 0, true, payload.c_str());
}

/********************************************************
  Statusmeldung für GUI senden
********************************************************/
void sendStatusText(String payload)
{
  JsonDocument status(&allocator);
  String message;

  status[PREFIX_GENERAL + "_SystemState"] = payload;

  serializeJsonPretty(status, message);
  publishMqtt(General_Topic, message);
}

/********************************************************
  Nachricht mit freiem Text senden
********************************************************/
void sendSomewhat(String prefix, const char *topic, String payload)
{
  JsonDocument status(&allocator);
  String message;

  status[prefix] = payload;
  serializeJsonPretty(status, message);
  publishMqtt(topic, message);
}

/********************************************************
  Status von Heap + PSRAM senden
********************************************************/
void sendStatusMemory()
{
  JsonDocument status(&allocator);
  String message;

  status[PREFIX_MEMORY + "_Size_Heap"] = serialized(String(ESP.getHeapSize()));
  status[PREFIX_MEMORY + "_Free_Heap"] = serialized(String(ESP.getFreeHeap()));
  status[PREFIX_MEMORY + "_Used_Heap"] = serialized(String(ESP.getHeapSize() - ESP.getFreeHeap()));
  status[PREFIX_MEMORY + "_Max_Alloc_Heap"] = serialized(String(ESP.getMaxAllocHeap()));
  status[PREFIX_MEMORY + "_Min_Free_Heap"] = serialized(String(ESP.getMinFreeHeap()));
  status[PREFIX_MEMORY + "_Size_Psram"] = serialized(String(ESP.getPsramSize()));
  status[PREFIX_MEMORY + "_Free_Psram"] = serialized(String(ESP.getFreePsram()));
  status[PREFIX_MEMORY + "_Used_Psram"] = serialized(String(ESP.getPsramSize() - ESP.getFreePsram()));
  status[PREFIX_MEMORY + "_Max_Alloc_Psram"] = serialized(String(ESP.getMaxAllocPsram()));
  status[PREFIX_MEMORY + "_Min_Free_Psram"] = serialized(String(ESP.getMinFreePsram()));

  serializeJsonPretty(status, message);
  publishMqtt(Memory_Topic, message);
}

/********************************************************
  Status von Internal RAM (HEAP) über serielle Schnittstelle senden
********************************************************/
void printStatusInternalRAM()
{
  Serial.print(ESP.getFreeHeap());
  Serial.println();
}

/********************************************************
  Status von External RAM (PSRAM) über serielle Schnittstelle senden
********************************************************/
void printStatusExternalRAM()
{
  Serial.print(ESP.getFreePsram());
  Serial.println();
}

/********************************************************
  Status von Chip senden
********************************************************/
void sendStatusChip()
{
  JsonDocument status(&allocator);
  String message;

  status[PREFIX_GENERAL + "_Chip_WiFiSignal"] = WiFi.RSSI();
  status[PREFIX_GENERAL + "_Chip_ResetReason"] = ConvertResetReasonToString(esp_reset_reason());
  status[PREFIX_GENERAL + "_Chip_Cores"] = serialized(String(ESP.getChipCores()));
  status[PREFIX_GENERAL + +"_Chip_Model"] = String(ESP.getChipModel());
  status[PREFIX_GENERAL + +"_Chip_Frequence"] = serialized(String(ESP.getCpuFreqMHz()));

  serializeJsonPretty(status, message);
  publishMqtt(General_Topic, message);
}

/********************************************************
  Status von Myo-Armband senden
********************************************************/
void sendStatusMyo()
{
  JsonDocument status(&allocator);
  String message;

  status[PREFIX_MYO + "_Connect_Bluetooth"] = myo_control_t->flag_connect_bluetooth;
  status[PREFIX_MYO + "_Status_BLE"] = myo_control_t->flag_myo_connected;
  status[PREFIX_MYO + "_Monitor_IntRAM"] = myo_control_t->flag_monitor_internal;
  status[PREFIX_MYO + "_Monitor_ExtRAM"] = myo_control_t->flag_monitor_external;

  serializeJsonPretty(status, message);
  publishMqtt(Myo_Topic, message);
}

/********************************************************
  Status von Data-Collecting senden
********************************************************/
void sendStatusData()
{
  JsonDocument status(&allocator);
  String message;
 
  // Anzahl der Zeilen mit jeweils 64 Einzelwerten
  status[PREFIX_DATA + "_Gesture_DataPackets"] = data_collecting_t->iDatapoints[data_collecting_t->iLabel];
  
  // Anzahl der Einzelwerte insgesamt --> Anzahl der Zeilen x 64
  status[PREFIX_DATA + "_Gesture_DataPoints"] = data_collecting_t->iDatapoints[data_collecting_t->iLabel] * 64;
  
  // 
  status[PREFIX_DATA + "_Total_DataPackets"] = data_collecting_t->iDatapoints[data_collecting_t->iLabel] * data_collecting_t->iRepetitions_done;
  status[PREFIX_DATA + "_Total_DataPoints"] = data_collecting_t->iDatapoints[data_collecting_t->iLabel] * data_collecting_t->iRepetitions_done * 64;
  status[PREFIX_DATA + "_TotalSize"] = (data_collecting_t->iDatapoints[data_collecting_t->iLabel] * 64 * 8) / 1024;
  status[PREFIX_DATA + "_Start_Collecting"] = data_collecting_t->flag_start_collecting;
  serializeJsonPretty(status, message);
  publishMqtt(Data_Topic, message);
}
