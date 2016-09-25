
#include "ThingSpeak.h"
#include "oBee.h"
#include "spark-dallas-temperature.h"
#include "OneWire.h"
#include "DroneTemperature.h"
#include "oBeeSound.h"
#include "Worker.h"
#include "DroneSwitch.h"
#include "Drone.h"
#include "oBeeRGB.h"

oBee oBeeOne;
/* Thingspeak */
unsigned int myChannelNumber = 141202;
const char * myWriteAPIKey = "L1UKP3FYTXHG69F2";
TCPClient client;

unsigned long publishTime = 15000;
unsigned long ms;
unsigned long msLast;

void setup() {

    Serial.begin(19200);

    delay(2000);

    Particle.function("rgbTrigger",TriggerRGBNotification);
    Particle.function("bzzTrigger",TriggeBzzrNotification);
    Particle.function("SetupDrone", SetupDrone);
    Particle.function("SetupWorker", SetupWorker);

    //TODO: Eliminar esto, usar variable para GUARDAR y SETEAR los DATOS
    oBeeOne.SetUpDrone("ID:01-TYPE:SW-PIN1:D21-PIN2:000-BZZR:0-RGB:2-TIMER:20000-FIELDID:1-MODE:P-WID:01-WTIMER:3000");
    //oBeeOne.SetUpDrone("ID:02-TYPE:SW-PIN1:A21-PIN2:000-BZZR:4-RGB:4-TIMER:20000-FIELDID:5-FIELDNAME:BigButton-MODE:P-WID:02-WTIMER:3000");

    //Temperature
    //oBeeOne.SetUpDrone("ID:01-TYPE:TEMP-PIN1:D11-PIN2:000-BZZR:0-RGB:0:TIMER:000-FIELDID:2-FIELDNAME:Temperature-MODE:P-WID:00-WTIMER:000");

    //Presence
    oBeeOne.SetUpDrone("ID:01-TYPE:DIGITAL-PIN1:D32-PIN2:000-BZZR:0-RGB:1:TIMER:000-FIELDID:3-FIELDNAME:Presence-MODE:P-WID:00-WTIMER:000");
    //Light
    //oBeeOne.SetUpDrone("ID:02-TYPE:DIGITAL-PIN1:A22-PIN2:000-BZZR:0-RGB:0:TIMER:000-FIELDID:4-FIELDNAME:Light-MODE:T-WID:00-WTIMER:000");

    oBeeOne.SetUpWorker("ID:01-PIN:A11-BZZR:0-RGB:0-TYPE:A-TIMER:5000");
    oBeeOne.SetUpWorker("ID:02-PIN:A12-BZZR:0-RGB:0-TYPE:A-TIMER:5000");

    ms = 0;
    msLast = 0;

    //Start ThingSpeak
    ThingSpeak.begin(client);

}

/*Public Function Particle API */
int TriggerRGBNotification(String command)
{
  oBeeOne.RGBNotification(command.toInt());
  return 1;
}

int TriggeBzzrNotification(String command)
{
  oBeeOne.BzzrNotification(command.toInt());
  return 1;
}

int SetupDrone(String command)
{
  oBeeOne.SetUpDrone(command);
  return 1;
}

int SetupWorker(String command)
{
  oBeeOne.SetUpWorker(command);
  return 1;
}


