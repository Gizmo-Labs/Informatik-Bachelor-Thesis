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