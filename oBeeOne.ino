
#include "ThingSpeak.h"
//#include "MQTT.h"
#include "oBee.h"
#include "ArduinoJson.h"

oBee oBeeOne;
/* Thingspeak */
int myChannelNumber = -1;
//unsigned long channelNumber = 141202;
String myWriteAPIKey = "my-write-api";
//const char* charAPIKey = "L1UKP3FYTXHG69F2";
TCPClient client;

String losantWebHook = "losant-webhook";
String losantDeviceId = "";

double publishTime = 15000;

unsigned long ms;
unsigned long msLast;

int oBeeID = 1;

typedef struct
{
  int fieldID;
  float value;
} fieldValue;

LinkedList<fieldValue*> fieldList = LinkedList<fieldValue*>();


/************Setup**************/
/*******************************/
void setup() {

    Serial.begin(19200);

    //delay(2000);
    while (!Spark.connected()) {
    Serial.print("+"); delay(100);
    }


    Particle.subscribe(System.deviceID() +"/hook-response/Firebase", FireBaseHandler, MY_DEVICES);

    Particle.variable("thingChannel", myChannelNumber);
    Particle.variable("thingWrite", myWriteAPIKey);
    Particle.variable("publishTime", publishTime);
    Particle.variable("losantWeb", losantWebHook);
    Particle.variable("losantDevId", losantDeviceId);

    Particle.function("rgbTrigger",TriggerRGBNotification);
    Particle.function("bzzTrigger",TriggeBzzrNotification);


    ms = 0;
    msLast = 0;

    //Start ThingSpeak
    ThingSpeak.begin(client);

    //Start Losant

    //Obtener Device Name
    Particle.subscribe("spark/", DeviceHandler);
    Particle.publish("spark/device/name");
}

/******* Handler of Publish DEVICE Name********/
void DeviceHandler(const char *topic, const char *data) {
    Serial.println("received " + String(topic) + ": " + String(data));
    //delay(1000);
    //TODO: Obtener ID del NOMBRE! oBee_xxx
    oBeeID = String(data).substring(5).toInt();

    Serial.println("oBeeID " + String(oBeeID));

    //llamado a WebHook para obtener variables
    //String data =  "{\"id\":\"1\"}";
    String dataFirebase = "{\"id\":\"" + String(oBeeID) + "\"}";
    Particle.publish("Firebase", dataFirebase, PRIVATE);

}

/******* Handler of FireBase********/
String strStart = "{\"oBee";
String strEnd = "}}";
String fullMessage;

void FireBaseHandler(const char *event, const char *data) {
  // Handle the webhook response
  Serial.println("***********FireBaseHandler***********");
  Serial.println(String(data));

  //Test if final data
  if (String(data).startsWith(strStart))
  {
    fullMessage = String(data);
    Serial.println("***********START***********");

    //Si no termina, salir y esperar el resto
    if(!String(data).endsWith(strEnd))
    {
      Serial.println("***********START AND END***********");
      return;
    }
  }
  else if (!String(data).endsWith(strEnd))
  {
    //Agrego data al mensaje total
    fullMessage += String(data);
    Serial.println("***********LOOP***********");
    return;
    //Salgo de la rutina y se vuelve a llamar hasta que se completa el mensaje
  }
  else{
    //Completo el mensaje
    fullMessage += String(data);
    Serial.println("***********END***********");

    Serial.println("FullMessage: " + fullMessage);
  }




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
  losantWebHook = oBeeJS["particleWebHook"].asString();
  losantDeviceId = oBeeJS["losantDeviceId"].asString();

  Serial.println("LosantDeviceID: " + losantDeviceId);

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
    HandleDroneAnalog();
    HandleDroneTemperature();
    HandleDroneAmbientTemp();

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
      Serial.println("SetField-" + String(oSensor.fieldID) + ": "+ String(oEvent.value));
      ThingSpeak.setField(oSensor.fieldID,oEvent.value);

      //Add to collection - LOSANT
      fieldValue *oValue = new fieldValue();

      oValue->fieldID = oSensor.fieldID;
      oValue->value = oEvent.value;

      fieldList.add(oValue);

      //Check notification publish
      if(oSensor.notificationFieldID != 0)
      {
        //Add to collection - LOSANT
        fieldValue *oValue = new fieldValue();

        oValue->fieldID = oSensor.notificationFieldID;
        oValue->value = oEvent.acumulatedNotification;

        fieldList.add(oValue);

        ThingSpeak.setField(oSensor.notificationFieldID,oEvent.acumulatedNotification);
      }

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
        Serial.println("SetField-" + String(oSensor.fieldID) + ": "+ String(oEvent.value));
        ThingSpeak.setField(oSensor.fieldID,oEvent.value);

        //Add to collection - LOSANT
        fieldValue *oValue = new fieldValue();

        oValue->fieldID = oSensor.fieldID;
        oValue->value = oEvent.value;

        fieldList.add(oValue);

        //Check notification publish
        if(oSensor.notificationFieldID != 0)
        {
          //Add to collection - LOSANT
          fieldValue *oValue = new fieldValue();

          oValue->fieldID = oSensor.notificationFieldID;
          oValue->value = oEvent.acumulatedNotification;

          fieldList.add(oValue);

          ThingSpeak.setField(oSensor.notificationFieldID,oEvent.acumulatedNotification);
        }

    }
  }
}

