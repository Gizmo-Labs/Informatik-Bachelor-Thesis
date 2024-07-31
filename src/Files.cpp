/********************************************************
  Bibliotheken
********************************************************/
#include "Prototypes.h"

void readFile(fs::FS &fs, const char *path)
{
    Serial.printf("Lese Datei : %s\r\n", path);

    static uint8_t buf[512];
    size_t len = 0;

    File file = fs.open(path);
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

        Serial.println("");
        end = millis() - start;
        Serial.printf("- %u bytes read in %lu ms\r\n", flen, end);
        file.close();

        // Serial.println("- Lese aus der Datei :");
        // while (file.available())
        // {
        //     Serial.write(file.read());
        // }
        file.close();
    }
}

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

void readLabels()
{
    if(cp.readFile(FFat, "/Labels.csv"))
    {
        
    }
  
}