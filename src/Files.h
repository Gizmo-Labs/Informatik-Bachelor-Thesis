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
void writeFile(fs::FS &fs, const char *path, const char *message);
void appendFile(fs::FS &fs, const char *path, const char *message);
void readLabels();
void readInput();


/********************************************************
  Globale Konstanten File-Handling  
********************************************************/
#define FORMAT_FFAT_IF_FAILED false
#define DEBUG_FILE_HANDLING true


/********************************************************
  Deklaration CSV-Parser
********************************************************/
extern CSV_Parser cp;
extern File file;