void loop() {

    //delay(50);
    ms = millis();

    oBeeOne.Update();

    sensor oSensor;
    sensor_event oEvent;

    oBeeOne.droneSwitch1.GetSensor(&oSensor);
    oBeeOne.droneSwitch1.GetEvent(&oEvent);

    oBeeOne.HandleWorker(oSensor, oEvent);
    oBeeOne.HandleNotification(oSensor, oEvent);

    GetPublishValuesSwitch1(oSensor.fieldID);

    //oBeeOne.droneSwitch2.GetSensor(&oSensor);
    //oBeeOne.droneSwitch2.GetEvent(&oEvent);

    //oBeeOne.HandleWorker(oSensor, oEvent);
    //oBeeOne.HandleNotification(oSensor, oEvent);

    //GetPublishValuesSwitch2(oSensor.fieldID, oSensor.fieldName);

    //TODO: DEfinir que sensore van en tiempo real y cuales no
    if(ms - msLast > publishTime)
    {
    //oBeeOne.droneTemperature1.GetSensor(&oSensor);
    //oBeeOne.droneTemperature1.GetEvent(&oEvent);


    //oBeeOne.HandleWorker(oSensor, oEvent);
    //oBeeOne.HandleNotification(oSensor, oEvent);

    //GetPublishValuesTemp1(oSensor.fieldID, oSensor.fieldName);

    }

    oBeeOne.droneDigital1.GetSensor(&oSensor);
    oBeeOne.droneDigital1.GetEvent(&oEvent);

    oBeeOne.HandleWorker(oSensor, oEvent);
    oBeeOne.HandleNotification(oSensor, oEvent);

    GetPublishValuesDigital1(oSensor.fieldID);

    //oBeeOne.droneDigital2.GetSensor(&oSensor);
    //oBeeOne.droneDigital2.GetEvent(&oEvent);

    //oBeeOne.HandleWorker(oSensor, oEvent);
    //oBeeOne.HandleNotification(oSensor, oEvent);

    //GetPublishValuesDigital2(oSensor.fieldID, oSensor.fieldName);

    Publish();
}

void GetPublishValuesSwitch1(int fieldID)
{
    if(ms - msLast > publishTime)
    {
        sensor_event oEvent;
        oBeeOne.droneSwitch1.Publish(&oEvent);

        //GetValue
        //Serial.println("SetField: " + String(oEvent.value));
        ThingSpeak.setField(fieldID,oEvent.value);

        //Losant
        //state[fieldName] = oEvent.value;
    }
}

void GetPublishValuesSwitch2(int fieldID, String fieldName)
{
    if(ms - msLast > publishTime)
    {
        sensor_event oEvent;
        oBeeOne.droneSwitch2.Publish(&oEvent);

        //GetValue
        //Serial.println("SetField: " + String(oEvent.value));
        ThingSpeak.setField(fieldID,oEvent.value);
        //Losant
        //state[fieldName] = oEvent.value;
    }
}

void GetPublishValuesTemp1(int fieldID, String fieldName)
{
    if(ms - msLast > publishTime)
    {
        sensor_event oEvent;
        oBeeOne.droneTemperature1.Publish(&oEvent);

        //GetValue
        Serial.println("SetField: " + String(oEvent.value));
        Serial.println("FieldID: " + String(fieldID));
        ThingSpeak.setField(fieldID,oEvent.value);
        //Losant
        //state[fieldName] = oEvent.value;
    }
}

void GetPublishValuesDigital1(int fieldID)
{
    if(ms - msLast > publishTime)
    {
        sensor_event oEvent;
        oBeeOne.droneDigital1.Publish(&oEvent);

        //GetValue
        //Serial.println("SetField: " + String(oEvent.value));
        ThingSpeak.setField(fieldID,oEvent.value);
        //Losant
        //state[fieldName] = oEvent.value;

    }
}

void GetPublishValuesDigital2(int fieldID, String fieldName)
{
    if(ms - msLast > publishTime)
    {
        sensor_event oEvent;
        oBeeOne.droneDigital2.Publish(&oEvent);

        //GetValue
        //Serial.println("SetField: " + String(oEvent.value));
        ThingSpeak.setField(fieldID,oEvent.value);
        //Losant
        //state[fieldName] = oEvent.value;
    }
}

void Publish()
{
    if(ms - msLast > publishTime)
    {

        msLast = ms;

        //Publish to ThingSpeak
        Serial.println("Publish ThingSpeak");
        ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

        //Test
        //Particle.publish("losant", 15, PRIVATE);

    }
}