void HandleDroneAnalog()
{
  sensor oSensor;
  sensor_event oEvent;

  //Serial.println("1");
  //delay(500);

  int listSize = oBeeOne.droneAnalogList.size();

  //Serial.println("listSize: " + String(listSize));
  //Serial.println("2");
  //delay(500);

  for (int h = 0; h < listSize; h++)
  {
  DroneAnalog *droneAnalog;

  //Serial.println("GetDrone: " + String(h));
  //delay(500);
  droneAnalog = oBeeOne.droneAnalogList.get(h);

  //Serial.println("3");
  //delay(500);
  droneAnalog->GetSensor(&oSensor);
  //Serial.println("4");
  //delay(500);
  droneAnalog->GetEvent(&oEvent);
  //Serial.println("5");
  //delay(500);

  oBeeOne.HandleWorker(oSensor, oEvent);
  oBeeOne.HandleNotification(oSensor, oEvent);
  //Serial.println("6");

  //IF Publish time
    if(ms - msLast > publishTime)
    {
        droneAnalog->Publish(&oEvent);

        //GetValue
        Serial.println("SetField-" + String(oSensor.fieldID) + ": "+ String(oEvent.value));
        ThingSpeak.setField(oSensor.fieldID,oEvent.value);

        //Add to collection - LOSANT
        fieldValue *oValue = new fieldValue();

        oValue->fieldID = oSensor.fieldID;
        oValue->value = oEvent.value;

        fieldList.add(oValue);

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
        Serial.println("SetField-" + String(oSensor.fieldID) + ": "+ String(oEvent.value));
        ThingSpeak.setField(oSensor.fieldID,oEvent.value);

        //Add to collection - LOSANT
        fieldValue *oValue = new fieldValue();

        oValue->fieldID = oSensor.fieldID;
        oValue->value = oEvent.value;

        fieldList.add(oValue);

        }
  }
}

void HandleDroneAmbientTemp()
{
  sensor oSensor;
  sensor_event oEvent;

  int listSize = oBeeOne.droneAmbientTempList.size();

  for (int h = 0; h < listSize; h++)
  {
    //IF Publish time - FOR TEMPERATURE ONLY WHEN PUBLISH! Evitar saturar el BUS -
    //Testear con varios sensores...
      if(ms - msLast > publishTime)
      {
        DroneAmbientTemp *droneAmbientTemp;

        droneAmbientTemp = oBeeOne.droneAmbientTempList.get(h);

        droneAmbientTemp->GetSensor(&oSensor);
        droneAmbientTemp->GetEvent(&oEvent);

        oBeeOne.HandleWorker(oSensor, oEvent);
        oBeeOne.HandleNotification(oSensor, oEvent);

        droneAmbientTemp->Publish(&oEvent);

        //GetValue
        Serial.println("SetField-" + String(oSensor.fieldID) + ": "+ String(oEvent.value));
        ThingSpeak.setField(oSensor.fieldID,oEvent.value);

        //Add to collection - LOSANT
        fieldValue *oValue = new fieldValue();

        oValue->fieldID = oSensor.fieldID;
        oValue->value = oEvent.value;

        fieldList.add(oValue);

        }
  }
}


/*********Publish to the cloud************/
/*******************************/
int randomNumber(int minVal, int maxVal)
{
  // int rand(void); included by default from newlib
  return rand() % (maxVal-minVal+1) + minVal;
}

void Publish()
{
    if(ms - msLast > publishTime)
    {
        msLast = ms;

        //TODO: quitar ThingSpeak
        //const char* charAPIKey = myWriteAPIKey.c_str();
        //unsigned long channelNumber = myChannelNumber;

        //ThingSpeak.writeFields(channelNumber, charAPIKey);

        //Test WebHook Losant
        //String losantComand = "temperature\":\"" + String(randomNumber(15,31))+ "";
        //String losantComand = "temperature\":\"15";

        //Particle.publish("Losant", jsonString, PRIVATE);

        //char publishString[256];
        //sprintf(publishString,"{\"temp\": %i, \"door\": %i }",randomNumber(15,31), randomNumber(1,4));
        //sprintf(publishString,"{\"status\": \"%i\", \"oBeeId\": \"" + losantDeviceId + "\", \"field-1\": %i, \"field-2\": %i, \"field-3\": %i, \"field-4\": %i, \"field-5\": %i, \"field-6\": %i, \"field-7\": %i, \"field-8\": %i, \"field-9\": %i, \"field-10\": %i}",1, randomNumber(0,4),randomNumber(19,27),randomNumber(0,1),randomNumber(0,1),randomNumber(0,7),0,0,0,0,0 );


        //Particle.publish(losantWebHook,publishString , PRIVATE);

        // Build the json payload: { "data" : { "temp" : val }}

        StaticJsonBuffer<1024> jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();


        //TODO: Ver como usar el status
        root["status"] = 1;
        root["oBeeId"] = losantDeviceId.c_str();

        root["field-1"] = "";
        root["field-2"] = "";
        root["field-3"] = "";
        root["field-4"] = "";
        root["field-5"] = "";
        root["field-6"] = "";
        root["field-7"] = "";
        root["field-8"] = "";
        root["field-9"] = "";
        root["field-10"] = "";

        for (int i = 0; i < fieldList.size(); i++)
        {
          //Serial.println("field-" + String(i) + ": " +  String(fieldList.get(i)));
          fieldValue *oValue = fieldList.get(i);

          root["field-" + String(oValue->fieldID)] = oValue->value;

          Serial.println("Field: " + String(oValue->fieldID) + " Value: " + String(oValue->value));

          //Delete object from Memory
          delete oValue;


        }

        // Get JSON string.
        char buffer[1024];
        root.printTo(Serial);
        root.printTo(buffer, sizeof(buffer));
        Serial.println("---------------");

        Particle.publish(losantWebHook, buffer , PRIVATE);

        //Clear the values
        fieldList.clear();


    }
}
