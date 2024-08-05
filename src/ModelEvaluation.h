/********************************************************
  Bibliotheken
********************************************************/
// #include "model.h"
#include "Model.h"
#include <tflm_esp32.h>
#include <eloquent_tinyml.h>

/********************************************************
  Funktions-Prototypen
********************************************************/
void runTestConfusionMatrix(int rows);
void runValidationConfusionMatrix(int rows);
void runClassifier();


/********************************************************
  Globale Konstanten
********************************************************/
#define ARENA_SIZE 50000
#define DEBUG_EVALUATION false
#define ROWS_OF_TESTDATA 1213
#define ROWS_OF_VALIDATIONDATA 1456
#define NUM_OF_CLASSES 4
#define FEATURES 64

/********************************************************
  Deklaration Struct Test-Daten
********************************************************/
typedef struct
{
  float fTest_Data[ROWS_OF_TESTDATA][FEATURES];
  int iTest_Label[ROWS_OF_TESTDATA][NUM_OF_CLASSES];
  float fValidation_Data[ROWS_OF_VALIDATIONDATA][FEATURES];
  int iValidation_Label[ROWS_OF_VALIDATIONDATA][NUM_OF_CLASSES];
} MODEL_DATA;

/********************************************************
  Deklaration Struct Evalution
********************************************************/
typedef struct
{
  volatile bool flag_start_classifying;
  volatile bool flag_classifying_light;
  volatile bool flag_start_evaluation;
  volatile bool flag_load_testdata;
  volatile bool flag_load_validationdata;  
  volatile bool flag_loaded_testdata;
  volatile bool flag_loaded_testinputs;
  volatile bool flag_loaded_testlabels;
  volatile bool flag_loaded_validationdata;
  volatile bool flag_loaded_validationinputs;
  volatile bool flag_loaded_validationlabels;
} EVALUATION_DATA;
