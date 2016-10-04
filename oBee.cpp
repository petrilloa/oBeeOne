#include "oBee.h"

oBee::oBee()
{
    //oRGB();
    //oRGB(N_PIXELS, PIN_RGB, WS2812);
    //oRGB(3, 1, WS2812);
    //oRGB.Setup(N_PIXELS, PIN_RGB);
    oRGB.begin();
    oRGB.Repeat = true;

    //Default anim
    oRGB.RainbowCycle(5);
    //color1 = oRGB.Color(255,0,0);
    //color2 = oRGB.Color(0,0,255);

    //oRGB.Scanner(color1, 100);
    //oRGB.TheaterChase(color1, color2, 100, FORWARD);

    oSound.pin = PIN_Buzzer;
    char *song = oSound.song_startup;
    oSound.Begin(song);

    //TODO: Poner en variable y usar para todo los One_Wire
    //oneWire.Setup(PIN_D12);
}

void oBee::Update()
{
    oRGB.Update();
    oSound.Update();
}

void oBee::SetUpDrone(String str)
{
    Serial.println("SetupDrone-DETAIL");
    Serial.println("FULL:" + str);

    //SetupDrone("ID:00-TYPE:SW-PIN1:D21-PIN2:000-BZZR:2-RGB:2-TIMER:1000-FIELDID:1-FIELDNAME:Temperature-MODE:P-WID:00-WTIMER:2000");

    int posID = str.indexOf("ID:");
    int posType = str.indexOf("-TYPE:");
    int posPIN1 = str.indexOf("-PIN1:");
    int posPIN2 = str.indexOf("-PIN2:");
    int posBZZR = str.indexOf("-BZZR:");
    int posRGB = str.indexOf("-RGB:");
    int posTimer = str.indexOf("-TIMER:");
    int posFieldID = str.indexOf("-FIELDID:");
    //int posFieldName = str.indexOf("-FIELDNAME:");
    int posMode = str.indexOf("-MODE:");
    int posWorkerID = str.indexOf("-WID:");
    int posWTimer = str.indexOf("-WTIMER:");

    String strId = str.substring (posID+3, posType);   // get 01
    //Serial.println("SetupDrone-ID:" + strId );

    String strType = str.substring (posType+6, posPIN1);
    //Serial.println("SetupDrone-TYPE:" + strType);

    String strPin1 = str.substring (posPIN1+6, posPIN2);
    Serial.println("SetupDrone-PIN1:" + strPin1);

    String strPin2 = str.substring (posPIN2+6, posBZZR);
    //Serial.println("SetupDrone-PIN2:" + strPin2);

    String strBzzr = str.substring (posBZZR+6, posRGB);
    //Serial.println("SetupDrone-BZZR:" + strBzzr);

    String strRGB = str.substring (posRGB+5, posTimer);
    //Serial.println("SetupDrone-RGB:" + strRGB);

    String strTimer = str.substring (posTimer+7, posFieldID);
    //Serial.println("SetupDrone-TIMER:" + strTimer);

    String strFieldId = str.substring (posFieldID+9, posMode);
    //Serial.println("SetupDrone-FELDID:" + strFieldId);

    //String strFieldName = str.substring (posFieldName+11, posMode);
    //Serial.println("SetupDrone-FIELDNAME:" + strFieldName);

    String strMode = str.substring (posMode+6, posWorkerID);
    Serial.println("SetupDrone-MODE:" + strMode);

    String strWID = str.substring (posWorkerID+5, posWTimer);
    Serial.println("SetupDrone-WORKERID:" + strWID);

    String strWTimer = str.substring (posWTimer+8);
    Serial.println("SetupDrone-WORKERTIME:" + strWTimer);


    int droneID = strId.toInt();
    int pin1 = GetPinValue(strPin1);
    int pin2 = GetPinValue(strPin2);
    int fieldID = strFieldId.toInt();
    int buzzerNotificationID = strBzzr.toInt();
    int rgbNotificationID = strRGB.toInt();
    int notificationTime = strTimer.toInt();
    int workerID = strWID.toInt();
    int workerTimer = strWTimer.toInt();


    sensor_type dtype = SENSOR_TYPE_SWITCH;

    if(strType == "SW")
    {
        dtype = SENSOR_TYPE_SWITCH;
    }
    else if (strType == "TEMP")
    {
        dtype = SENSOR_TYPE_TEMP;
    }
    else if (strType == "DIGITAL")
    {
        dtype = SENSOR_TYPE_DIGITAL;
    }

    //TODO: droneMode dmode = dmPULSE;
    sensor_mode dmode = SENSOR_MODE_PULSE;

    if(strMode == "P")
    {
        dmode = SENSOR_MODE_PULSE;
    }
    else
    {
        dmode = SENSOR_MODE_TIME;
    }

    sensor oSensor;

    oSensor.droneID = droneID;
    oSensor.type = dtype;
    oSensor.pin = pin1;
    oSensor.pin2 = pin2;
    oSensor.buzzerNotificationID = buzzerNotificationID;
    oSensor.rgbNotificationID = rgbNotificationID;
    oSensor.notificationElapsedTime = notificationTime;
    oSensor.fieldID = fieldID;
    //oSensor.fieldName = strFieldName;
    oSensor.mode = dmode;
    oSensor.workerID = workerID;
    oSensor.workerElapsedTime = workerTimer;

    Serial.println("SensorPIN: " + String(pin1));

    switch (oSensor.type)
    {
        case SENSOR_TYPE_SWITCH:
            SetUpDroneSwitch(oSensor);
            break;
        case SENSOR_TYPE_TEMP:
            SetUpDroneTemperature(oSensor);
            break;
        case SENSOR_TYPE_DIGITAL:
            SetUpDroneDigital(oSensor);
            break;
    }


}

