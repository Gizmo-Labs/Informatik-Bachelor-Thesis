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
void readLabels()

/********************************************************
  Globale Konstanten File-Handling  
********************************************************/
#define FORMAT_FFAT_IF_FAILED false


/********************************************************
  Deklaration CSV-Parser
********************************************************/
CSV_Parser cp(/*format*/ "dddd", /*has_header*/ false, /*delimiter*/ ',');