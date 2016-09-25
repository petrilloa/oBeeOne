
#include "ThingSpeak.h"
#include "oBee.h"
//#include "spark-dallas-temperature.h"
//#include "OneWire.h"
//#include "DroneTemperature.h"
//#include "oBeeSound.h"
//#include "Worker.h"
//#include "DroneSwitch.h"
//#include "Drone.h"
//#include "oBeeRGB.h"

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
    oBeeOne.SetUpDrone("ID:01-TYPE:TEMP-PIN1:D11-PIN2:000-BZZR:0-RGB:0:TIMER:000-FIELDID:2-FIELDNAME:Temperature-MODE:P-WID:00-WTIMER:000");

    //Presence
    oBeeOne.SetUpDrone("ID:01-TYPE:DIGITAL-PIN1:D32-PIN2:000-BZZR:0-RGB:1:TIMER:000-FIELDID:3-FIELDNAME:Presence-MODE:P-WID:00-WTIMER:000");
    //Light
    oBeeOne.SetUpDrone("ID:02-TYPE:DIGITAL-PIN1:A22-PIN2:000-BZZR:0-RGB:0:TIMER:000-FIELDID:4-FIELDNAME:Light-MODE:T-WID:00-WTIMER:000");

    oBeeOne.SetUpWorker("ID:01-PIN:A11-BZZR:0-RGB:0-TYPE:A-TIMER:5000");
    oBeeOne.SetUpWorker("ID:02-PIN:A12-BZZR:0-RGB:0-TYPE:A-TIMER:5000");

    ms = 0;
    msLast = 0;

    //Start ThingSpeak
    ThingSpeak.begin(client);

}

/*Public Function Particle API */
/*******************************/
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

/*******************************/

/*********Main loop ************/
/*******************************/
void loop() {

    //delay(50);
    ms = millis();

    oBeeOne.Update();

    HandleDroneSwitch();
    HandleDroneDigital();

    //Publish to the cloud
    Publish();
}
/*******************************/

void HandleDroneSwitch()
{
  sensor oSensor;
  sensor_event oEvent;

  int listSize = oBeeOne.droneSwitchList.size();

  for (int h = 0; h < listSize; h++)
  {
  DroneSwitch *droneSwitch;

  droneSwitch = oBeeOne.droneSwitchList.get(h);

  droneSwitch->GetSensor(&oSensor);
  droneSwitch->GetEvent(&oEvent);

  oBeeOne.HandleWorker(oSensor, oEvent);
  oBeeOne.HandleNotification(oSensor, oEvent);

  //IF Publish time
    if(ms - msLast > publishTime)
    {
        sensor_event oEvent;
        droneSwitch->Publish(&oEvent);

        //GetValue
        //Serial.println("SetField: " + String(oEvent.value));
        ThingSpeak.setField(oSensor.fieldID,oEvent.value);

        //Losant
        //state[fieldName] = oEvent.value;
    }
  }
}

void HandleDroneDigital()
{
  sensor oSensor;
  sensor_event oEvent;

  int listSize = oBeeOne.droneDigitalList.size();

  for (int h = 0; h < listSize; h++)
  {
  DroneDigital *droneDigital;

  droneDigital = oBeeOne.droneDigitalList.get(h);

  droneDigital->GetSensor(&oSensor);
  droneDigital->GetEvent(&oEvent);

  oBeeOne.HandleWorker(oSensor, oEvent);
  oBeeOne.HandleNotification(oSensor, oEvent);

  //IF Publish time
    if(ms - msLast > publishTime)
    {
        droneDigital->Publish(&oEvent);

        //GetValue
        //Serial.println("SetField: " + String(oEvent.value));
        ThingSpeak.setField(oSensor.fieldID,oEvent.value);

        //Losant
        //state[fieldName] = oEvent.value;
    }
  }
}

void HandleDroneTemperature()
{
  sensor oSensor;
  sensor_event oEvent;

  int listSize = oBeeOne.droneTemperatureList.size();

  for (int h = 0; h < listSize; h++)
  {
    //IF Publish time - FOR TEMPERATURE ONLY WHEN PUBLISH! Evitar saturar el BUS OneWIRE -
    //Testear con varios sensores...
      if(ms - msLast > publishTime)
      {
        DroneTemperature *droneTemperature;

        droneTemperature = oBeeOne.droneTemperatureList.get(h);

        droneTemperature->GetSensor(&oSensor);
        droneTemperature->GetEvent(&oEvent);

        oBeeOne.HandleWorker(oSensor, oEvent);
        oBeeOne.HandleNotification(oSensor, oEvent);


        droneTemperature->Publish(&oEvent);

        //GetValue
        //Serial.println("SetField: " + String(oEvent.value));
        ThingSpeak.setField(oSensor.fieldID,oEvent.value);

        //Losant
        //state[fieldName] = oEvent.value;
        }
  }
}

/*********Publish to the cloud************/
/*******************************/

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