void oBee::SetUpDroneSwitch(sensor oSensor)
{
    DroneSwitch *oDroneSwitch = new DroneSwitch();
    oDroneSwitch->SetUpSensor(oSensor);
    droneSwitchList.add(oDroneSwitch);
}

void oBee::SetUpDroneTemperature(sensor oSensor)
{
    //TODO: Ver si lo dejamos aca
    oneWire.Setup(oSensor.pin);

    DroneTemperature *oDroneTemperature = new DroneTemperature;
    oDroneTemperature->SetUpSensor(oSensor, &oneWire);
    droneTemperatureList.add(oDroneTemperature);
}

void oBee::SetUpDroneDigital(sensor oSensor)
{
  DroneDigital *oDroneDigital = new DroneDigital();
  oDroneDigital->SetUpSensor(oSensor);
  droneDigitalList.add(oDroneDigital);
}

void oBee::ClearLists()
{
  //Clear list to initial SetUp
  droneSwitchList.clear();
  droneTemperatureList.clear();
  droneDigitalList.clear();
  workerList.clear();
}

int oBee::GetPinValue(String strPIN)
{
    int pinValue;

    if(strPIN == "D11")
        pinValue = PIN_D11;
    else if (strPIN == "D12")
        pinValue = PIN_D12;
    else if (strPIN == "D21")
        pinValue = PIN_D21;
    else if (strPIN == "D22")
        pinValue = PIN_D22;
    else if (strPIN == "D31")
        pinValue = PIN_D31;
    else if (strPIN == "D32")
        pinValue = PIN_D32;
    else if (strPIN == "A11")
        pinValue = PIN_A11;
    else if (strPIN == "A12")
        pinValue = PIN_A12;
    else if (strPIN == "A21")
        pinValue = PIN_A21;
    else if (strPIN == "A22")
        pinValue = PIN_A22;


    return pinValue;
}

void oBee::SetUpWorker(String str)
{
    //CreateWorker("ID:00-PIN:D11-BZZR:1-RGB:1-TYPE:M-TIMER:005");
    Serial.println("SetupWorker-DETAIL");
    Serial.println("FULL:" + str);

    int posID = str.indexOf("ID:");
    int posPIN = str.indexOf("-PIN:");
    int posBZZR = str.indexOf("-BZZR:");
    int posRGB = str.indexOf("-RGB:");
    int posType = str.indexOf("-TYPE:");
    int posTimer = str.indexOf("-TIMER:");
    //int posTallbackID = str.indexOf("-TALLBACKID:");


    String strId = str.substring (posID+3, posPIN);   // get 01
    Serial.println("SetupWorker-ID:" + strId );

    String strPin = str.substring (posPIN+5, posBZZR);
    Serial.println("SetupWorker-PIN:" + strPin);

    String strBzzr = str.substring (posBZZR+6, posRGB);
    Serial.println("SetupWorker-BZZR:" + strBzzr);

    String strRGB = str.substring (posRGB+5, posType);
    Serial.println("SetupWorker-RGB:" + strRGB);

    String strType = str.substring (posType+6, posTimer);
    Serial.println("SetupWorker-TYPE:" + strType);

    String strTimer = str.substring (posTimer+7);
    Serial.println("SetupWorker-TIMER:" + strTimer);

    //String strTallBackID = str.substring (posTallbackID+9);
    //Serial.println("SetupWorker-TALLBACKID:" + strTallBackID);

    int workerID = strId.toInt();
    int pin = GetPinValue(strPin);
    //String tallbackID = strTallBackID;
    int buzzerNotificationID = strBzzr.toInt();
    int rgbNotificationID = strRGB.toInt();
    int autoTimer = strTimer.toInt();

    worker_type wtype = WORKER_MANUAL;


    if(strType == "M")
    {
        wtype = WORKER_MANUAL;
    }
    else
    {
        wtype = WORKER_AUTO;
    }

    Worker *worker = new Worker();

    worker->workerID = workerID;
    worker->pin = pin;
    //worker.tallBAckID
    worker->buzzerNotificationID = buzzerNotificationID;
    worker->rgbNotificationID = rgbNotificationID;
    worker->autoTimer = autoTimer;
    worker->type = wtype;

    worker->SetUpWorker();

    Serial.println("WorkerID: " + String(worker->workerID));

    workerList.add(worker->workerID-1, worker);

}

