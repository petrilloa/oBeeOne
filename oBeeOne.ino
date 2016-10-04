
#include "ThingSpeak.h"
#include "oBee.h"
#include <ArduinoJson.h>

oBee oBeeOne;
/* Thingspeak */
int myChannelNumber = 141202;
//unsigned long channelNumber = 141202;

String myWriteAPIKey = "L1UKP3FYTXHG69F2";
//const char* charAPIKey = "L1UKP3FYTXHG69F2";

TCPClient client;

double publishTime = 15000;

unsigned long ms;
unsigned long msLast;

int oBeeID = 1;


/************Setup**************/
/*******************************/
void setup() {

    Serial.begin(19200);

    delay(2000);

    Particle.subscribe("hook-response/Firebase", FireBaseHandler, MY_DEVICES);

    Particle.variable("thingChannel", myChannelNumber);
    Particle.variable("thingWrite", myWriteAPIKey);
    Particle.variable("publishTime", publishTime);

    Particle.function("rgbTrigger",TriggerRGBNotification);
    Particle.function("bzzTrigger",TriggeBzzrNotification);


    //TODO: Eliminar esto, usar variable para GUARDAR y SETEAR los DATOS
    //oBeeOne.SetUpDrone("ID:01-TYPE:SW-PIN1:D21-PIN2:00-BZZR:0-RGB:2-TIMER:20000-FIELDID:1-MODE:P-WID:01-WTIMER:3000");
    //oBeeOne.SetUpDrone("ID:02-TYPE:SW-PIN1:A21-PIN2:00-BZZR:4-RGB:4-TIMER:20000-FIELDID:5-FIELDNAME:BigButton-MODE:P-WID:02-WTIMER:3000");

    //Temperature
    //oBeeOne.SetUpDrone("ID:03-TYPE:TEMP-PIN1:D11-PIN2:01-BZZR:0-RGB:0:TIMER:000-FIELDID:2-FIELDNAME:Temperature-MODE:P-WID:00-WTIMER:000");

    //Presence
    //oBeeOne.SetUpDrone("ID:04-TYPE:DIGITAL-PIN1:D32-PIN2:00-BZZR:0-RGB:1:TIMER:000-FIELDID:3-FIELDNAME:Presence-MODE:P-WID:00-WTIMER:000");
    //Light
    //oBeeOne.SetUpDrone("ID:05-TYPE:DIGITAL-PIN1:A22-PIN2:00-BZZR:0-RGB:0:TIMER:000-FIELDID:4-FIELDNAME:Light-MODE:T-WID:00-WTIMER:000");

    //oBeeOne.SetUpWorker("ID:01-PIN:A11-BZZR:0-RGB:0-TYPE:A-TIMER:5000");
    //oBeeOne.SetUpWorker("ID:02-PIN:A12-BZZR:0-RGB:0-TYPE:A-TIMER:5000");

    ms = 0;
    msLast = 0;

    //Start ThingSpeak
    ThingSpeak.begin(client);

    //Obtener Device Name
    Particle.subscribe("spark/", DeviceHandler);
    Particle.publish("spark/device/name");


}

/******* Handler of Publish DEVICE Name********/
void DeviceHandler(const char *topic, const char *data) {
    Serial.println("received " + String(topic) + ": " + String(data));
    delay(1000);
    //TODO: Obtener ID del NOMBRE! oBee_xxx
    oBeeID = String(data).substring(5).toInt();

    Serial.println("oBeeID " + String(oBeeID));

    //llamado a WebHook para obtener variables
    //String data =  "{\"id\":\"1\"}";
    String dataFirebase = "{\"id\":\"" + String(oBeeID) + "\"}";
    Particle.publish("Firebase", dataFirebase, PRIVATE);
}

/******* Handler of FireBase********/
String strEnd = "}}";
String fullMessage;

void FireBaseHandler(const char *event, const char *data) {
  // Handle the webhook response
  Serial.println(String(data));

  //Test if final data
  if (!String(data).endsWith(strEnd))
  {
    //Agrego data al mensaje total
    fullMessage += String(data);
    return;

    //Salgo de la rutina y se vuelve a llamar hasta que se completa el mensaje
  }

  //Completo el mensaje
  fullMessage += String(data);
  Serial.println(fullMessage);

  StaticJsonBuffer<2048> jsonBuffer;
  //char *dataCopy = strdup(data);
  //char *dataCopy = fullMessage.c_str();
  char *dataCopy = strdup(fullMessage);

  JsonObject& root = jsonBuffer.parseObject(dataCopy);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  JsonObject & oBeeJS = root["oBee-" + String(oBeeID)];  // query root
  //Get variables

  publishTime = (int)oBeeJS["publishTime"];

  myChannelNumber = (int)oBeeJS["thingChannel"];
  myWriteAPIKey = oBeeJS["thingWrite"].asString();

  String strNode;

  //Check for Drones
  for(int i=1; i < 11; i++)
  {
    if (oBeeJS.containsKey("Drone" + String(i)))
    {
        strNode = oBeeJS["Drone" + String(i)].asString();
        oBeeOne.SetUpDrone(strNode);
    }
  }

  //Check for Workers
  for(int i=1; i < 11; i++)
  {
    if (oBeeJS.containsKey("Worker" + String(i)))
    {
        strNode = oBeeJS["Worker" + String(i)].asString();
        oBeeOne.SetUpWorker(strNode);
    }
  }
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

/*******************************/

/*********Main loop ************/
/*******************************/
void loop() {

    ms = millis();

    oBeeOne.Update();

    HandleDroneSwitch();
    HandleDroneDigital();
    HandleDroneTemperature();

    //Publish to the cloud
    Publish();

    //Serial.println("Loop ms:" + String(ms));

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

  //Serial.println("1");

  int listSize = oBeeOne.droneDigitalList.size();

  //Serial.println("listSize: " + String(listSize));
  //Serial.println("2");

  for (int h = 0; h < listSize; h++)
  {
  DroneDigital *droneDigital;

  //Serial.println("GetDrone: " + String(h));
  droneDigital = oBeeOne.droneDigitalList.get(h);

  //Serial.println("3");
  droneDigital->GetSensor(&oSensor);
  //Serial.println("4");
  droneDigital->GetEvent(&oEvent);
  //Serial.println("5");

  oBeeOne.HandleWorker(oSensor, oEvent);
  oBeeOne.HandleNotification(oSensor, oEvent);
  //Serial.println("6");

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

        const char* charAPIKey = myWriteAPIKey.c_str();
        unsigned long channelNumber = myChannelNumber;

        ThingSpeak.writeFields(channelNumber, charAPIKey);

        //Test
        //Particle.publish("losant", 15, PRIVATE);

    }
}
