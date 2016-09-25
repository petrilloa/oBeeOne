
#ifndef _drone_h
#define _drone_h

#include "application.h"

typedef enum
{
    SENSOR_MODE_PULSE       = (1),
    SENSOR_MODE_TIME        = (2)
} sensor_mode;

typedef struct
{
    float value;                //sensor value
    float lastValue;            //sensor lastValue
    float acumulatedValue;       //sensor acumulatedValue (between Publish)
    bool changed;               //value changed since last read

    int timestamp;              //LastRead
    int lastChange;             //Elapsed time since last change

    bool triggerNotification;   //If must trigger a Notification
    bool triggerWorker;         //If must trigger a Worker
} sensor_event;

typedef struct
{
    int droneID;
    int type;                       //Sensor type example SENSOR_TYPE_SWITCH
    int pin;                        //Pin conected to Photon
    int pin2;                       //Used for some sensors
    int buzzerNotificationID;       /**< Has a Buzer ID */
    int rgbNotificationID;          /**< Has a RGB ID */
    int notificationElapsedTime;    /**< Notification en elapsed Time */

    int fieldID;                    /**< Thingspeak field */
    //String fieldName;               /** Losant field*/
    sensor_mode mode;                /**< Pulse or Time Sensor*/

    int workerID = -1;                   /**< Asociated with a Worker */
    int workerElapsedTime = 0;          /**< WorkerDelayedTime */
} sensor;

typedef enum
{
    SENSOR_TYPE_SWITCH      = (1),
    SENSOR_TYPE_TEMP        = (2),
    SENSOR_TYPE_DIGITAL     = (3)
} sensor_type;



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

class Drone {
    public:

        Drone() {}
        //virtual ~Drone() {}

        //Thease must be defined by the subClass
        virtual void SetUpSensor(sensor);
        virtual void GetEvent(sensor_event*);
        virtual void GetSensor(sensor*);
        virtual void Publish(sensor_event*);

};

#endif
