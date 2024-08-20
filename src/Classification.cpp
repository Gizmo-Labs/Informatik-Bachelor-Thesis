/********************************************************
  Bibliotheken
********************************************************/
#include "Prototypes.h"

extern TINYML_DATA *data_collecting_t;
int classes[10] = {};


/********************************************************
  EloquentTinyML Instanz
********************************************************/
extern Eloquent::TF::Sequential<TF_NUM_OPS, ARENA_SIZE> tf;


/********************************************************
  Klassifizierung durchführen
********************************************************/
void runClassifier()
{
    // Checke ob Vorhersage korrekt ausgeführt wurde
    if (!tf.predict(data_collecting_t->fBluetoothData).isOk())
    {
        sendSomewhat(PREFIX_EVAL + "_Result_Classifier", Evaluation_Topic, "4");
        Serial.println(tf.exception.toString());
        delay(100);
        return;
    }

    if (data_collecting_t->iCount_Classifications < 10)
    {
        classes[data_collecting_t->iCount_Classifications] = tf.classification;
        data_collecting_t->iCount_Classifications++;
    }

    if (data_collecting_t->iCount_Classifications == 10)
    {
        size_t i, j, count;
        size_t most = 0;
        int temp, elem;

        for (i = 0; i < 10; i++)
        {
            temp = classes[i];
            count = 1;
            for (j = i + 1; j < 10; j++)
            {
                if (classes[j] == temp)
                {
                    count++;
                }
            }
            if (most < count)
            {
                most = count;
                elem = classes[i];
            }
        }

        sendSomewhat(PREFIX_EVAL + "_Result_Classifier", Evaluation_Topic, String(elem));
        //Serial.println("Geste : " + String(elem));
        data_collecting_t->iCount_Classifications = 0;
    }
}
