/********************************************************
  Bibliotheken
********************************************************/
#include "Prototypes.h"

/********************************************************
  Array für Training-Labels auf PSRAM allokieren
********************************************************/
TARGET_DATA *target_data_t = (TARGET_DATA *)heap_caps_malloc(sizeof(TARGET_DATA), MALLOC_CAP_SPIRAM);

/********************************************************
  Array für Input-Daten auf PSRAM allokieren
********************************************************/
INPUT_DATA *input_data_t = (INPUT_DATA *)heap_caps_malloc(sizeof(INPUT_DATA), MALLOC_CAP_SPIRAM);

/********************************************************
  Definitionen Fraunhofer AIfes Model-Input
  Das funktioniert!!
********************************************************/
u_int16_t input_shape[] = {ROWS_OF_DATA, FEATURES};
u_int16_t input_layer_shape[] = {1, 3};
aitensor_t input_tensor = AITENSOR_2D_F32(input_shape, input_data_t->fInput_Data);

/********************************************************
  Defintionen Fraunhofer AIfes Model-Output
********************************************************/
uint16_t target_shape[] = {ROWS_OF_DATA, NUM_OF_CLASSES};
aitensor_t target_tensor = AITENSOR_2D_F32(target_shape, target_data_t->iTarget_Label);

ailayer_input_f32_t input_layer = AILAYER_INPUT_F32_A(2, input_shape);
ailayer_dense_f32_t dense_layer_1 = AILAYER_DENSE_F32_A(512);
ailayer_dense_f32_t dense_layer_2 = AILAYER_DENSE_F32_A(256);
ailayer_dense_f32_t dense_layer_3 = AILAYER_DENSE_F32_A(64);
ailayer_dense_f32_t dense_layer_4 = AILAYER_DENSE_F32_A(NUM_OF_CLASSES);

ailayer_batch_norm_f32_t batch_norm_1 = AILAYER_BATCH_NORM_F32_A(/* momentum =*/0.9f, /* eps =*/1e-6f);
ailayer_batch_norm_f32_t batch_norm_2 = AILAYER_BATCH_NORM_F32_A(/* momentum =*/0.9f, /* eps =*/1e-6f);
ailayer_batch_norm_f32_t batch_norm_3 = AILAYER_BATCH_NORM_F32_A(/* momentum =*/0.9f, /* eps =*/1e-6f);

ailayer_relu_f32_t relu_1 = AILAYER_RELU_F32_A();
ailayer_relu_f32_t relu_2 = AILAYER_RELU_F32_A();
ailayer_relu_f32_t relu_3 = AILAYER_RELU_F32_A();

ailayer_softmax_f32_t softmax = AILAYER_SOFTMAX_F32_A();

ailoss_crossentropy_f32_t cross;

/********************************************************
  Defintionen Fraunhofer AIfes Model-Aufbau
********************************************************/
aimodel_t model;
ailayer_t *x;

/********************************************************
  Definition Globale Variablen Training
********************************************************/
uint32_t global_epoch_counter = 0;

/********************************************************
  Serielle Ausgabe der Trainingsverluste
********************************************************/
void printLoss(float loss)
{
  global_epoch_counter = global_epoch_counter + 1;
  Serial.print(F("Epoche: "));
  Serial.print(global_epoch_counter * PRINT_INTERVAL);
  Serial.print(F(" / Verlust: "));
  Serial.println(loss, 5);
}

/********************************************************
  Erstellen des Machine-Learning Modells
********************************************************/
void buildModel()
{
  
  model.input_layer = ailayer_input_f32_default(&input_layer);

  x = ailayer_dense_f32_default(&dense_layer_1, model.input_layer);
  x = ailayer_batch_norm_f32_default(&batch_norm_1, x);
  x = ailayer_relu_f32_default(&relu_1, x);

  x = ailayer_dense_f32_default(&dense_layer_2, x);
  x = ailayer_batch_norm_f32_default(&batch_norm_2, x);
  x = ailayer_relu_f32_default(&relu_2, x);

  x = ailayer_dense_f32_default(&dense_layer_3, x);
  x = ailayer_batch_norm_f32_default(&batch_norm_3, x);
  x = ailayer_relu_f32_default(&relu_3, x);

  x = ailayer_dense_f32_default(&dense_layer_4, x);
  x = ailayer_softmax_f32_default(&softmax, x);

  model.output_layer = x;

  model.loss = ailoss_crossentropy_f32_default(&cross, model.output_layer);

  aialgo_compile_model(&model);


  /**********************************************************
    Speicher für Machine-Learning Modell ermitteln
  **********************************************************/
  uint32_t parameter_memory_size = aialgo_sizeof_parameter_memory(&model);
  

  /**********************************************************
    Speicherbedarf seriell ausgeben
  **********************************************************/
  Serial.println();
  Serial.print(F("Speicherplatzbedarf für alle Parameter (Weights, Bias, ...): "));
  Serial.print(parameter_memory_size);
  Serial.print(F(" Byte\n"));
  Serial.println();

  /**********************************************************
    Speicher für Machine-Learning Modell auf PSRAM allokieren
  **********************************************************/
  void *parameter_memory = heap_caps_malloc(parameter_memory_size, MALLOC_CAP_SPIRAM);
  
  if (DEBUG_MODEL_BUILD)
  {
    Serial.print("------------ AIfes Modell-Struktur -------------\n");
    Serial.println();
    aialgo_print_model_structure(&model);
    Serial.println();
    Serial.print("----------------------------------------------\n\n");
    Serial.println();
  }
}
