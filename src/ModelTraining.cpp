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
********************************************************/
u_int16_t input_shape[] = {1, FEATURES};

u_int16_t x_train_shape[] = {ROWS_OF_DATA, FEATURES};
aitensor_t x_train = AITENSOR_2D_F32(x_train_shape, input_data_t->fInput_Data);

/********************************************************
  Defintionen Fraunhofer AIfes Model-Output
********************************************************/
uint16_t y_train_shape[] = {ROWS_OF_DATA, NUM_OF_CLASSES};
aitensor_t y_train = AITENSOR_2D_F32(y_train_shape, target_data_t->fTarget_Label);

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
aiopti_adam_f32_t adam_opti = AIOPTI_ADAM_F32(0.01f, 0.9f, 0.999f, 1e-6f);

ailoss_crossentropy_f32_t cross;

/********************************************************
  Defintionen Fraunhofer AIfes Model-Aufbau
********************************************************/
aimodel_t model;
ailayer_t *x;
aiopti_t *optimizer;

/********************************************************
  Definition Globale Variablen Training
********************************************************/
uint32_t global_epoch_counter = 0;
float loss = 0;

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

  if (DEBUG_MODEL_BUILD)
  {
    Serial.println();
    Serial.println("----------------------------------------------");
    aialgo_print_loss_specs(model.loss);
    Serial.println("----------------------------------------------");
  }

  aialgo_compile_model(&model);

  /**********************************************************
    Speicher für Machine-Learning Modell ermitteln
  **********************************************************/
  uint32_t parameter_memory_size = aialgo_sizeof_parameter_memory(&model);

  /**********************************************************
    Speicherbedarf seriell ausgeben
  **********************************************************/
  if (DEBUG_MODEL_BUILD)
  {
    Serial.println();
    Serial.print("----------------------------------------------");
    Serial.println();
    Serial.print(F("Speicherplatzbedarf für alle Parameter (Weights, Bias, ...): "));
    Serial.print(parameter_memory_size);
    Serial.print(F(" Byte\n"));
    Serial.print("----------------------------------------------");
    Serial.println();
  }

  /**********************************************************
    Speicher für Machine-Learning Modell auf PSRAM allokieren
  **********************************************************/
  void *parameter_memory = heap_caps_malloc(parameter_memory_size, MALLOC_CAP_SPIRAM);

  if (DEBUG_MODEL_BUILD)
  {
    Serial.print("------------ AIfes Modell-Struktur -------------");
    Serial.println();
    aialgo_print_model_structure(&model);
    Serial.println();
    Serial.print("------------------------------------------------");
    Serial.println();
  }

  /**********************************************************
    Speicher auf trainierbare Parameter aufteilen
  **********************************************************/
  aialgo_distribute_parameter_memory(&model, parameter_memory, parameter_memory_size);

  /**********************************************************
    Parameter initalisieren
  **********************************************************/

  aimath_f32_default_init_he_uniform(&dense_layer_1.weights);
  aimath_f32_default_init_zeros(&dense_layer_1.bias);

  aimath_f32_default_init_he_uniform(&dense_layer_2.weights);
  aimath_f32_default_init_zeros(&dense_layer_2.bias);

  aimath_f32_default_init_he_uniform(&dense_layer_3.weights);
  aimath_f32_default_init_zeros(&dense_layer_3.bias);

  aimath_f32_default_init_glorot_uniform(&dense_layer_4.weights);
  aimath_f32_default_init_zeros(&dense_layer_4.bias);

  srand(time(0));
  aialgo_initialize_parameters_model(&model);

  /**********************************************************
    Optimizer definieren
  **********************************************************/

  optimizer = aiopti_adam_f32_default(&adam_opti);

  if (DEBUG_MODEL_BUILD)
  {
    Serial.println();
    Serial.println("----------------------------------------------");
    aialgo_print_optimizer_specs(optimizer);
    Serial.println();
    Serial.println("----------------------------------------------");
  }

  /**********************************************************
    Speicher für Training ermitteln
  **********************************************************/
  uint32_t working_memory_size = aialgo_sizeof_training_memory(&model, optimizer);

  /**********************************************************
    Speicherbedarf für Training seriell ausgeben
  **********************************************************/
  if (DEBUG_MODEL_BUILD)
  {
    Serial.println();
    Serial.println("----------------------------------------------");
    Serial.print(F("Speicherplatzbedarf Training (Zwischenergebnisse, Gradienten, Momenti): "));
    Serial.print(working_memory_size);
    Serial.print(F(" Byte\n"));
    Serial.print("----------------------------------------------\n\n");
  }

  /**********************************************************
    Speicher für Machine-Learning Modell auf PSRAM allokieren
  **********************************************************/
  void *working_memory = heap_caps_malloc(working_memory_size, MALLOC_CAP_SPIRAM);

  /**********************************************************
    Speicher für Training planen
  **********************************************************/
  aialgo_schedule_training_memory(&model, optimizer, working_memory, working_memory_size);

  /**********************************************************
    Modell für Training initalisieren
  **********************************************************/
  aialgo_init_model_for_training(&model, optimizer);
}


/********************************************************
  Training durchführen
********************************************************/
void runTraining()
{
  Serial.println("Das Training beginnt.");

  for (global_epoch_counter = 0; global_epoch_counter < EPOCHS; global_epoch_counter++)
  {
    aialgo_train_model(&model, &x_train, &y_train, optimizer, BATCH_SIZE);
  }

  
  if (global_epoch_counter % 10 == 0)
  {
    aialgo_calc_loss_model_f32(&model, &x_train, &y_train, &loss);
    printLoss(loss);
  }

  Serial.println("Training beendet.");
}