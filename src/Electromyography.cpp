/********************************************************
  Bibliotheken
********************************************************/
#include "Prototypes.h"
#include "ESP32TimerInterrupt.h"
#include "ESP32_ISR_Timer.h"


/********************************************************
  Extern deklarierte Instanzen
********************************************************/
extern u_int8_t ICounter1;
extern MYO_DATA *myo_control_t;
extern TINYML_DATA *data_collecting_t;
extern MODEL_DATA *model_data_t;
extern EVALUATION_DATA *evaluation_data_t;
extern Eloquent::TF::Sequential<TF_NUM_OPS, ARENA_SIZE> tf;


/********************************************************
  Lokal deklarierte Instanzen
********************************************************/
armband myo;


/********************************************************
  Initialisierung Myo-Armband
********************************************************/
void initMyo()
{
  if (DEBUG_ELECTROMYOGRAPHY)
  {
    Serial.println("Inside initMyo");
    myo.debug = true;
  }
  else
  {
    myo.debug = false;
  }

  // Verbinde mit Armband
  myo.connect();

  // Setze Betriebsarten
  myo.set_myo_mode(myohw_emg_mode_send_emg,         // EMG mode ON
                   myohw_imu_mode_none,             // IMU mode OFF
                   myohw_classifier_mode_disabled); // Classifier mode OFF

  // Aktiviere Callback
  myo.emg_notification(TURN_ON)->subscribe(true, emgCallback);
  myo.set_sleep_mode(1);

  // Setze Tastenanforderung zurück
  myo_control_t->flag_connect_bluetooth = false;
}


/********************************************************
  Arduino Setup
********************************************************/
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

  /********************************************************
    Ein-/Ausgänge Machine Learning Modell konfigurieren
  ********************************************************/
  tf.setNumInputs(TF_NUM_INPUTS);
  tf.setNumOutputs(TF_NUM_OUTPUTS);

  /********************************************************
    Tensorflow Layer aus Model.h einlesen
  ********************************************************/
  registerNetworkOps(tf);

  while (!tf.begin(tfModel).isOk())
    Serial.println(tf.exception.toString());
}


/********************************************************
  Arduino Loop
********************************************************/
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
      Führe Evaluierung mit Testdaten aus
  ********************************************************/
  if ((evaluation_data_t->flag_start_evaluation_test == true) && ((evaluation_data_t->flag_loaded_testdata == true)))
  {
    evaluation_data_t->flag_start_evaluation_test = false;
    runTestConfusionMatrix(ROWS_OF_TESTDATA);
    sendSomewhat(PREFIX_EVAL + "_Start_Evaluation_Test", Evaluation_Topic, "false");
  }

  if ((evaluation_data_t->flag_start_evaluation_test == true) && (evaluation_data_t->flag_loaded_testdata == false))
  {
    evaluation_data_t->flag_start_evaluation_test = false;
    sendStatusText("Evaluierung mit Testdaten nicht möglich! Keine Daten geladen!");
    delay(5000);
    sendSomewhat(PREFIX_EVAL + "_Start_Evaluation_Test", Evaluation_Topic, "false");
  }

  /********************************************************
        Führe Evaluierung mit Validierungsdaten aus
    ********************************************************/
  if ((evaluation_data_t->flag_start_evaluation_validation == true) && ((evaluation_data_t->flag_loaded_validationdata == true)))
  {
    evaluation_data_t->flag_start_evaluation_validation = false;
    runValidationConfusionMatrix(ROWS_OF_VALIDATIONDATA);
    sendSomewhat(PREFIX_EVAL + "_Start_Evaluation_Validation", Evaluation_Topic, "false");
  }

  if ((evaluation_data_t->flag_start_evaluation_validation == true) && (evaluation_data_t->flag_loaded_validationdata == false))
  {
    evaluation_data_t->flag_start_evaluation_validation = false;
    sendStatusText("Evaluierung mit Validierungsdaten nicht möglich! Keine Daten geladen!");
    delay(5000);
    sendSomewhat(PREFIX_EVAL + "_Start_Evaluation_Validation", Evaluation_Topic, "false");
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

    if ((myo.connected == true) && (data_collecting_t->flag_traffic_light == false) && (evaluation_data_t->flag_start_classifying == false))
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

    if (evaluation_data_t->flag_start_classifying == false)
    {
      sendSomewhat(PREFIX_EVAL + "_Result_Classifier", Evaluation_Topic, "4");
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
    Armband ist verbunden und war es vorher nicht
  ********************************************************/
  if ((myo.connected == true) && (myo_control_t->flag_myo_connected == false))
  {
    myo_control_t->flag_myo_connected = true;
    myo_control_t->flag_connect_bluetooth = false;
  }

  /********************************************************
    Armband ist verbunden und kein Data-Collecting
  ********************************************************/
  if ((myo.connected == true) && (data_collecting_t->flag_traffic_light == false) && (evaluation_data_t->flag_classifying_light == false) && (evaluation_data_t->flag_start_classifying == false))
  {
    setNeoColor(0, 0, 255); // Blau
  }

  /********************************************************
    Armband ist nicht verbunden und war es vorher
  ********************************************************/
  if ((myo.connected == false) && (myo_control_t->flag_myo_connected == true))
  {
    sendStatusText("Myo-Armband getrennt!");
    setNeoColor(255, 0, 0); // Rot
    myo_control_t->flag_myo_connected = false;
    data_collecting_t->flag_start_collecting = false;
  }

  /********************************************************
    Armband ist nicht verbunden und war es vorher nicht
  ********************************************************/
  if ((myo.connected == false) && (myo_control_t->flag_myo_connected == false) && (myo_control_t->flag_connect_bluetooth == false))
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

  /********************************************************
    Gesten-Klassifizierung
  ********************************************************/
  if ((evaluation_data_t->flag_start_classifying == true) && (myo.connected == true) && (evaluation_data_t->flag_classifying_light == false))
  {
    classifyingLight();
  }

  if ((evaluation_data_t->flag_start_classifying == true) && (myo.connected == false))
  {
    sendSomewhat(PREFIX_EVAL + "_Start_Classifying", Evaluation_Topic, "false");
    sendStatusText("Klassifizierung nicht möglich!");
    delay(5000);
  }
}
