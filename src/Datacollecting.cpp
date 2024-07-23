/********************************************************
  Bibliotheken
********************************************************/
#include "Prototypes.h"

/********************************************************
  Definition Globale Variablen Data-Collecting
********************************************************/
TINYML_DATA *data_collecting_t = (TINYML_DATA *)heap_caps_malloc(sizeof(TINYML_DATA), MALLOC_CAP_SPIRAM);
enum STATE_TIMER0 state0;

extern armband myo;

uint8_t count_samples = 0;

/********************************************************
  Erstelle Interrupt-Timer (maximal 16)
********************************************************/
ESP32Timer ITimer0(0);
ESP32Timer ITimer1(1);

volatile u_int8_t ICounter = 0;
volatile u_int8_t ICounter1 = 0;

/********************************************************
  Erstelle Instanz für Hardware-Timer
********************************************************/
ESP32_ISR_Timer ISR_Timer;

/********************************************************
  Interrupt Handler für Interrupt-Timer 0

  - Kein Serial.print/println in dieser Funktion !
  - Keine Float-Berechnungen in dieser Funktion !
  - Ganz minimal halten !
********************************************************/
bool IRAM_ATTR TimerHandler0(void *timerNo)
{
  ICounter++;
  return true;
}

/********************************************************
  Interrupt Handler für Interrupt-Timer 1

  - Kein Serial.print/println in dieser Funktion !
  - Keine Float-Berechnungen in dieser Funktion !
  - Ganz minimal halten !
********************************************************/
bool IRAM_ATTR TimerHandler1(void *timerNo)
{

  ICounter1++;
  return true;
}

/********************************************************
  Initalisiere Interrupt-Timer
********************************************************/
void setupISR()
{
  if (ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0))
  {
    if (DEBUG_DATA_COLLECTING)
      Serial.println(F("Starting ITimer 0 OK"));
  }
  else if (DEBUG_DATA_COLLECTING)
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));

  if (ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS * 1000, TimerHandler1))
  {
    if (DEBUG_DATA_COLLECTING)
      Serial.println(F("Starting ITimer 1 OK"));
  }
  else if (DEBUG_DATA_COLLECTING)
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));
}

/********************************************************
  Lese EMG-Rohdaten ein
********************************************************/
void write_emg_sample(int8_t *sample, size_t len)
{
  // Lese Vergleichswerte ein
  data_collecting_t->flag_capture_old = data_collecting_t->flag_capture_new;
  data_collecting_t->flag_capture_new = data_collecting_t->flag_green_light;

  // Falls seit letzem Zyklus eine Änderung aufgetreten ist...
  if (data_collecting_t->flag_capture_old != data_collecting_t->flag_capture_new)
  {
    data_collecting_t->flag_capture_old = data_collecting_t->flag_capture_new;
    ITimer0.restartTimer();
    ICounter = 0;
  }

  // Wenn die Ampel grünes Licht zeigt...
  if (data_collecting_t->flag_green_light == true)
  {
    switch (ICounter)
    {
    // Messintervall ausführen
    case BEGIN:

    if(count_samples < 64)
    {    
      for (int i = 0; i < len; i++)
      {
        Serial.print(sample[i]);
        // if (i < len - 1)
        Serial.print(",");
        count_samples++;
      }
    }
    else
    {
      Serial.println();
      data_collecting_t->iDatapoints[data_collecting_t->iLabel]++;
      count_samples = 0;
    }
      break;
    // Messintervall beendet --> Ampel aus
    case END:
      Serial.print("Label: ");
      Serial.print(data_collecting_t->iLabel);
      Serial.print(",");
      Serial.println();

      data_collecting_t->flag_traffic_light = false;
      data_collecting_t->flag_green_light = false;
      data_collecting_t->iRepetitions_done++;
      ITimer0.stopTimer();
      break;
    // Wenn etwas schiefgeht --> Abbruch
    default:
      sendSomewhat(PREFIX_DATA + "_Start_Collecting", Data_Topic, "false");
      data_collecting_t->flag_start_collecting = false;
      data_collecting_t->flag_traffic_light = false;
      data_collecting_t->flag_green_light = false;
      ITimer0.stopTimer();
      break;
    }
  }
}

/********************************************************
  Callback-Funktion für EMG-Signale
********************************************************/
void emgCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
  myohw_emg_data_t *emg_data = (myohw_emg_data_t *)pData;    
    write_emg_sample(emg_data->sample1, myohw_num_emg_sensors);
    write_emg_sample(emg_data->sample2, myohw_num_emg_sensors);  
}
