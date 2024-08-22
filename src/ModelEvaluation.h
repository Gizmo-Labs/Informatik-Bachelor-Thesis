/********************************************************
  Bibliotheken
********************************************************/
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
#define ROWS_OF_TESTDATA 1000
#define ROWS_OF_VALIDATIONDATA 500
#define NUM_OF_CLASSES 3
#define FEATURES 64


/********************************************************
  Deklaration Struct Test-Daten
********************************************************/
typedef struct
{
  float fTest_Data[ROWS_OF_TESTDATA][FEATURES];
  float fValidation_Data[ROWS_OF_VALIDATIONDATA][FEATURES];
  int iTest_Label[ROWS_OF_TESTDATA][NUM_OF_CLASSES];
  int iValidation_Label[ROWS_OF_VALIDATIONDATA][NUM_OF_CLASSES];
} MODEL_DATA;


/********************************************************
  Deklaration Struct Evalution
********************************************************/
typedef struct
{
  volatile bool flag_start_classifying;
  volatile bool flag_classifying_light;
  volatile bool flag_start_evaluation_test;
  volatile bool flag_start_evaluation_validation;
  volatile bool flag_load_testdata;
  volatile bool flag_load_validationdata;  
  volatile bool flag_loaded_testdata;
  volatile bool flag_loaded_testinputs;
  volatile bool flag_loaded_testlabels;
  volatile bool flag_loaded_validationdata;
  volatile bool flag_loaded_validationinputs;
  volatile bool flag_loaded_validationlabels;
} EVALUATION_DATA;
