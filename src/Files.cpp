/********************************************************
  Bibliotheken
********************************************************/
#include "Prototypes.h"

/********************************************************
  Extern deklarierte Instanzen
********************************************************/
extern MODEL_DATA *model_data_t;
extern EVALUATION_DATA *evaluation_data_t;

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
  Test-Labels vom Dateisystem einlesen
********************************************************/
void readTestLabels()
{
    sendStatusText("Lade Test-Label Datensätze! Dauer max. 1 Minute!");

    CSV_Parser cp(/*format*/ "dddd", /*has_header*/ false);

    file = FFat.open("/y_test.csv", "r");

    uint16_t *label_0 = (uint16_t *)cp[0];
    uint16_t *label_1 = (uint16_t *)cp[1];
    uint16_t *label_2 = (uint16_t *)cp[2];
    uint16_t *label_3 = (uint16_t *)cp[3];

    int row_index = 0;

    uint32_t start = millis();

    while (cp.parseRow())
    {
        model_data_t->iTest_Label[row_index][0] = label_0[0];
        model_data_t->iTest_Label[row_index][1] = label_1[0];
        model_data_t->iTest_Label[row_index][2] = label_2[0];
        model_data_t->iTest_Label[row_index][3] = label_3[0];

        row_index++;
    }

    uint32_t end = millis() - start;

    if (DEBUG_LABEL_FILE_HANDLING)
    {
        Serial.println();
        Serial.println("Test-Labels");
        Serial.println();
        Serial.println("Geparste Reihen : " + String(row_index));
        Serial.printf(" Zeit : %lu ms\r\n", end);
        Serial.println();
    }

    file.close();

    evaluation_data_t->flag_loaded_testlabels = true;

    if (DEBUG_LABEL_FILE_HANDLING)
    {
        delay(2000);
        for (int j = 0; j < 20; j++)
        {
            Serial.println("Reihe : " + String(j + 1) + " | " + String(model_data_t->iTest_Label[j][0]) + " | " + String(model_data_t->iTest_Label[j][1]) + " | " + String(model_data_t->iTest_Label[j][2]) + " | " + String(model_data_t->iTest_Label[j][3]));
        }
    }
}


/********************************************************
  Validation-Labels vom Dateisystem einlesen
********************************************************/
void readValidationLabels()
{
    sendStatusText("Lade Validation-Label Datensätze! Dauer max. 1 Minute!");

    CSV_Parser cp(/*format*/ "dddd", /*has_header*/ false);

    file = FFat.open("/y_val.csv", "r");

    uint16_t *label_0 = (uint16_t *)cp[0];
    uint16_t *label_1 = (uint16_t *)cp[1];
    uint16_t *label_2 = (uint16_t *)cp[2];
    uint16_t *label_3 = (uint16_t *)cp[3];

    int row_index = 0;

    uint32_t start = millis();

    while (cp.parseRow())
    {
        model_data_t->iValidation_Label[row_index][0] = label_0[0];
        model_data_t->iValidation_Label[row_index][1] = label_1[0];
        model_data_t->iValidation_Label[row_index][2] = label_2[0];
        model_data_t->iValidation_Label[row_index][3] = label_3[0];

        row_index++;
    }

    uint32_t end = millis() - start;

    if (DEBUG_LABEL_FILE_HANDLING)
    {
        Serial.println();
        Serial.println("Validation-Labels");
        Serial.println();
        Serial.println("Geparste Reihen : " + String(row_index));
        Serial.printf(" Zeit : %lu ms\r\n", end);
        Serial.println();
    }

    file.close();

    evaluation_data_t->flag_loaded_validationlabels = true;

    if (DEBUG_LABEL_FILE_HANDLING)
    {
        delay(2000);
        for (int j = 0; j < 20; j++)
        {
            Serial.println("Reihe : " + String(j + 1) + " | " + String(model_data_t->iValidation_Label[j][0]) + " | " + String(model_data_t->iValidation_Label[j][1]) + " | " + String(model_data_t->iValidation_Label[j][2]) + " | " + String(model_data_t->iValidation_Label[j][3]));
        }
    }
}


