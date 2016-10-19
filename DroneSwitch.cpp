#include "DroneSwitch.h"

DroneSwitch::DroneSwitch()
{
    }

void DroneSwitch::SetUpSensor(sensor oSensor)
{
    _sensor.droneID = oSensor.droneID;
    _sensor.type = oSensor.type;
    _sensor.pin = oSensor.pin;
    _sensor.pin2 = oSensor.pin2;
    _sensor.buzzerNotificationID = oSensor.buzzerNotificationID;
    _sensor.rgbNotificationID = oSensor.rgbNotificationID;
    _sensor.notificationElapsedTime = oSensor.notificationElapsedTime;
    _sensor.fieldID = oSensor.fieldID;
    //_sensor.fieldName = oSensor.fieldName;
    _sensor.mode = oSensor.mode;
    _sensor.workerID = oSensor.workerID;
    _sensor.workerElapsedTime = oSensor.workerElapsedTime;
    _sensor.notificationFieldID = oSensor.notificationFieldID;

    pinMode(_sensor.pin, INPUT_PULLUP);

    _sensor_event.timestamp = millis();
    _sensor_event.lastChange = _sensor_event.timestamp;
    _sensor_event.value = 0;
    _sensor_event.lastValue = _sensor_event.value;
    _sensor_event.changed = false;

    _sensor_event.acumulatedNotification = 0;

}

void DroneSwitch::GetSensor(sensor *oSensor)
{
    /* Clear the event */
    memset(oSensor, 0, sizeof(sensor));

    oSensor->droneID = _sensor.droneID;
    oSensor->type = _sensor.type;
    oSensor->pin = _sensor.pin;
    oSensor->pin2 = _sensor.pin2;
    oSensor->buzzerNotificationID = _sensor.buzzerNotificationID;
    oSensor->rgbNotificationID = _sensor.rgbNotificationID;
    oSensor->notificationElapsedTime = _sensor.notificationElapsedTime;
    oSensor->fieldID = _sensor.fieldID;
    //oSensor->fieldName = _sensor.fieldName;
    oSensor->mode = _sensor.mode;
    oSensor->workerID = _sensor.workerID;
    oSensor->workerElapsedTime = _sensor.workerElapsedTime;
    oSensor->notificationFieldID = _sensor.notificationFieldID;

}

void DroneSwitch::GetEvent(sensor_event *oEvent)
{
    /* Clear the event */
    memset(oEvent, 0, sizeof(sensor_event));

    //Read the sensor to get Value and Times
    read();

    //Check for acumulated
    if (wasActive())
        _sensor_event.acumulatedValue = _sensor_event.acumulatedValue + 1;

    //TODO:
    //Check notification
    if(activeFor(_sensor.notificationElapsedTime))
    {
      _sensor_event.triggerNotification = true;

      if (!_sensor_event.changed)
        _sensor_event.acumulatedNotification = 1;
      else
        _sensor_event.acumulatedNotification = _sensor_event.acumulatedNotification + 1;

    }

    else
        _sensor_event.triggerNotification = false;

    //Check worker
    if(_sensor.workerID != -1 && activeFor(_sensor.workerElapsedTime))
        _sensor_event.triggerWorker = true;
    else
        _sensor_event.triggerWorker = false;

    //Copy event
    /* Copy object to return */
    oEvent->value = _sensor_event.value;
    oEvent->lastValue = _sensor_event.lastValue;
    oEvent->acumulatedValue = _sensor_event.acumulatedValue;
    oEvent->changed = _sensor_event.changed;
    oEvent->timestamp = _sensor_event.timestamp;
    oEvent->lastChange = _sensor_event.lastChange;
    oEvent->triggerNotification = _sensor_event.triggerNotification;
    oEvent->triggerWorker = _sensor_event.triggerWorker;
    oEvent->acumulatedNotification = _sensor_event.acumulatedNotification;

    //Serial.println("_Event value: " + String(_sensor_event.value));
    //Serial.println("_Event acumulated: " + String(_sensor_event.acumulatedValue));


}

float DroneSwitch::read()
{
    static int ms;
    static int pinVal;

    ms = millis();
    pinVal = digitalRead(_sensor.pin);

    //Serial.println("PINVal: " + String(pinVal));

    if(ms-_sensor_event.lastChange < _dbTime) //if TRUE is en Debounce time
    {
        _sensor_event.timestamp = ms;
        _sensor_event.changed = false;

        return _sensor_event.value; //Return value (is en Debounce time)
    }
    else //is FALSE -> read value to return
    {
        _sensor_event.lastValue = _sensor_event.value;
        _sensor_event.value = !pinVal;
        _sensor_event.timestamp = ms;

        if(_sensor_event.lastValue != _sensor_event.value)
        {
            _sensor_event.lastChange = ms;
            _sensor_event.changed = true;
        }
        else
        {
            _sensor_event.changed = false;
        }

        //Serial.println("Value: " + String(_sensor_event.value));

        return _sensor_event.value;
    }

}

int DroneSwitch::wasActive(void)
{
    return _sensor_event.value == 1 && _sensor_event.changed;
}

int DroneSwitch::activeFor(int ms)
{
    return (_sensor_event.value == 1 && _sensor_event.timestamp - _sensor_event.lastChange >= ms) ? 1 : 0;
}

void DroneSwitch::Publish(sensor_event *oEvent)
{
    //TODO: CHECK!!!
    if(_sensor.mode == SENSOR_MODE_PULSE)
        oEvent->value = _sensor_event.acumulatedValue;
    else
        oEvent->value = _sensor_event.value;

    Serial.println("Notification 3: " + String(_sensor_event.acumulatedNotification));

    oEvent->acumulatedNotification = _sensor_event.acumulatedNotification;

    Serial.println("Notification 4: " + String(oEvent->acumulatedNotification));

    //Clean acumulatedValue - Only if is not Active
    if(_sensor_event.value == 0)
        _sensor_event.acumulatedValue = 0;

    //Clean acumulatedNotification
    _sensor_event.acumulatedNotification = 0;
}