void oBee::HandleWorker(sensor oSensor, sensor_event oEvent)
{
    if (oSensor.workerID != 0) //Check if has a Worker asociated
    {
      //Serial.println("Worker: " + String(oSensor.workerID));
      int active = -1;
      int workerAutoOff = 0;

      Worker *worker = workerList.get(oSensor.workerID-1);
      workerAutoOff = worker->autoTimer;

      if(oEvent.triggerWorker)
      {
          active = 1;
          //Serial.println("Active 1");
      }
      else if (!oEvent.triggerWorker && workerAutoOff == 0) //if value is 0 -> OFF
      {
         active = 0;
         //Serial.println("Active 0");
      }

      if (active == 1)
      {
        //Serial.println("WorkerID_ON: " + String(worker->workerID));
        worker->WorkerON();
      }
      else if(active == 0)
      {
          worker->WorkerOFF();
      }
      //If active -1 -> Check AutoWorker
      worker->AutoOffWorker();
    }
}

void oBee::HandleNotification(sensor oSensor, sensor_event oEvent)
{
    if (oEvent.triggerNotification)
    {
        //Serial.println("Notification");

        if(oSensor.rgbNotificationID != 0)
          RGBNotification(oSensor.rgbNotificationID);

        if(oSensor.buzzerNotificationID != 0)
          BzzrNotification(oSensor.buzzerNotificationID);
    }
}

void oBee::RGBNotification(int id)
{
  if(!oRGB.Running)
  {
    color1 = oRGB.Color(255,0,0);
    color2 = oRGB.Color(0,0,255);

    switch(id)
    {
      case 1:
        //Notification 1
        color1 = oRGB.Color(100, 55, 0);
        color2 = oRGB.Color(0, 160, 255);
        oRGB.Repeat = false;
        oRGB.TheaterChase(color1, color2, 100, FORWARD);
        break;
      case 2:
        //Notification 2
        color1 = oRGB.Color(255, 0, 139);
        color2 = oRGB.Color(167, 255, 0);
        oRGB.Repeat = false;
        oRGB.TheaterChase(color1, color2, 100, FORWARD);
        break;
      case 3:
        //Notification 3
        color1 = oRGB.Color(255, 0, 50);
        color2 = oRGB.Color(0, 50, 255);
        oRGB.Repeat = false;
        oRGB.TheaterChase(color1, color2, 100, FORWARD);
        break;
      case 4:
        //Alert 1
        color1 = oRGB.Color(255, 113, 0);
        oRGB.Repeat = false;
        oRGB.Scanner(color1, 100);
        break;
      case 5:
        //Alert 2
        color1 = oRGB.Color(2, 96, 255);
        oRGB.Repeat = false;
        oRGB.Scanner(color1, 100);
        break;
      case 6:
        //Alert 2
        color1 = oRGB.Color(24, 255, 0);
        oRGB.Repeat = false;
        oRGB.Scanner(color1, 100);
        break;
    };
  }

}

void oBee::BzzrNotification(int id)
{
  if (!oSound.IsPlaying())
  {
    char *song;
    switch(id)
    {
      case 1:
          song = oSound.song_notification1;
          break;
      case 2:
          song = oSound.song_notification2;
          break;
      case 3:
          song = oSound.song_notification3;
          break;
      case 4:
          song = oSound.song_alamr1;
          break;
      case 5:
          song = oSound.song_alamr2;
          break;
      case 6:
          song = oSound.song_alamr3;
          break;
      case 7:
          song = oSound.song_error;
          break;
      case 8:
          song = oSound.song_custom1;
          break;
      case 9:
          song = oSound.song_custom2;
          break;
      case 10:
          song = oSound.song_custom3;
          break;
    }
    oSound.Begin(song);
  }

}
