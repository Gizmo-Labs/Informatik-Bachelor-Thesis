/********************************************************
  Bibliotheken
********************************************************/
#pragma once
#include <LittleFS.h>
#include "FS.h"


/********************************************************
  Funktions-Prototypen File-Handling
********************************************************/
void WiFiStart();
void connectToWifi();
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
void createDir(fs::FS &fs, const char *path);
void removeDir(fs::FS &fs, const char *path);
void readFile(fs::FS &fs, const char *path);
void writeFile(fs::FS &fs, const char *path, const char *message);
void appendFile(fs::FS &fs, const char *path, const char *message);
void renameFile(fs::FS &fs, const char *path1, const char *path2);
void deleteFile(fs::FS &fs, const char *path);
void writeFile2(fs::FS &fs, const char *path, const char *message);
void deleteFile2(fs::FS &fs, const char *path);
void testFileIO(fs::FS &fs, const char *path);


/********************************************************
  Globale Konstanten File-Handling  
********************************************************/
#define FORMAT_LITTLEFS_IF_FAILED true
