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
  Das funktioniert!!
********************************************************/
uint16_t target_shape[] = {ROWS_OF_DATA, NUM_OF_CLASSES};
aitensor_t target_tensor = AITENSOR_2D_F32(target_shape, target_data_t->iTarget_Label);

/********************************************************
  Dieser Block funktioniert, ist das original Beispiel!!
********************************************************/
// ailayer_input_f32_t input_layer = AILAYER_INPUT_F32_A(2, input_layer_shape);
// ailayer_dense_f32_t dense_layer_1 = AILAYER_DENSE_F32_A(3);
// ailayer_leaky_relu_f32_t leaky_relu_layer = AILAYER_LEAKY_RELU_F32_A(0.01f);
// ailayer_dense_f32_t dense_layer_2 = AILAYER_DENSE_F32_A(2);
// ailayer_sigmoid_f32_t sigmoid_layer = AILAYER_SIGMOID_F32_A();

/********************************************************
  Das funktioniert auch noch!!
********************************************************/
ailayer_input_f32_t input_layer = AILAYER_INPUT_F32_A(2, input_shape);
ailayer_dense_f32_t dense_layer_1 = AILAYER_DENSE_F32_A(2);
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
  /********************************************************
  Sobald ich diese Zeile einkommentiere ist Feierabend!!
  ********************************************************/
  model.input_layer = ailayer_input_f32_default(&input_layer);

  x = ailayer_dense(&dense_layer_1, model.input_layer);
  x = ailayer_batch_norm_f32_default(&batch_norm_1, x);
  x = ailayer_relu_f32_default(&relu_1, x);

  // add = ailayer_dense(&dense_layer_2, add);
  // add = ailayer_batch_norm_f32_default(&batch_norm_2 ,add);
  // add = ailayer_relu_f32_default(&relu_2, add);

  // add = ailayer_dense(&dense_layer_3, add);
  // add = ailayer_batch_norm_f32_default(&batch_norm_3, add);
  // add = ailayer_relu_f32_default(&relu_3, add);

  x = ailayer_dense(&dense_layer_4, x);
  x = ailayer_softmax_f32_default(&softmax, x);

  model.output_layer = x;

  // model.loss = ailoss_crossentropy_f32_default(&cross, model.output_layer);

  /********************************************************
   Dieser Block funktioniert, ist das original Beispiel!!
  ********************************************************/
  // model.input_layer = ailayer_input_f32_default(&input_layer);
  // x = ailayer_dense_f32_default(&dense_layer_1, model.input_layer);
  // x = ailayer_leaky_relu_f32_default(&leaky_relu_layer, x);
  // x = ailayer_dense_f32_default(&dense_layer_2, x);
  // x = ailayer_sigmoid_f32_default(&sigmoid_layer, x);
  // model.output_layer = x;

  aialgo_compile_model(&model);

  if (DEBUG_MODEL_BUILD)
  {
    Serial.print("-------------- Model structure ---------------\n");
    aialgo_print_model_structure(&model);
    Serial.print("----------------------------------------------\n\n");
  }
}
