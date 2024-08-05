/********************************************************
  Bibliotheken
********************************************************/
#include "Prototypes.h"

/********************************************************
  EloquentTinyML Instanz
********************************************************/
Eloquent::TF::Sequential<TF_NUM_OPS, ARENA_SIZE> tf;

/********************************************************
  Array für Training-Labels auf PSRAM allokieren
********************************************************/
MODEL_DATA *model_data_t = (MODEL_DATA *)heap_caps_malloc(sizeof(MODEL_DATA), MALLOC_CAP_SPIRAM);
EVALUATION_DATA *evaluation_data_t = (EVALUATION_DATA *)heap_caps_malloc(sizeof(EVALUATION_DATA), MALLOC_CAP_SPIRAM);

extern TINYML_DATA *data_collecting_t;

/********************************************************
    Ausgabe der Konfusions-Matrix für Testdaten
********************************************************/
void runTestConfusionMatrix(int rows)
{
    const int N = 4;
    int i = 0;
    int A[4];

    int True_Positives_Class_1 = 0;
    int True_Negatives_Class_1 = 0;
    int False_Positives_Class_1 = 0;
    int False_Negatives_Class_1 = 0;

    int True_Positives_Class_2 = 0;
    int True_Negatives_Class_2 = 0;
    int False_Positives_Class_2 = 0;
    int False_Negatives_Class_2 = 0;

    int True_Positives_Class_3 = 0;
    int True_Negatives_Class_3 = 0;
    int False_Positives_Class_3 = 0;
    int False_Negatives_Class_3 = 0;

    int True_Positives_Class_4 = 0;
    int True_Negatives_Class_4 = 0;
    int False_Positives_Class_4 = 0;
    int False_Negatives_Class_4 = 0;

    for (i = 0; i < rows; i++)
    {
        // Checke ob Vorhersage korrekt ausgeführt wurde
        if (!tf.predict(model_data_t->fTest_Data[i]).isOk())
        {
            Serial.println(tf.exception.toString());
            return;
        }

        // Ermitteln des korrekten Labels aus dem Struct
        for (int j = 0; j < 4; j++)
        {
            A[j] = model_data_t->iTest_Label[i][j];
        }

        // Index der "1" aus dem One-Hot-Encoded Array ermitteln
        uint8_t index = std::distance(A, std::max_element(A, A + N));

        // Eins dazu zählen, wegen 0-basiertem Index
        index += 1;
        tf.classification += 1;

        /********************************************************
          Auswertungen von Klasse 1 (Geballte Faust)
        ********************************************************/
        if ((index == 1) && (tf.classification == 1))
        {
            // Wenn Klasse 1 richtigerweise als Klasse 1 erkannt wurde
            // Dies ist ein True Positive für Klasse 1 [Geballte Faust]
            True_Positives_Class_1 += 1;
        }

        if (((index == 2) && (tf.classification == 2)) || ((index == 3) && (tf.classification == 3)) || ((index == 4) && (tf.classification == 4)))
        {
            // Wenn Klasse 2 richtigerweise als Klasse 2 erkannt wurde oder
            // Klasse 3 richtigerweise als Klasse 3 erkannt wurde oder
            // Klasse 4 richtigerweise als Klasse 4 erkannt wurde
            // Dies ist ein True Negative für Klasse 1 [Geballte Faust]
            True_Negatives_Class_1 += 1;
        }

        if (((index == 2) || (index == 3) || (index == 4)) && (tf.classification == 1))
        {
            // Wenn Klasse 2,3 oder 4 fälschlicherweise als Klasse 1 erkannt wurden
            // Dies ist ein False Positive für Geste 1 [Geballte Faust]
            False_Positives_Class_1 += 1;
        }

        if ((index == 1) && ((tf.classification == 2) || (tf.classification == 3) || (tf.classification == 4)))
        {
            // Wenn Klasse 1 korrekt aber fälschlicherweise als Klasse 2, 3 oder 4 erkannt
            // Dies ist ein False Positive für Geste 1 [Geballte Faust]
            False_Negatives_Class_1 += 1;
        }

        /********************************************************
          Auswertungen von Klasse 2 (Daumen hoch)
        ********************************************************/
        if ((index == 2) && (tf.classification == 2))
        {
            // Wenn Klasse 2 richtigerweise als Klasse 2 erkannt wurde
            // Dies ist ein True Positive für Klasse 2 [Daumen hoch]
            True_Positives_Class_2 += 1;
        }

        if (((index == 1) && (tf.classification == 1)) || ((index == 3) && (tf.classification == 3)) || ((index == 4) && (tf.classification == 4)))
        {
            // Wenn Klasse 1 richtigerweise als Klasse 1 erkannt wurde oder
            // Klasse 3 richtigerweise als Klasse 3 erkannt wurde oder
            // Klasse 4 richtigerweise als Klasse 4 erkannt wurde
            // Dies ist ein True Negative für Klasse 2 [Daumen hoch]
            True_Negatives_Class_2 += 1;
        }

        if (((index == 1) || (index == 3) || (index == 4)) && (tf.classification == 2))
        {
            // Wenn Klasse 1,3 oder 4 fälschlicherweise als Klasse 2 erkannt wurden
            // Dies ist ein False Positive für Geste 2 [Daumen hoch]
            False_Positives_Class_2 += 1;
        }

        if ((index == 2) && ((tf.classification == 1) || (tf.classification == 3) || (tf.classification == 4)))
        {
            // Wenn Klasse 2 korrekt aber fälschlicherweise als Klasse 1, 3 oder 4 erkannt
            // Dies ist ein False Positive für Geste 2 [Daumen hoch]
            False_Negatives_Class_2 += 1;
        }

        /********************************************************
          Auswertungen von Klasse 3 (Zeige- und Mittelfinger)
        ********************************************************/
        if ((index == 3) && (tf.classification == 3))
        {
            // Wenn Klasse 3 richtigerweise als Klasse 3 erkannt wurde
            // Dies ist ein True Positive für Klasse 3 [Zeige- und Mittelfinger]
            True_Positives_Class_3 += 1;
        }

        if (((index == 1) && (tf.classification == 1)) || ((index == 2) && (tf.classification == 2)) || ((index == 4) && (tf.classification == 4)))
        {
            // Wenn Klasse 1 richtigerweise als Klasse 1 erkannt wurde oder
            // Klasse 2 richtigerweise als Klasse 2 erkannt wurde oder
            // Klasse 4 richtigerweise als Klasse 4 erkannt wurde
            // Dies ist ein True Negative für Klasse 3 [Zeige- und Mittelfinger]
            True_Negatives_Class_3 += 1;
        }

        if (((index == 1) || (index == 2) || (index == 4)) && (tf.classification == 3))
        {
            // Wenn Klasse 1,2 oder 4 fälschlicherweise als Klasse 3 erkannt wurden
            // Dies ist ein False Positive für Geste 3 [Zeige- und Mittelfinger]
            False_Positives_Class_3 += 1;
        }

        if ((index == 3) && ((tf.classification == 2) || (tf.classification == 3) || (tf.classification == 4)))
        {
            // Wenn Klasse 3 korrekt aber fälschlicherweise als Klasse 2, 3 oder 4 erkannt
            // Dies ist ein False Positive für Geste 3 [Zeige- und Mittelfinger]
            False_Negatives_Class_3 += 1;
        }

        /********************************************************
          Auswertungen von Klasse 4 (Alle Finger)
        ********************************************************/
        if ((index == 4) && (tf.classification == 4))
        {
            // Wenn Klasse 4 richtigerweise als Klasse 4 erkannt wurde
            // Dies ist ein True Positive für Klasse 4 [Alle Finger]
            True_Positives_Class_4 += 1;
        }

        if (((index == 1) && (tf.classification == 1)) || ((index == 2) && (tf.classification == 2)) || ((index == 3) && (tf.classification == 3)))
        {
            // Wenn Klasse 1 richtigerweise als Klasse 1 erkannt wurde oder
            // Klasse 2 richtigerweise als Klasse 2 erkannt wurde oder
            // Klasse 3 richtigerweise als Klasse 3 erkannt wurde
            // Dies ist ein True Negative für Klasse 4 [Zeige- und Mittelfinger]
            True_Negatives_Class_4 += 1;
        }

        if (((index == 1) || (index == 2) || (index == 3)) && (tf.classification == 4))
        {
            // Wenn Klasse 1,2 oder 3 fälschlicherweise als Klasse 4 erkannt wurden
            // Dies ist ein False Positive für Geste 4 [Zeige- und Mittelfinger]
            False_Positives_Class_4 += 1;
        }

        if ((index == 4) && ((tf.classification == 1) || (tf.classification == 2) || (tf.classification == 3)))
        {
            // Wenn Klasse 4 korrekt aber fälschlicherweise als Klasse 1, 2 oder 3 erkannt
            // Dies ist ein False Positive für Geste 4 [Zeige- und Mittelfinger]
            False_Negatives_Class_4 += 1;
        }

        if (DEBUG_EVALUATION)
        {
            Serial.print("Das hat gedauert --> ");
            Serial.print(tf.benchmark.microseconds() / 1000);
            Serial.println(" ms fuer eine Vorhersage");
        }
    }

    Serial.println();
    Serial.println("True Positives von Geste 1 : " + String(True_Positives_Class_1));
    Serial.println("False Positives von Geste 1 : " + String(False_Positives_Class_1));
    Serial.println("True Negatives von Geste 1 : " + String(True_Negatives_Class_1));
    Serial.println("False Negatives von Geste 1 : " + String(False_Negatives_Class_1));
    Serial.println();
    Serial.println("-------------------------------------------------------------");
    Serial.println();
    Serial.println("True Positives von Geste 2 : " + String(True_Positives_Class_2));
    Serial.println("False Positives von Geste 2 : " + String(False_Positives_Class_2));
    Serial.println("True Negatives von Geste 2 : " + String(True_Negatives_Class_2));
    Serial.println("False Negatives von Geste 2 : " + String(False_Negatives_Class_2));
    Serial.println();
    Serial.println("-------------------------------------------------------------");
    Serial.println();
    Serial.println("True Positives von Geste 3 : " + String(True_Positives_Class_3));
    Serial.println("False Positives von Geste 3 : " + String(False_Positives_Class_3));
    Serial.println("True Negatives von Geste 3 : " + String(True_Negatives_Class_3));
    Serial.println("False Negatives von Geste 3 : " + String(False_Negatives_Class_3));
    Serial.println();
    Serial.println("-------------------------------------------------------------");
    Serial.println();
    Serial.println("True Positives von Geste 4 : " + String(True_Positives_Class_4));
    Serial.println("False Positives von Geste 4 : " + String(False_Positives_Class_4));
    Serial.println("True Negatives von Geste 4 : " + String(True_Negatives_Class_4));
    Serial.println("False Negatives von Geste 4 : " + String(False_Negatives_Class_4));
    Serial.println();
    Serial.println("-------------------------------------------------------------");
    Serial.println();

    /********************************************************
        Precision und Recall gemäß Micro-Averaging
    ********************************************************/
    float Precision_1 = ((True_Positives_Class_1 / 1.0f) / (True_Positives_Class_1 + False_Positives_Class_1 / 1.0f));
    float Precision_2 = ((True_Positives_Class_2 / 1.0f) / (True_Positives_Class_2 + False_Positives_Class_2 / 1.0f));
    float Precision_3 = ((True_Positives_Class_3 / 1.0f) / (True_Positives_Class_3 + False_Positives_Class_3 / 1.0f));
    float Precision_4 = ((True_Positives_Class_4 / 1.0f) / (True_Positives_Class_4 + False_Positives_Class_4 / 1.0f));

    float Recall_1 = ((True_Positives_Class_1 / 1.0f) / (True_Positives_Class_1 + False_Negatives_Class_1 / 1.0f));
    float Recall_2 = ((True_Positives_Class_2 / 1.0f) / (True_Positives_Class_2 + False_Negatives_Class_2 / 1.0f));
    float Recall_3 = ((True_Positives_Class_3 / 1.0f) / (True_Positives_Class_3 + False_Negatives_Class_3 / 1.0f));
    float Recall_4 = ((True_Positives_Class_4 / 1.0f) / (True_Positives_Class_4 + False_Negatives_Class_4 / 1.0f));

    float Prec_Oben = (True_Positives_Class_1 + True_Positives_Class_2 + True_Positives_Class_3 + True_Positives_Class_4) / 1.0f;
    float Prec_Unten = (True_Positives_Class_1 + False_Positives_Class_1 + True_Positives_Class_2 + False_Positives_Class_2 + True_Positives_Class_3 + False_Positives_Class_3 + True_Positives_Class_4 + False_Positives_Class_4) / 1.0f;
    float Micro_Precision = Prec_Oben / Prec_Unten;

    float Recall_Unten = (True_Positives_Class_1 + False_Negatives_Class_1 + True_Positives_Class_2 + False_Negatives_Class_2 + True_Positives_Class_3 + False_Negatives_Class_3 + True_Positives_Class_4 + False_Negatives_Class_4) / 1.0f;
    float Micro_Recall = Prec_Oben / Recall_Unten;

    float Macro_Precision = (Precision_1 + Precision_2 + Precision_3 + Precision_4 / 1.0f) / 4;
    float Macro_Recall = (Recall_1 + Recall_2 + Recall_3 + Recall_4 / 1.0f) / 4;

    float f1_score_1 = 2 * ((Precision_1 * Recall_1) / (Precision_1 + Recall_1));
    float f1_score_2 = 2 * ((Precision_2 * Recall_2) / (Precision_2 + Recall_2));
    float f1_score_3 = 2 * ((Precision_3 * Recall_3) / (Precision_3 + Recall_3));
    float f1_score_4 = 2 * ((Precision_4 * Recall_4) / (Precision_4 + Recall_4));

    float Macro_f1_score = 2 * ((Macro_Precision * Macro_Recall) / (Macro_Precision + Macro_Recall));
    float Micro_f1_score = 2 * ((Micro_Precision * Micro_Recall) / (Micro_Precision + Micro_Recall));

    Serial.println("\t\tPrecision\tRecall\t\tF1-Score");
    Serial.println();
    Serial.println("Klasse 1\t" + String(Precision_1) + "\t\t" + String(Recall_1) + "\t\t" + String(f1_score_1));
    Serial.println("Klasse 2\t" + String(Precision_2) + "\t\t" + String(Recall_2) + "\t\t" + String(f1_score_2));
    Serial.println("Klasse 3\t" + String(Precision_3) + "\t\t" + String(Recall_3) + "\t\t" + String(f1_score_3));
    Serial.println("Klasse 4\t" + String(Precision_4) + "\t\t" + String(Recall_4) + "\t\t" + String(f1_score_4));
    Serial.println();
    Serial.println("-------------------------------------------------------------");
    Serial.println();
    Serial.println("Micro-Average\t" + String(Micro_Precision) + "\t\t" + String(Micro_Recall) + "\t\t" + String(Micro_f1_score));
    Serial.println("Macro-Average\t" + String(Macro_Precision) + "\t\t" + String(Macro_Recall) + "\t\t" + String(Macro_f1_score));
}


