/********************************************************
  Bibliotheken
********************************************************/
#include "Prototypes.h"


/********************************************************
  Array für Training-Labels auf PSRAM allokieren
********************************************************/
TARGET_DATA *target_data_t = (TARGET_DATA *)heap_caps_malloc(sizeof(TARGET_DATA), MALLOC_CAP_SPIRAM);
OUTPUT_DATA *output_data_t = (OUTPUT_DATA *)heap_caps_malloc(sizeof(OUTPUT_DATA), MALLOC_CAP_SPIRAM);


/********************************************************
  Array für Input-Daten auf PSRAM allokieren
********************************************************/
INPUT_DATA *input_data_t = (INPUT_DATA *)heap_caps_malloc(sizeof(INPUT_DATA), MALLOC_CAP_SPIRAM);


/********************************************************
  Definitionen Fraunhofer AIfes Model-Input
********************************************************/
u_int16_t input_shape[] = {1, FEATURES};
u_int16_t output_shape[] = {1, NUM_OF_CLASSES};

u_int16_t x_train_shape[] = {10, FEATURES};
u_int16_t y_train_shape[] = {10, NUM_OF_CLASSES};


/********************************************************
  Defintionen Fraunhofer AIfes Model-Output
********************************************************/
ailayer_input_f32_t input_layer = AILAYER_INPUT_F32_A(2, input_shape);
ailayer_dense_f32_t dense_layer_1 = AILAYER_DENSE_F32_A(32);
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
  Erstellen des Machine-Learning Modells
********************************************************/
uint8_t buildModel()
{

  model.input_layer = ailayer_input_f32_default(&input_layer);

  x = ailayer_dense_wt_f32_default(&dense_layer_1, model.input_layer);
  batch_norm_1.base.channel_axis = 1;

  x = ailayer_batch_norm_f32_default(&batch_norm_1, x);
  x = ailayer_relu_f32_default(&relu_1, x);

  x = ailayer_dense_wt_f32_default(&dense_layer_2, x);
  batch_norm_2.base.channel_axis = 1;
  x = ailayer_batch_norm_f32_default(&batch_norm_2, x);
  x = ailayer_relu_f32_default(&relu_2, x);

  x = ailayer_dense_wt_f32_default(&dense_layer_3, x);
  batch_norm_3.base.channel_axis = 1;
  x = ailayer_batch_norm_f32_default(&batch_norm_3, x);
  x = ailayer_relu_f32_default(&relu_3, x);

  x = ailayer_dense_wt_f32_default(&dense_layer_4, x);
  model.output_layer = ailayer_softmax_f32_default(&softmax, x);

  model.loss = ailoss_crossentropy_f32_default(&cross, model.output_layer);

  if (DEBUG_MODEL_BUILD)
  {
    // Serial.println();
    Serial.println("----------------------------------------------");
    aialgo_print_loss_specs(model.loss);
    // Serial.println("----------------------------------------------");
  }

  aialgo_compile_model(&model);

  /**********************************************************
    Speicher für Machine-Learning Modell ermitteln
  **********************************************************/
  uint32_t parameter_memory_size = aialgo_sizeof_parameter_memory(&model);
  aialgo_distribute_parameter_memory(&model, (void *)aifes_f32_flat_weights, parameter_memory_size);

  /**********************************************************
    Speicher für Machine-Learning Modell auf PSRAM allokieren
  **********************************************************/
  uint32_t memory_size = aialgo_sizeof_inference_memory(&model);
  void *memory_ptr = heap_caps_malloc(memory_size, MALLOC_CAP_SPIRAM);

  if (memory_ptr == NULL)
  {
    AIPRINT("Nicht genug Inferenz-Speicher.");
    return 1;
  }

  /**********************************************************
      Speicherbedarf seriell ausgeben
    **********************************************************/
  if (DEBUG_MODEL_BUILD)
  {
    Serial.println();
    Serial.print("----------------------------------------------");
    Serial.println();
    Serial.print(F("Speicherplatzbedarf Inferenz-Speicher"));
    AIPRINT_UINT("%u", memory_size);
    Serial.print(F(" Byte\n"));
    Serial.print("----------------------------------------------");
    Serial.println();
  }

  if (DEBUG_MODEL_BUILD)
  {
    Serial.print("------------ AIfes Modell-Struktur -------------");
    Serial.println();
    aialgo_print_model_structure(&model);
    Serial.println();
    Serial.print("------------------------------------------------");
    Serial.println();
  }

  aialgo_schedule_inference_memory(&model, memory_ptr, memory_size);

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
  uint32_t training_memory_size = aialgo_sizeof_training_memory(&model, optimizer);

  /**********************************************************
    Speicherbedarf für Training seriell ausgeben
  **********************************************************/
  if (DEBUG_MODEL_BUILD)
  {
    Serial.println();
    Serial.println("----------------------------------------------");
    Serial.print(F("Speicherplatzbedarf Training (Zwischenergebnisse, Gradienten, Momenti): "));
    AIPRINT_UINT("%u", training_memory_size);
    Serial.print(F(" Byte\n"));
    Serial.print("----------------------------------------------\n\n");
  }

  /**********************************************************
    Speicher für Machine-Learning Modell auf PSRAM allokieren
  **********************************************************/
  void *training_memory = heap_caps_malloc(training_memory_size, MALLOC_CAP_SPIRAM);

  if (training_memory == NULL)
  {
    AIPRINT("Nicht genug Speicher fuer Training-Speicher.");
    return 1;
  }

  /**********************************************************
    Speicher für Training planen
  **********************************************************/
  aialgo_schedule_training_memory(&model, optimizer, training_memory, training_memory_size);

  /**********************************************************
    Modell für Training initalisieren
  **********************************************************/
  aialgo_init_model_for_training(&model, optimizer);

  return 0;
}

