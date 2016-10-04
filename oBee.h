/* oBee.h */
#include "Drone.h"
#include "DroneSwitch.h"
#include "DroneTemperature.h"
#include "DroneDigital.h"

#include "oBeeRGB.h"
#include "oBeeSound.h"
#include "Worker.h"

#include <LinkedList.h>

#ifndef _oBee_h
#define _oBee_h

const int PIN_Buzzer = D0;
const int PIN_RGB = D1;

const int PIN_D11 = D2;
const int PIN_D12 = D3;
const int PIN_D21 = D5;
const int PIN_D22 = D4;
const int PIN_D31 = D7;
const int PIN_D32 = D6;
const int PIN_A11 = A0;
const int PIN_A12 = A1;
const int PIN_A21 = A2;
const int PIN_A22 = A3;

const int N_PIXELS = 3;

class oBee
{
    public:
        oBee();

        oBeeRGB oRGB;
        oBeeSound oSound;

        void Update();
        //void NotificationComplete();

        void SetUpDrone(String str);
        void SetUpDroneSwitch(sensor oSensor);
        void SetUpDroneTemperature(sensor oSensor);
        void SetUpDroneDigital(sensor oSensor);

        void ClearLists();

        LinkedList<DroneSwitch*> droneSwitchList = LinkedList<DroneSwitch*>();
        LinkedList<DroneTemperature*> droneTemperatureList = LinkedList<DroneTemperature*>();
        LinkedList<DroneDigital*> droneDigitalList = LinkedList<DroneDigital*>();

        //DronePresence;
        //DroneLight;
        //DroneButton; ???
        //DroneLine1;
        //DroneLine2;
        //DroneLine3;


        //DroneDistance;
        //DroneAmbientTemperature;
        //DroneAmbientHumidity
        //DroneTemperature1;
        //DroneTemperature2;
        //DroneSoilMoisture1;
        //DroneSoilMoisture2;
        //DroneRain;
        //DroneWaterLevel1;
        //DroneWaterLevel2;
        //DroneSound;
        //DroneLightLevel;
        //DroneAmp;
        //DroneVolt;
        //DroneGas;


        int GetPinValue(String strPIN);

        void SetUpWorker(String str);
        LinkedList<Worker*> workerList = LinkedList<Worker*>();


        void HandleWorker(sensor oSensor, sensor_event oEvent);
        void HandleNotification(sensor oSensor, sensor_event oEvent);

        void RGBNotification(int id);
        void BzzrNotification(int id);

        void NotificationComplete();

        uint32_t color1, color2, lastColor;

        long msLast = 0;

    private:

        unsigned int _channelNumber;
        String writeAPIKey;
        String buzzerTallBackID;
        String rgbTallBackID;

        OneWire oneWire;
};

#endif