/********************************************************
    Ausgabe der Konfusions-Matrix für Validierungsdaten
********************************************************/
void runValidationConfusionMatrix(int rows)
{
    const int N = 4;
    int i = 0;
    int A[4];

    int True_Positives_Class_1 = 0;
    int True_Negatives_Class_1 = 0;
    int False_Positives_Class_1 = 0;
    int False_Negatives_Class_1 = 0;

    int True_Positives_Class_2 = 0;
    int True_Negatives_Class_2 = 0;
    int False_Positives_Class_2 = 0;
    int False_Negatives_Class_2 = 0;

    int True_Positives_Class_3 = 0;
    int True_Negatives_Class_3 = 0;
    int False_Positives_Class_3 = 0;
    int False_Negatives_Class_3 = 0;

    int True_Positives_Class_4 = 0;
    int True_Negatives_Class_4 = 0;
    int False_Positives_Class_4 = 0;
    int False_Negatives_Class_4 = 0;

    for (i = 0; i < rows; i++)
    {
        // Checke ob Vorhersage korrekt ausgeführt wurde
        if (!tf.predict(model_data_t->fValidation_Data[i]).isOk())
        {
            Serial.println(tf.exception.toString());
            return;
        }

        // Ermitteln des korrekten Labels aus dem Struct
        for (int j = 0; j < 4; j++)
        {
            A[j] = model_data_t->iValidation_Label[i][j];
        }

        // Index der "1" aus dem One-Hot-Encoded Array ermitteln
        uint8_t index = std::distance(A, std::max_element(A, A + N));

        // Eins dazu zählen, wegen 0-basiertem Index
        index += 1;
        tf.classification += 1;

        /********************************************************
          Auswertungen von Klasse 1 (Geballte Faust)
        ********************************************************/
        if ((index == 1) && (tf.classification == 1))
        {
            // Wenn Klasse 1 richtigerweise als Klasse 1 erkannt wurde
            // Dies ist ein True Positive für Klasse 1 [Geballte Faust]
            True_Positives_Class_1 += 1;
        }

        if (((index == 2) && (tf.classification == 2)) || ((index == 3) && (tf.classification == 3)) || ((index == 4) && (tf.classification == 4)))
        {
            // Wenn Klasse 2 richtigerweise als Klasse 2 erkannt wurde oder
            // Klasse 3 richtigerweise als Klasse 3 erkannt wurde oder
            // Klasse 4 richtigerweise als Klasse 4 erkannt wurde
            // Dies ist ein True Negative für Klasse 1 [Geballte Faust]
            True_Negatives_Class_1 += 1;
        }

        if (((index == 2) || (index == 3) || (index == 4)) && (tf.classification == 1))
        {
            // Wenn Klasse 2,3 oder 4 fälschlicherweise als Klasse 1 erkannt wurden
            // Dies ist ein False Positive für Geste 1 [Geballte Faust]
            False_Positives_Class_1 += 1;
        }

        if ((index == 1) && ((tf.classification == 2) || (tf.classification == 3) || (tf.classification == 4)))
        {
            // Wenn Klasse 1 korrekt aber fälschlicherweise als Klasse 2, 3 oder 4 erkannt
            // Dies ist ein False Positive für Geste 1 [Geballte Faust]
            False_Negatives_Class_1 += 1;
        }

        /********************************************************
          Auswertungen von Klasse 2 (Daumen hoch)
        ********************************************************/
        if ((index == 2) && (tf.classification == 2))
        {
            // Wenn Klasse 2 richtigerweise als Klasse 2 erkannt wurde
            // Dies ist ein True Positive für Klasse 2 [Daumen hoch]
            True_Positives_Class_2 += 1;
        }

        if (((index == 1) && (tf.classification == 1)) || ((index == 3) && (tf.classification == 3)) || ((index == 4) && (tf.classification == 4)))
        {
            // Wenn Klasse 1 richtigerweise als Klasse 1 erkannt wurde oder
            // Klasse 3 richtigerweise als Klasse 3 erkannt wurde oder
            // Klasse 4 richtigerweise als Klasse 4 erkannt wurde
            // Dies ist ein True Negative für Klasse 2 [Daumen hoch]
            True_Negatives_Class_2 += 1;
        }

        if (((index == 1) || (index == 3) || (index == 4)) && (tf.classification == 2))
        {
            // Wenn Klasse 1,3 oder 4 fälschlicherweise als Klasse 2 erkannt wurden
            // Dies ist ein False Positive für Geste 2 [Daumen hoch]
            False_Positives_Class_2 += 1;
        }

        if ((index == 2) && ((tf.classification == 1) || (tf.classification == 3) || (tf.classification == 4)))
        {
            // Wenn Klasse 2 korrekt aber fälschlicherweise als Klasse 1, 3 oder 4 erkannt
            // Dies ist ein False Positive für Geste 2 [Daumen hoch]
            False_Negatives_Class_2 += 1;
        }

        /********************************************************
          Auswertungen von Klasse 3 (Zeige- und Mittelfinger)
        ********************************************************/
        if ((index == 3) && (tf.classification == 3))
        {
            // Wenn Klasse 3 richtigerweise als Klasse 3 erkannt wurde
            // Dies ist ein True Positive für Klasse 3 [Zeige- und Mittelfinger]
            True_Positives_Class_3 += 1;
        }

        if (((index == 1) && (tf.classification == 1)) || ((index == 2) && (tf.classification == 2)) || ((index == 4) && (tf.classification == 4)))
        {
            // Wenn Klasse 1 richtigerweise als Klasse 1 erkannt wurde oder
            // Klasse 2 richtigerweise als Klasse 2 erkannt wurde oder
            // Klasse 4 richtigerweise als Klasse 4 erkannt wurde
            // Dies ist ein True Negative für Klasse 3 [Zeige- und Mittelfinger]
            True_Negatives_Class_3 += 1;
        }

        if (((index == 1) || (index == 2) || (index == 4)) && (tf.classification == 3))
        {
            // Wenn Klasse 1,2 oder 4 fälschlicherweise als Klasse 3 erkannt wurden
            // Dies ist ein False Positive für Geste 3 [Zeige- und Mittelfinger]
            False_Positives_Class_3 += 1;
        }

        if ((index == 3) && ((tf.classification == 2) || (tf.classification == 3) || (tf.classification == 4)))
        {
            // Wenn Klasse 3 korrekt aber fälschlicherweise als Klasse 2, 3 oder 4 erkannt
            // Dies ist ein False Positive für Geste 3 [Zeige- und Mittelfinger]
            False_Negatives_Class_3 += 1;
        }

        /********************************************************
          Auswertungen von Klasse 4 (Alle Finger)
        ********************************************************/
        if ((index == 4) && (tf.classification == 4))
        {
            // Wenn Klasse 4 richtigerweise als Klasse 4 erkannt wurde
            // Dies ist ein True Positive für Klasse 4 [Alle Finger]
            True_Positives_Class_4 += 1;
        }

        if (((index == 1) && (tf.classification == 1)) || ((index == 2) && (tf.classification == 2)) || ((index == 3) && (tf.classification == 3)))
        {
            // Wenn Klasse 1 richtigerweise als Klasse 1 erkannt wurde oder
            // Klasse 2 richtigerweise als Klasse 2 erkannt wurde oder
            // Klasse 3 richtigerweise als Klasse 3 erkannt wurde
            // Dies ist ein True Negative für Klasse 4 [Zeige- und Mittelfinger]
            True_Negatives_Class_4 += 1;
        }

        if (((index == 1) || (index == 2) || (index == 3)) && (tf.classification == 4))
        {
            // Wenn Klasse 1,2 oder 3 fälschlicherweise als Klasse 4 erkannt wurden
            // Dies ist ein False Positive für Geste 4 [Zeige- und Mittelfinger]
            False_Positives_Class_4 += 1;
        }

        if ((index == 4) && ((tf.classification == 1) || (tf.classification == 2) || (tf.classification == 3)))
        {
            // Wenn Klasse 4 korrekt aber fälschlicherweise als Klasse 1, 2 oder 3 erkannt
            // Dies ist ein False Positive für Geste 4 [Zeige- und Mittelfinger]
            False_Negatives_Class_4 += 1;
        }

        if (DEBUG_EVALUATION)
        {
            Serial.print("Das hat gedauert --> ");
            Serial.print(tf.benchmark.microseconds() / 1000);
            Serial.println(" ms fuer eine Vorhersage");
        }
    }

    Serial.println();
    Serial.println("True Positives von Geste 1 : " + String(True_Positives_Class_1));
    Serial.println("False Positives von Geste 1 : " + String(False_Positives_Class_1));
    Serial.println("True Negatives von Geste 1 : " + String(True_Negatives_Class_1));
    Serial.println("False Negatives von Geste 1 : " + String(False_Negatives_Class_1));
    Serial.println();
    Serial.println("-------------------------------------------------------------");
    Serial.println();
    Serial.println("True Positives von Geste 2 : " + String(True_Positives_Class_2));
    Serial.println("False Positives von Geste 2 : " + String(False_Positives_Class_2));
    Serial.println("True Negatives von Geste 2 : " + String(True_Negatives_Class_2));
    Serial.println("False Negatives von Geste 2 : " + String(False_Negatives_Class_2));
    Serial.println();
    Serial.println("-------------------------------------------------------------");
    Serial.println();
    Serial.println("True Positives von Geste 3 : " + String(True_Positives_Class_3));
    Serial.println("False Positives von Geste 3 : " + String(False_Positives_Class_3));
    Serial.println("True Negatives von Geste 3 : " + String(True_Negatives_Class_3));
    Serial.println("False Negatives von Geste 3 : " + String(False_Negatives_Class_3));
    Serial.println();
    Serial.println("-------------------------------------------------------------");
    Serial.println();
    Serial.println("True Positives von Geste 4 : " + String(True_Positives_Class_4));
    Serial.println("False Positives von Geste 4 : " + String(False_Positives_Class_4));
    Serial.println("True Negatives von Geste 4 : " + String(True_Negatives_Class_4));
    Serial.println("False Negatives von Geste 4 : " + String(False_Negatives_Class_4));
    Serial.println();
    Serial.println("-------------------------------------------------------------");
    Serial.println();

    /********************************************************
        Precision und Recall gemäß Micro-Averaging
    ********************************************************/
    float Precision_1 = ((True_Positives_Class_1 / 1.0f) / (True_Positives_Class_1 + False_Positives_Class_1 / 1.0f));
    float Precision_2 = ((True_Positives_Class_2 / 1.0f) / (True_Positives_Class_2 + False_Positives_Class_2 / 1.0f));
    float Precision_3 = ((True_Positives_Class_3 / 1.0f) / (True_Positives_Class_3 + False_Positives_Class_3 / 1.0f));
    float Precision_4 = ((True_Positives_Class_4 / 1.0f) / (True_Positives_Class_4 + False_Positives_Class_4 / 1.0f));

    float Recall_1 = ((True_Positives_Class_1 / 1.0f) / (True_Positives_Class_1 + False_Negatives_Class_1 / 1.0f));
    float Recall_2 = ((True_Positives_Class_2 / 1.0f) / (True_Positives_Class_2 + False_Negatives_Class_2 / 1.0f));
    float Recall_3 = ((True_Positives_Class_3 / 1.0f) / (True_Positives_Class_3 + False_Negatives_Class_3 / 1.0f));
    float Recall_4 = ((True_Positives_Class_4 / 1.0f) / (True_Positives_Class_4 + False_Negatives_Class_4 / 1.0f));

    float Prec_Oben = (True_Positives_Class_1 + True_Positives_Class_2 + True_Positives_Class_3 + True_Positives_Class_4) / 1.0f;
    float Prec_Unten = (True_Positives_Class_1 + False_Positives_Class_1 + True_Positives_Class_2 + False_Positives_Class_2 + True_Positives_Class_3 + False_Positives_Class_3 + True_Positives_Class_4 + False_Positives_Class_4) / 1.0f;
    float Micro_Precision = Prec_Oben / Prec_Unten;

    float Recall_Unten = (True_Positives_Class_1 + False_Negatives_Class_1 + True_Positives_Class_2 + False_Negatives_Class_2 + True_Positives_Class_3 + False_Negatives_Class_3 + True_Positives_Class_4 + False_Negatives_Class_4) / 1.0f;
    float Micro_Recall = Prec_Oben / Recall_Unten;

    float Macro_Precision = (Precision_1 + Precision_2 + Precision_3 + Precision_4 / 1.0f) / 4;
    float Macro_Recall = (Recall_1 + Recall_2 + Recall_3 + Recall_4 / 1.0f) / 4;

    float f1_score_1 = 2 * ((Precision_1 * Recall_1) / (Precision_1 + Recall_1));
    float f1_score_2 = 2 * ((Precision_2 * Recall_2) / (Precision_2 + Recall_2));
    float f1_score_3 = 2 * ((Precision_3 * Recall_3) / (Precision_3 + Recall_3));
    float f1_score_4 = 2 * ((Precision_4 * Recall_4) / (Precision_4 + Recall_4));

    float Macro_f1_score = 2 * ((Macro_Precision * Macro_Recall) / (Macro_Precision + Macro_Recall));
    float Micro_f1_score = 2 * ((Micro_Precision * Micro_Recall) / (Micro_Precision + Micro_Recall));

    Serial.println("\t\tPrecision\tRecall\t\tF1-Score");
    Serial.println();
    Serial.println("Klasse 1\t" + String(Precision_1) + "\t\t" + String(Recall_1) + "\t\t" + String(f1_score_1));
    Serial.println("Klasse 2\t" + String(Precision_2) + "\t\t" + String(Recall_2) + "\t\t" + String(f1_score_2));
    Serial.println("Klasse 3\t" + String(Precision_3) + "\t\t" + String(Recall_3) + "\t\t" + String(f1_score_3));
    Serial.println("Klasse 4\t" + String(Precision_4) + "\t\t" + String(Recall_4) + "\t\t" + String(f1_score_4));
    Serial.println();
    Serial.println("-------------------------------------------------------------");
    Serial.println();
    Serial.println("Micro-Average\t" + String(Micro_Precision) + "\t\t" + String(Micro_Recall) + "\t\t" + String(Micro_f1_score));
    Serial.println("Macro-Average\t" + String(Macro_Precision) + "\t\t" + String(Macro_Recall) + "\t\t" + String(Macro_f1_score));
}


void runClassifier()
{
    classifyingLight();
    // Checke ob Vorhersage korrekt ausgeführt wurde
        if (!tf.predict(data_collecting_t->iBluetoothData).isOk())
        {
            Serial.println(tf.exception.toString());
            return;
        }

        uint8_t gesture = tf.classification;

        sendSomewhat(PREFIX_EVAL + "_Result_Classifier", Evaluation_Topic, String(gesture));        
}