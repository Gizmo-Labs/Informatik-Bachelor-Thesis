#include <aifes.h>
#include "aifes_f32_weights.h"


/********************************************************
  Funktions-Prototypen WiFi
********************************************************/
uint8_t buildModel();
void runInferenz(float *input_data, float *output_data);
void runTraining(float *train_data, float *train_label);
void error_handling_training(int8_t error_nr);

/********************************************************
  Globale Konstanten Training-Daten
********************************************************/
#define ROWS_OF_DATA 1456
#define NUM_OF_CLASSES 4
#define FEATURES 64

#define DEBUG_MODEL_TRAINING true
#define DEBUG_MODEL_BUILD true


/********************************************************
  Deklaration Struct Training-Daten
********************************************************/
typedef struct
{
  int iTarget_Label[ROWS_OF_DATA][NUM_OF_CLASSES];
} TARGET_DATA;


/********************************************************
  Deklaration Struct Ausgabe-Daten
********************************************************/
typedef struct
{
  float fOutput_Data[ROWS_OF_DATA][NUM_OF_CLASSES];
} OUTPUT_DATA;



/********************************************************
  Deklaration Struct Input-Daten
********************************************************/
typedef struct
{
  float fInput_Data[ROWS_OF_DATA][FEATURES];
} INPUT_DATA;