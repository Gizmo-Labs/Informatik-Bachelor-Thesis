/********************************************************
  Globale Konstanten 
********************************************************/
#define LINES_OF_DATA 6064
#define NUM_OF_CLASSES 4

#define DEBUG_MODEL_TRAINING false

/********************************************************
  Deklaration Struct Training-Daten
********************************************************/
typedef struct 
{
  uint16_t iTarget_Label[LINES_OF_DATA][NUM_OF_CLASSES];  
} TARGET_DATA;




