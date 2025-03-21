/********************************************************
  Bibliotheken
********************************************************/
#pragma once
#include "FFat.h"
#include "FS.h"
#include <CSV_Parser.h>


/********************************************************
  Funktions-Prototypen File-Handling
********************************************************/
void readFile(fs::FS &fs, const char *path);
void readTestLabels();
void readTestInput();
void readValidationLabels();
void readValidationInput();


/********************************************************
  Globale Konstanten File-Handling  
********************************************************/
#define FORMAT_FFAT_IF_FAILED false
#define DEBUG_LABEL_FILE_HANDLING true
#define DEBUG_FEATURE_FILE_HANDLING true


/********************************************************
  Deklaration CSV-Parser
********************************************************/
extern CSV_Parser cp;
extern File file;