/********************************************************
  Training-Daten vom Dateisystem einlesen
********************************************************/
void readTestInput()
{
    sendStatusText("Lade Test-Input Datensätze! Dauer max. 1 Minute!");

    CSV_Parser cp(/*format*/ "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff", /*has_header*/ false);

    file = FFat.open("/X_test.csv", "r");

    float *signal_0 = (float *)cp[0];
    float *signal_1 = (float *)cp[1];
    float *signal_2 = (float *)cp[2];
    float *signal_3 = (float *)cp[3];
    float *signal_4 = (float *)cp[4];
    float *signal_5 = (float *)cp[5];
    float *signal_6 = (float *)cp[6];
    float *signal_7 = (float *)cp[7];
    float *signal_8 = (float *)cp[8];
    float *signal_9 = (float *)cp[9];
    float *signal_10 = (float *)cp[10];
    float *signal_11 = (float *)cp[11];
    float *signal_12 = (float *)cp[12];
    float *signal_13 = (float *)cp[13];
    float *signal_14 = (float *)cp[14];
    float *signal_15 = (float *)cp[15];
    float *signal_16 = (float *)cp[16];
    float *signal_17 = (float *)cp[17];
    float *signal_18 = (float *)cp[18];
    float *signal_19 = (float *)cp[19];
    float *signal_20 = (float *)cp[20];
    float *signal_21 = (float *)cp[21];
    float *signal_22 = (float *)cp[22];
    float *signal_23 = (float *)cp[23];
    float *signal_24 = (float *)cp[24];
    float *signal_25 = (float *)cp[25];
    float *signal_26 = (float *)cp[26];
    float *signal_27 = (float *)cp[27];
    float *signal_28 = (float *)cp[28];
    float *signal_29 = (float *)cp[29];
    float *signal_30 = (float *)cp[30];
    float *signal_31 = (float *)cp[31];

    float *signal_32 = (float *)cp[32];
    float *signal_33 = (float *)cp[33];
    float *signal_34 = (float *)cp[34];
    float *signal_35 = (float *)cp[35];
    float *signal_36 = (float *)cp[36];
    float *signal_37 = (float *)cp[37];
    float *signal_38 = (float *)cp[38];
    float *signal_39 = (float *)cp[39];
    float *signal_40 = (float *)cp[40];
    float *signal_41 = (float *)cp[41];
    float *signal_42 = (float *)cp[42];
    float *signal_43 = (float *)cp[43];
    float *signal_44 = (float *)cp[44];
    float *signal_45 = (float *)cp[45];
    float *signal_46 = (float *)cp[46];
    float *signal_47 = (float *)cp[47];
    float *signal_48 = (float *)cp[48];
    float *signal_49 = (float *)cp[49];
    float *signal_50 = (float *)cp[50];
    float *signal_51 = (float *)cp[51];
    float *signal_52 = (float *)cp[52];
    float *signal_53 = (float *)cp[53];
    float *signal_54 = (float *)cp[54];
    float *signal_55 = (float *)cp[55];
    float *signal_56 = (float *)cp[56];
    float *signal_57 = (float *)cp[57];
    float *signal_58 = (float *)cp[58];
    float *signal_59 = (float *)cp[59];
    float *signal_60 = (float *)cp[60];
    float *signal_61 = (float *)cp[61];
    float *signal_62 = (float *)cp[62];
    float *signal_63 = (float *)cp[63];

    int row_index = 0;
    uint32_t start = millis();

    while (cp.parseRow())
    {
        model_data_t->fTest_Data[row_index][0] = signal_0[0];
        model_data_t->fTest_Data[row_index][1] = signal_1[0];
        model_data_t->fTest_Data[row_index][2] = signal_2[0];
        model_data_t->fTest_Data[row_index][3] = signal_3[0];
        model_data_t->fTest_Data[row_index][4] = signal_4[0];
        model_data_t->fTest_Data[row_index][5] = signal_5[0];
        model_data_t->fTest_Data[row_index][6] = signal_6[0];
        model_data_t->fTest_Data[row_index][7] = signal_7[0];
        model_data_t->fTest_Data[row_index][8] = signal_8[0];
        model_data_t->fTest_Data[row_index][9] = signal_9[0];
        model_data_t->fTest_Data[row_index][10] = signal_10[0];
        model_data_t->fTest_Data[row_index][11] = signal_11[0];
        model_data_t->fTest_Data[row_index][12] = signal_12[0];
        model_data_t->fTest_Data[row_index][13] = signal_13[0];
        model_data_t->fTest_Data[row_index][14] = signal_14[0];
        model_data_t->fTest_Data[row_index][15] = signal_15[0];
        model_data_t->fTest_Data[row_index][16] = signal_16[0];
        model_data_t->fTest_Data[row_index][17] = signal_17[0];
        model_data_t->fTest_Data[row_index][18] = signal_18[0];
        model_data_t->fTest_Data[row_index][19] = signal_19[0];
        model_data_t->fTest_Data[row_index][20] = signal_20[0];
        model_data_t->fTest_Data[row_index][21] = signal_21[0];
        model_data_t->fTest_Data[row_index][22] = signal_22[0];
        model_data_t->fTest_Data[row_index][23] = signal_23[0];
        model_data_t->fTest_Data[row_index][24] = signal_24[0];
        model_data_t->fTest_Data[row_index][25] = signal_25[0];
        model_data_t->fTest_Data[row_index][26] = signal_26[0];
        model_data_t->fTest_Data[row_index][27] = signal_27[0];
        model_data_t->fTest_Data[row_index][28] = signal_28[0];
        model_data_t->fTest_Data[row_index][29] = signal_29[0];
        model_data_t->fTest_Data[row_index][30] = signal_30[0];
        model_data_t->fTest_Data[row_index][31] = signal_31[0];
        model_data_t->fTest_Data[row_index][32] = signal_32[0];
        model_data_t->fTest_Data[row_index][33] = signal_33[0];
        model_data_t->fTest_Data[row_index][34] = signal_34[0];
        model_data_t->fTest_Data[row_index][35] = signal_35[0];
        model_data_t->fTest_Data[row_index][36] = signal_36[0];
        model_data_t->fTest_Data[row_index][37] = signal_37[0];
        model_data_t->fTest_Data[row_index][38] = signal_38[0];
        model_data_t->fTest_Data[row_index][39] = signal_39[0];
        model_data_t->fTest_Data[row_index][40] = signal_40[0];
        model_data_t->fTest_Data[row_index][41] = signal_41[0];
        model_data_t->fTest_Data[row_index][42] = signal_42[0];
        model_data_t->fTest_Data[row_index][43] = signal_43[0];
        model_data_t->fTest_Data[row_index][44] = signal_44[0];
        model_data_t->fTest_Data[row_index][45] = signal_45[0];
        model_data_t->fTest_Data[row_index][46] = signal_46[0];
        model_data_t->fTest_Data[row_index][47] = signal_47[0];
        model_data_t->fTest_Data[row_index][48] = signal_48[0];
        model_data_t->fTest_Data[row_index][49] = signal_49[0];
        model_data_t->fTest_Data[row_index][50] = signal_50[0];
        model_data_t->fTest_Data[row_index][51] = signal_51[0];
        model_data_t->fTest_Data[row_index][52] = signal_52[0];
        model_data_t->fTest_Data[row_index][53] = signal_53[0];
        model_data_t->fTest_Data[row_index][54] = signal_54[0];
        model_data_t->fTest_Data[row_index][55] = signal_55[0];
        model_data_t->fTest_Data[row_index][56] = signal_56[0];
        model_data_t->fTest_Data[row_index][57] = signal_57[0];
        model_data_t->fTest_Data[row_index][58] = signal_58[0];
        model_data_t->fTest_Data[row_index][59] = signal_59[0];
        model_data_t->fTest_Data[row_index][60] = signal_60[0];
        model_data_t->fTest_Data[row_index][61] = signal_61[0];
        model_data_t->fTest_Data[row_index][62] = signal_62[0];
        model_data_t->fTest_Data[row_index][63] = signal_63[0];

        row_index++;
    }

    uint32_t end = millis() - start;

    if (DEBUG_FEATURE_FILE_HANDLING)
    {
        Serial.println();
        Serial.println("Test-Features");
        Serial.println();
        Serial.println("Geparste Zeilen : " + String(row_index));
        Serial.printf(" Zeit : %lu ms\r\n", end);
        Serial.println();
    }

    file.close();

    evaluation_data_t->flag_loaded_testinputs = true;

    
    if (DEBUG_FEATURE_FILE_HANDLING)
    {
        delay(2000);
        for (int j = 0; j < 20; j++)
        {
            Serial.println("Reihe : " + String(j + 1) + " | " + String(model_data_t->fTest_Data[j][0], 4) + " | " + String(model_data_t->fTest_Data[j][1], 4) + " | " + String(model_data_t->fTest_Data[j][2], 4) + " | " + String(model_data_t->fTest_Data[j][3], 4) + " | " + String(model_data_t->fTest_Data[j][4], 4) + " | " + String(model_data_t->fTest_Data[j][5], 4) + " | " + String(model_data_t->fTest_Data[j][6], 4) + " | " + String(model_data_t->fTest_Data[j][7], 4) + " | ");
        }
    }
}


