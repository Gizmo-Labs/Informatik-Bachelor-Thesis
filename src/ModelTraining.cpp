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
  Globale Konstanten Fraunhofer AIfes Model-Input
********************************************************/
u_int16_t input_shape[] = {ROWS_OF_DATA, FEATURES};
aitensor_t input_tensor = AITENSOR_2D_F32(input_shape, input_data_t->fInput_Data);


/********************************************************
  Globale Konstanten Fraunhofer AIfes Model-Output
********************************************************/
uint16_t target_shape[] = {ROWS_OF_DATA, NUM_OF_CLASSES};
aitensor_t target_tensor = AITENSOR_2D_F32(target_shape, target_data_t->iTarget_Label);


/********************************************************
  Globale Konstanten Fraunhofer AIfes Model-Layers
********************************************************/
ailayer_input_f32_t input_layer   = AILAYER_INPUT_F32_A(2, input_shape);
ailayer_dense_f32_t dense_layer_1 = AILAYER_DENSE_F32_A(512);
ailayer_dense_f32_t dense_layer_2 = AILAYER_DENSE_F32_A(256);
ailayer_dense_f32_t dense_layer_3 = AILAYER_DENSE_F32_A(64);
ailayer_dense_f32_t dense_layer_4 = AILAYER_DENSE_F32_A(NUM_OF_CLASSES);

ailayer_batch_norm_f32_t batch_norm_1 = AILAYER_BATCH_NORM_F32_A(/* momentum =*/ 0.9f, /* eps =*/ 1e-6f);


/********************************************************
  Globale Konstanten Fraunhofer AIfes Model-Aufbau
********************************************************/
aimodel_t model;


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
