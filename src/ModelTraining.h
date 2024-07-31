#include <aifes.h>

/********************************************************
  Globale Konstanten Training-Daten
********************************************************/
#define ROWS_OF_DATA 6064
#define NUM_OF_CLASSES 4
#define FEATURES 64
#define DEBUG_MODEL_TRAINING false
#define PRINT_INTERVAL 10

/********************************************************
  Deklaration Struct Training-Daten
********************************************************/
typedef struct
{
  uint16_t iTarget_Label[ROWS_OF_DATA][NUM_OF_CLASSES];
} TARGET_DATA;


/********************************************************
  Deklaration Struct Input-Daten
********************************************************/
typedef struct
{
  float fInput_Data[ROWS_OF_DATA][FEATURES];
} INPUT_DATA;