/********************************************************
  Validation-Daten vom Dateisystem einlesen
********************************************************/
void readValidationInput()
{
    sendStatusText("Lade Validation-Input Datensätze! Dauer max. 1 Minute!");

    CSV_Parser cp(/*format*/ "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff", /*has_header*/ false);

    file = FFat.open("/X_val.csv", "r");

    float *signal_0 = (float *)cp[0];
    float *signal_1 = (float *)cp[1];
    float *signal_2 = (float *)cp[2];
    float *signal_3 = (float *)cp[3];
    float *signal_4 = (float *)cp[4];
    float *signal_5 = (float *)cp[5];
    float *signal_6 = (float *)cp[6];
    float *signal_7 = (float *)cp[7];
    float *signal_8 = (float *)cp[8];
    float *signal_9 = (float *)cp[9];
    float *signal_10 = (float *)cp[10];
    float *signal_11 = (float *)cp[11];
    float *signal_12 = (float *)cp[12];
    float *signal_13 = (float *)cp[13];
    float *signal_14 = (float *)cp[14];
    float *signal_15 = (float *)cp[15];
    float *signal_16 = (float *)cp[16];
    float *signal_17 = (float *)cp[17];
    float *signal_18 = (float *)cp[18];
    float *signal_19 = (float *)cp[19];
    float *signal_20 = (float *)cp[20];
    float *signal_21 = (float *)cp[21];
    float *signal_22 = (float *)cp[22];
    float *signal_23 = (float *)cp[23];
    float *signal_24 = (float *)cp[24];
    float *signal_25 = (float *)cp[25];
    float *signal_26 = (float *)cp[26];
    float *signal_27 = (float *)cp[27];
    float *signal_28 = (float *)cp[28];
    float *signal_29 = (float *)cp[29];
    float *signal_30 = (float *)cp[30];
    float *signal_31 = (float *)cp[31];

    float *signal_32 = (float *)cp[32];
    float *signal_33 = (float *)cp[33];
    float *signal_34 = (float *)cp[34];
    float *signal_35 = (float *)cp[35];
    float *signal_36 = (float *)cp[36];
    float *signal_37 = (float *)cp[37];
    float *signal_38 = (float *)cp[38];
    float *signal_39 = (float *)cp[39];
    float *signal_40 = (float *)cp[40];
    float *signal_41 = (float *)cp[41];
    float *signal_42 = (float *)cp[42];
    float *signal_43 = (float *)cp[43];
    float *signal_44 = (float *)cp[44];
    float *signal_45 = (float *)cp[45];
    float *signal_46 = (float *)cp[46];
    float *signal_47 = (float *)cp[47];
    float *signal_48 = (float *)cp[48];
    float *signal_49 = (float *)cp[49];
    float *signal_50 = (float *)cp[50];
    float *signal_51 = (float *)cp[51];
    float *signal_52 = (float *)cp[52];
    float *signal_53 = (float *)cp[53];
    float *signal_54 = (float *)cp[54];
    float *signal_55 = (float *)cp[55];
    float *signal_56 = (float *)cp[56];
    float *signal_57 = (float *)cp[57];
    float *signal_58 = (float *)cp[58];
    float *signal_59 = (float *)cp[59];
    float *signal_60 = (float *)cp[60];
    float *signal_61 = (float *)cp[61];
    float *signal_62 = (float *)cp[62];
    float *signal_63 = (float *)cp[63];

    int row_index = 0;
    uint32_t start = millis();

    while (cp.parseRow())
    {
        model_data_t->fValidation_Data[row_index][0] = signal_0[0];
        model_data_t->fValidation_Data[row_index][1] = signal_1[0];
        model_data_t->fValidation_Data[row_index][2] = signal_2[0];
        model_data_t->fValidation_Data[row_index][3] = signal_3[0];
        model_data_t->fValidation_Data[row_index][4] = signal_4[0];
        model_data_t->fValidation_Data[row_index][5] = signal_5[0];
        model_data_t->fValidation_Data[row_index][6] = signal_6[0];
        model_data_t->fValidation_Data[row_index][7] = signal_7[0];
        model_data_t->fValidation_Data[row_index][8] = signal_8[0];
        model_data_t->fValidation_Data[row_index][9] = signal_9[0];
        model_data_t->fValidation_Data[row_index][10] = signal_10[0];
        model_data_t->fValidation_Data[row_index][11] = signal_11[0];
        model_data_t->fValidation_Data[row_index][12] = signal_12[0];
        model_data_t->fValidation_Data[row_index][13] = signal_13[0];
        model_data_t->fValidation_Data[row_index][14] = signal_14[0];
        model_data_t->fValidation_Data[row_index][15] = signal_15[0];
        model_data_t->fValidation_Data[row_index][16] = signal_16[0];
        model_data_t->fValidation_Data[row_index][17] = signal_17[0];
        model_data_t->fValidation_Data[row_index][18] = signal_18[0];
        model_data_t->fValidation_Data[row_index][19] = signal_19[0];
        model_data_t->fValidation_Data[row_index][20] = signal_20[0];
        model_data_t->fValidation_Data[row_index][21] = signal_21[0];
        model_data_t->fValidation_Data[row_index][22] = signal_22[0];
        model_data_t->fValidation_Data[row_index][23] = signal_23[0];
        model_data_t->fValidation_Data[row_index][24] = signal_24[0];
        model_data_t->fValidation_Data[row_index][25] = signal_25[0];
        model_data_t->fValidation_Data[row_index][26] = signal_26[0];
        model_data_t->fValidation_Data[row_index][27] = signal_27[0];
        model_data_t->fValidation_Data[row_index][28] = signal_28[0];
        model_data_t->fValidation_Data[row_index][29] = signal_29[0];
        model_data_t->fValidation_Data[row_index][30] = signal_30[0];
        model_data_t->fValidation_Data[row_index][31] = signal_31[0];
        model_data_t->fValidation_Data[row_index][32] = signal_32[0];
        model_data_t->fValidation_Data[row_index][33] = signal_33[0];
        model_data_t->fValidation_Data[row_index][34] = signal_34[0];
        model_data_t->fValidation_Data[row_index][35] = signal_35[0];
        model_data_t->fValidation_Data[row_index][36] = signal_36[0];
        model_data_t->fValidation_Data[row_index][37] = signal_37[0];
        model_data_t->fValidation_Data[row_index][38] = signal_38[0];
        model_data_t->fValidation_Data[row_index][39] = signal_39[0];
        model_data_t->fValidation_Data[row_index][40] = signal_40[0];
        model_data_t->fValidation_Data[row_index][41] = signal_41[0];
        model_data_t->fValidation_Data[row_index][42] = signal_42[0];
        model_data_t->fValidation_Data[row_index][43] = signal_43[0];
        model_data_t->fValidation_Data[row_index][44] = signal_44[0];
        model_data_t->fValidation_Data[row_index][45] = signal_45[0];
        model_data_t->fValidation_Data[row_index][46] = signal_46[0];
        model_data_t->fValidation_Data[row_index][47] = signal_47[0];
        model_data_t->fValidation_Data[row_index][48] = signal_48[0];
        model_data_t->fValidation_Data[row_index][49] = signal_49[0];
        model_data_t->fValidation_Data[row_index][50] = signal_50[0];
        model_data_t->fValidation_Data[row_index][51] = signal_51[0];
        model_data_t->fValidation_Data[row_index][52] = signal_52[0];
        model_data_t->fValidation_Data[row_index][53] = signal_53[0];
        model_data_t->fValidation_Data[row_index][54] = signal_54[0];
        model_data_t->fValidation_Data[row_index][55] = signal_55[0];
        model_data_t->fValidation_Data[row_index][56] = signal_56[0];
        model_data_t->fValidation_Data[row_index][57] = signal_57[0];
        model_data_t->fValidation_Data[row_index][58] = signal_58[0];
        model_data_t->fValidation_Data[row_index][59] = signal_59[0];
        model_data_t->fValidation_Data[row_index][60] = signal_60[0];
        model_data_t->fValidation_Data[row_index][61] = signal_61[0];
        model_data_t->fValidation_Data[row_index][62] = signal_62[0];
        model_data_t->fValidation_Data[row_index][63] = signal_63[0];

        row_index++;
    }

    uint32_t end = millis() - start;

    if (DEBUG_FEATURE_FILE_HANDLING)
    {
        Serial.println();
        Serial.println("Validation-Features");
        Serial.println();
        Serial.println("Geparste Zeilen : " + String(row_index));
        Serial.printf(" Zeit : %lu ms\r\n", end);
        Serial.println();
    }

    file.close();

    evaluation_data_t->flag_loaded_validationinputs = true;

    
    if (DEBUG_FEATURE_FILE_HANDLING)
    {
        delay(2000);
        for (int j = 0; j < 20; j++)
        {
            Serial.println("Reihe : " + String(j + 1) + " | " + String(model_data_t->fValidation_Data[j][0], 4) + " | " + String(model_data_t->fValidation_Data[j][1], 4) + " | " + String(model_data_t->fValidation_Data[j][2], 4) + " | " + String(model_data_t->fValidation_Data[j][3], 4) + " | " + String(model_data_t->fValidation_Data[j][4], 4) + " | " + String(model_data_t->fValidation_Data[j][5], 4) + " | " + String(model_data_t->fValidation_Data[j][6], 4) + " | " + String(model_data_t->fValidation_Data[j][7], 4) + " | ");
        }
    }
}