/********************************************************
  Training durchführen
********************************************************/
void runInferenz(float *input_data, float *output_data)
{
  aitensor_t input_tensor = AITENSOR_2D_F32(input_shape, input_data);
  aitensor_t output_tensor = AITENSOR_2D_F32(output_shape, output_data);

  aialgo_inference_model(&model, &input_tensor, &output_tensor);

  // AIPRINT("Eingabe : \n");
  // print_aitensor(&input_tensor);
  // AIPRINT("NN Ausgabe : \n");
  // print_aitensor(&output_tensor);
  // Serial.println();
}

/********************************************************
  Training durchführen
********************************************************/
void runTraining(float *train_data, float *train_label)
{
  float loss;

  uint16_t i;
  uint32_t batch_size = 5;
  uint32_t epochs = 5;

  aitensor_t x_train = AITENSOR_2D_F32(x_train_shape, train_data);
  aitensor_t y_train = AITENSOR_2D_F32(y_train_shape, train_label);

  optimizer = aiopti_adam_f32_default(&adam_opti);

  if(DEBUG_MODEL_TRAINING)
  {
   AIPRINT("X-Train : \n");
   print_aitensor(&x_train);
   AIPRINT("Y-Train : \n");
   print_aitensor(&y_train);
   Serial.println();
  }

  delay(5000);

  Serial.println("Training startet!");

  for (int i = 0; i < epochs; i++)
  {
    int8_t error = 0;

    error = aialgo_train_model(&model, &x_train, &y_train, optimizer, batch_size);
    error_handling_training(error);

    // Calculate and print loss every 1 epochs
    if (i % 1 == 0)
    {
      aialgo_calc_loss_model_f32(&model, &x_train, &y_train, &loss);
      Serial.print("Epoch ");
      Serial.print(i);
      Serial.print(":  loss: ");
      Serial.print(loss);
      Serial.println();
    }
  }
  Serial.println("Training beendet!");
}

void error_handling_training(int8_t error_nr)
{
  switch (error_nr)
  {
  case 0:
    // Serial.println(F("No Error :)"));
    break;
  case -1:
    Serial.println(F("ERROR! Tensor dtype"));
    break;
  case -2:
    Serial.println(F("ERROR! Tensor shape: Data Number"));
    break;
  case -3:
    Serial.println(F("ERROR! Input tensor shape does not correspond to ANN inputs"));
    break;
  case -4:
    Serial.println(F("ERROR! Output tensor shape does not correspond to ANN outputs"));
    break;
  case -5:
    Serial.println(F("ERROR! Use the crossentropy as loss for softmax"));
    break;
  case -6:
    Serial.println(F("ERROR! learn_rate or sgd_momentum negative"));
    break;
  case -7:
    Serial.println(F("ERROR! Init uniform weights min - max wrong"));
    break;
  case -8:
    Serial.println(F("ERROR! batch_size: min = 1 / max = Number of training data"));
    break;
  case -9:
    Serial.println(F("ERROR! Unknown activation function"));
    break;
  case -10:
    Serial.println(F("ERROR! Unknown loss function"));
    break;
  case -11:
    Serial.println(F("ERROR! Unknown init weights method"));
    break;
  case -12:
    Serial.println(F("ERROR! Unknown optimizer"));
    break;
  case -13:
    Serial.println(F("ERROR! Not enough memory"));
    break;
  default:
    Serial.println(F("Unknown error"));
  }
}