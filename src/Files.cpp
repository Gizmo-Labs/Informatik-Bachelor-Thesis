/********************************************************
  Bibliotheken
********************************************************/
#include "Prototypes.h"


/********************************************************
  Extern deklarierte Instanzen
********************************************************/
extern TARGET_DATA *target_data_t;


/********************************************************
  Lokal deklarierte Instanzen
********************************************************/
File file;


/********************************************************
  Datei von Dateisystem lesen
********************************************************/
void readFile(fs::FS &fs, const char *path)
{
    Serial.printf("Lese Datei : %s\r\n", path);

    static uint8_t buf[512];
    size_t len = 0;

    file = fs.open(path);
    if (!file || file.isDirectory())
    {
        Serial.println("- Öffnen der Datei fehlgeschlagen");
        return;
    }

    uint32_t start = millis();
    uint32_t end = start;

    int i = 0;

    if (file && !file.isDirectory())
    {
        len = file.size();
        size_t flen = len;
        start = millis();
        Serial.print("- Lese aus der Datei :");
        while (len)
        {
            size_t toRead = len;
            if (toRead > 512)
            {
                toRead = 512;
            }
            file.read(buf, toRead);
            if ((i++ & 0x001F) == 0x001F)
            {
                Serial.print(".");
            }
            len -= toRead;
        }

        Serial.println("- Lese aus der Datei :");
        while (file.available())
        {
            Serial.write(file.read());
        }
        file.close();
    }
}


/********************************************************
  Datei auf Dateisystem schreiben
********************************************************/
void writeFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("- failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
        Serial.println("- file written");
    }
    else
    {
        Serial.println("- write failed");
    }
    file.close();
}


/********************************************************
  Inhalte an bestehende Datei anhängen
********************************************************/
void appendFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file)
    {
        Serial.println("- failed to open file for appending");
        return;
    }
    if (file.print(message))
    {
        Serial.println("- message appended");
    }
    else
    {
        Serial.println("- append failed");
    }
    file.close();
}


/********************************************************
  Nur zu Testzwecken
********************************************************/
void testFileIO(fs::FS &fs, const char *path)
{
    Serial.printf("Testing file I/O with %s\r\n", path);

    static uint8_t buf[512];
    size_t len = 0;
    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("- failed to open file for writing");
        return;
    }

    size_t i;
    Serial.print("- writing");
    uint32_t start = millis();
    for (i = 0; i < 2048; i++)
    {
        if ((i & 0x001F) == 0x001F)
        {
            Serial.print(".");
        }
        file.write(buf, 512);
    }
    Serial.println("");
    uint32_t end = millis() - start;
    Serial.printf(" - %u bytes written in %lu ms\r\n", 2048 * 512, end);
    file.close();

    file = fs.open(path);
    start = millis();
    end = start;
    i = 0;
    if (file && !file.isDirectory())
    {
        len = file.size();
        size_t flen = len;
        start = millis();
        Serial.print("- reading");
        while (len)
        {
            size_t toRead = len;
            if (toRead > 512)
            {
                toRead = 512;
            }
            file.read(buf, toRead);
            if ((i++ & 0x001F) == 0x001F)
            {
                Serial.print(".");
            }
            len -= toRead;
        }
        Serial.println("");
        end = millis() - start;
        Serial.printf("- %u bytes read in %lu ms\r\n", flen, end);
        file.close();
    }
    else
    {
        Serial.println("- failed to open file for reading");
    }
}


/********************************************************
  Hilfsfunktion von CSV-Parser
********************************************************/
char feedRowParser()
{
    return file.read();
}


/********************************************************
  Hilfsfunktion von CSV-Parser
********************************************************/
bool rowParserFinished()
{
    return ((file.available() > 0) ? false : true);
}


/********************************************************
  Training-Labels vom Dateisystem einlesen
********************************************************/
void readLabels()
{
    CSV_Parser cp(/*format*/ "dddd", /*has_header*/ false);

    file = FFat.open("/Labels.csv", "r");

    uint16_t *label_0 = (uint16_t *)cp[0];
    uint16_t *label_1 = (uint16_t *)cp[1];
    uint16_t *label_2 = (uint16_t *)cp[2];
    uint16_t *label_3 = (uint16_t *)cp[3];

    int row_index = 0;
    uint32_t start = millis();
    while (cp.parseRow())
    {
        target_data_t->iTarget_Label[row_index][0] = label_0[0];
        target_data_t->iTarget_Label[row_index][1] = label_1[0];
        target_data_t->iTarget_Label[row_index][2] = label_2[0];
        target_data_t->iTarget_Label[row_index][3] = label_3[0];

        row_index++;
    }

    uint32_t end = millis() - start;

    if (DEBUG_FILE_HANDLING)
    {
        Serial.println("Parsed Rows : " + String(row_index));
        Serial.printf(" Took %lu ms\r\n", end);
    }

    file.close();

    delay(2000);

    if (DEBUG_FILE_HANDLING)
    {
        for (int j = 0; j < row_index; j++)
        {
            Serial.println("Reihe : " + String(j + 1) + " | " + String(target_data_t->iTarget_Label[j][0]) + " | " + String(target_data_t->iTarget_Label[j][1]) + " | " + String(target_data_t->iTarget_Label[j][2]) + " | " + String(target_data_t->iTarget_Label[j][3]));
        }
    }
}
