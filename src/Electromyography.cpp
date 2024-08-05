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

/********************************************************
  Extern deklarierte Instanzen
********************************************************/
extern u_int8_t ICounter1;
extern MYO_DATA *myo_control_t;
extern TINYML_DATA *data_collecting_t;

extern MODEL_DATA *model_data_t;
extern EVALUATION_DATA *evaluation_data_t;

/********************************************************
  Definition Globale Variablen
********************************************************/
extern Eloquent::TF::Sequential<TF_NUM_OPS, ARENA_SIZE> tf;

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
  model_data_t = new MODEL_DATA();
  evaluation_data_t = new EVALUATION_DATA();

  /********************************************************
    RGB-LED initialisieren
  ********************************************************/
  initNeo();

  /********************************************************
    MQTT initialisieren
  ********************************************************/
  initMqtt();

  if (!FFat.begin(FORMAT_FFAT_IF_FAILED, "/ffat", 10, "ffat"))
  {
    Serial.println("Dateisystem FFat konnte nicht erstellt werden!");
    return;
  }
  else
  {
    Serial.println("Dateisystem erfolgreich erstellt!");
  }

  while (!tf.begin(FCNN).isOk())
    Serial.println(tf.exception.toString());

  // delay(1000);

  // runConfusionMatrix();
}

void loop()
{
  /********************************************************
      Lade Testdaten
  ********************************************************/
  if ((evaluation_data_t->flag_load_testdata == true))
  {
    evaluation_data_t->flag_load_testdata = false;
    readTestLabels();
    readTestInput();

    if ((evaluation_data_t->flag_loaded_testinputs == true) && (evaluation_data_t->flag_loaded_testlabels == true))
    {
      evaluation_data_t->flag_loaded_testdata = true;
      sendSomewhat(PREFIX_EVAL + "_Load_Testdata", Evaluation_Topic, "false");
      sendSomewhat(PREFIX_EVAL + "_State_Testdata", Evaluation_Topic, "1");
      sendStatusText("Ladevorgang Testdaten erfolgreich!");
      delay(1000);
    }
  }


  /********************************************************
      Lade Validierungsdaten
  ********************************************************/
  if ((evaluation_data_t->flag_load_validationdata == true))
  {
    evaluation_data_t->flag_load_validationdata = false;
    readValidationLabels();
    readValidationInput();

    if ((evaluation_data_t->flag_loaded_validationinputs == true) && (evaluation_data_t->flag_loaded_validationlabels == true))
    {
      evaluation_data_t->flag_loaded_validationdata = true;
      sendSomewhat(PREFIX_EVAL + "_Load_Valdata", Evaluation_Topic, "false");
      sendSomewhat(PREFIX_EVAL + "_State_Valdata", Evaluation_Topic, "1");
      sendStatusText("Ladevorgang Validierungsdaten erfolgreich!");
      delay(1000);
    }
  }


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
    sendStatusData();

    if ((myo.connected == false) && (myo_control_t->flag_myo_connected == false) && (myo_control_t->flag_connect_bluetooth == false) && (data_collecting_t->flag_start_collecting == false))
    {
      sendStatusText("System bereit --> wartet auf Armband!");
    }
    else if ((myo.connected == true) && (data_collecting_t->flag_traffic_light == false))
    {
      sendStatusText("Armband verbunden --> Betriebsbereit!");
    }

    if ((evaluation_data_t->flag_load_testdata == false) && (evaluation_data_t->flag_loaded_testdata == false))
    {
      sendSomewhat(PREFIX_EVAL + "_Load_Testdata", Evaluation_Topic, "false");
      sendSomewhat(PREFIX_EVAL + "_State_Testdata", Evaluation_Topic, "0");
    }

    if ((evaluation_data_t->flag_load_validationdata == false) && (evaluation_data_t->flag_loaded_validationdata == false))
    {
      sendSomewhat(PREFIX_EVAL + "_Load_Valdata", Evaluation_Topic, "false");
      sendSomewhat(PREFIX_EVAL + "_State_Valdata", Evaluation_Topic, "0");
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
    Arband ist verbunden und war es vorher nicht
  ********************************************************/
  if ((myo.connected == true) && (myo_control_t->flag_myo_connected == false))
  {
    setNeoColor(0, 0, 255); // Blau
    myo_control_t->flag_myo_connected = true;
    myo_control_t->flag_connect_bluetooth = false;
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
