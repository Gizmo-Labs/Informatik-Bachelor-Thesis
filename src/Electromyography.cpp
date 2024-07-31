/********************************************************
  Bibliotheken
********************************************************/
#include "Prototypes.h"
#include "ESP32TimerInterrupt.h"
#include "ESP32_ISR_Timer.h"

/********************************************************
  Myo-Armband Instanz
********************************************************/
armband myo;

extern u_int8_t ICounter1;
extern MYO_DATA *myo_control_t;
extern TINYML_DATA *data_collecting_t;

/********************************************************
  Definition Globale Variablen
********************************************************/

void initMyo()
{
  if (DEBUG_ELECTROMYOGRAPHY)
    Serial.println("Inside initMyo");

  // Debugging für Bluetooth-Stack true/false
  myo.debug = false;

  myo.connect();

  myo.set_myo_mode(myohw_emg_mode_send_emg,         // EMG mode ON
                   myohw_imu_mode_none,             // IMU mode OFF
                   myohw_classifier_mode_disabled); // Classifier mode OFF

  myo.emg_notification(TURN_ON)->subscribe(true, emgCallback);
  myo.set_sleep_mode(1);

  myo_control_t->flag_connect_bluetooth = false;
  myo_control_t->flag_myo_sleepmode_external = false;
}

void setup()
{
  /********************************************************
    Serielle Schnittstelle für Debugging initialisieren
  ********************************************************/
  Serial.begin(115200);
  Serial.println(ARDUINO_BOARD);

  WiFiStart();

  /********************************************************
    Interrupt-Service-Routinen starten
  ********************************************************/
  setupISR();

  myo_control_t = new MYO_DATA();
  data_collecting_t = new TINYML_DATA();

  /********************************************************
    RGB-LED initialisieren
  ********************************************************/
  initNeo();

  /********************************************************
    MQTT initialisieren
  ********************************************************/
  initMqtt();
  sendStatusMyo();

  if (!FFat.begin(FORMAT_FFAT_IF_FAILED, "/ffat", 10, "ffat"))
  {
    Serial.println("Dateisystem FFat konnte nicht erstellt werden!");
    return;
  }
  else
  {
    Serial.println("Dateisystem erfolgreich erstellt!");
  }

  readFile(FFat, "/Labels.csv");
  delay(500);
  readFile(FFat, "/Features.csv");

}

void loop()
{
  /********************************************************
    Zyklisch Allgemeine Daten behandeln
    Wert in Sekunden
  ********************************************************/
  if (updateMemory())
  {
    sendStatusMemory();
    sendStatusChip();
  }

  /********************************************************
    Ausgabe des Heap-Zustandes über serielle Schnittstelle
    Ausgabe alle x-mal 100 Millisekunden
  ********************************************************/
  if (updateStatus())
  {
    sendStatusMyo();

    if ((myo.connected == false) && (myo_control_t->flag_myo_connected == false) && (myo_control_t->flag_connect_bluetooth == false) && (data_collecting_t->flag_start_collecting == false))
    {
      sendStatusText("System bereit --> wartet auf Armband!");
    }
    else if ((myo.connected == true) && (data_collecting_t->flag_traffic_light == false))
    {
      sendStatusText("Armband verbunden --> Betriebsbereit!");
    }
  }

  if (updateSerialOutput())
  {
    if (myo_control_t->flag_monitor_internal == true)
      printStatusInternalRAM();

    if (myo_control_t->flag_monitor_external == true)
      printStatusExternalRAM();
  }

  /********************************************************
    Verbinde Bluetooth über GUI
  ********************************************************/
  if ((myo_control_t->flag_connect_bluetooth == true) && (myo.connected == false))
  {
    initMyo();
  }

  /********************************************************
    Sleepmode über GUI einstellen
  ********************************************************/
  if (myo_control_t->flag_myo_sleepmode_external == true && myo_control_t->flag_myo_sleepmode_internal == false)
  {
    if (DEBUG_ELECTROMYOGRAPHY)
      Serial.println("Changed SleepMode --> Now 0");

    myo.set_sleep_mode(0);
    myo_control_t->flag_myo_sleepmode_internal = true;
  }
  else if (myo_control_t->flag_myo_sleepmode_external == false && myo_control_t->flag_myo_sleepmode_internal == true)
  {
    if (DEBUG_ELECTROMYOGRAPHY)
      Serial.println("Changed SleepMode --> Now 1");

    myo.set_sleep_mode(1);
    myo_control_t->flag_myo_sleepmode_internal = false;
  }

  /********************************************************
    Arband ist verbunden und war es vorher nicht
  ********************************************************/
  if ((myo.connected == true) && (myo_control_t->flag_myo_connected == false))
  {
    setNeoColor(0, 0, 255); // Blau
    myo_control_t->flag_myo_connected = true;
    myo_control_t->flag_connect_bluetooth = false;
    myo_control_t->flag_myo_sleepmode_external = false;
  }
  /********************************************************
    Armband ist verbunden und kein Data-Collecting
  ********************************************************/
  else if ((myo.connected == true) && (data_collecting_t->flag_traffic_light == false))
  {
    setNeoColor(0, 0, 255); // Blau
  }
  /********************************************************
    Armband ist nicht verbunden und war es vorher
  ********************************************************/
  else if ((myo.connected == false) && (myo_control_t->flag_myo_connected == true))
  {
    sendStatusText("Myo-Armband getrennt!");
    setNeoColor(255, 0, 0); // Rot
    myo_control_t->flag_myo_connected = false;
    myo_control_t->flag_myo_sleepmode_external = false;
    data_collecting_t->flag_start_collecting = false;
  }
  /********************************************************
    Armband ist nicht verbunden und war es vorher nicht
  ********************************************************/
  else if ((myo.connected == false) && (myo_control_t->flag_myo_connected == false) && (myo_control_t->flag_connect_bluetooth == false))
  {
    setNeoColor(255, 0, 0); // Rot
  }

  /********************************************************
    Data-Collecting
  ********************************************************/
  if ((data_collecting_t->flag_start_collecting == true) && (myo.connected == true) && (data_collecting_t->flag_traffic_light == false))
  {
    sendStatusText("Datenaufnahme startet!");
    while ((data_collecting_t->iRepetitions_done < data_collecting_t->iRepetitions) && (data_collecting_t->flag_traffic_light == false))
    {
      sendStatusData();
      trafficLight();
    }
    
    if ((data_collecting_t->iRepetitions) == data_collecting_t->iRepetitions_done)
    {
      data_collecting_t->flag_start_collecting = false;
      sendStatusData();
      data_collecting_t->iRepetitions_done = 0;
      data_collecting_t->iDatapoints[data_collecting_t->iLabel] = 0;
    }    
  }

  if ((data_collecting_t->flag_start_collecting == true) && (myo.connected == false))
  {
    data_collecting_t->flag_start_collecting = false;
    sendSomewhat(PREFIX_DATA + "_Start_Collecting", Data_Topic, "false");
    sendStatusText("Datenaufnahme nicht möglich!");
  }
}
