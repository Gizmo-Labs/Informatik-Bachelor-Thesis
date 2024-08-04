/********************************************************
  Bibliotheken
********************************************************/
//#include "model.h"
#include "Model.h"
#include <tflm_esp32.h>
#include <eloquent_tinyml.h>


/********************************************************
  Funktions-Prototypen 
********************************************************/
void runConfusionMatrix();


/********************************************************
  Globale Konstanten 
********************************************************/
#define ARENA_SIZE 50000
#define DEBUG_EVALUATION false
#define ROWS_OF_DATA 1456
#define NUM_OF_CLASSES 4
#define FEATURES 64


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

