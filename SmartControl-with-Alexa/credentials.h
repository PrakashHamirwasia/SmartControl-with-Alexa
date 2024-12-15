#ifndef _CAPACITIVESOILMOISTURESENSOR_H_
#define _CAPACITIVESOILMOISTURESENSOR_H_

#include <SinricProDevice.h>
#include <Capabilities/ModeController.h>
#include <Capabilities/RangeController.h>
#include <Capabilities/PushNotification.h>

#define APP_KEY    "" //Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET "" //Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#define SMS_ID  "" //SoilMSesnsorID"
#define TEMP_SENSOR_ID    "" //TempSensorID
#define pump_device  ""     // WaterPump device "devide id of waterpump created in sinric pro" 

#define WIFI_SSID       "" // Your WiFi SSID
#define WIFI_PASS       "" // Your WiFi Password

class CapacitiveSoilMoistureSensor 
: public SinricProDevice
, public ModeController<CapacitiveSoilMoistureSensor>
, public RangeController<CapacitiveSoilMoistureSensor>
, public PushNotification<CapacitiveSoilMoistureSensor> {
  friend class ModeController<CapacitiveSoilMoistureSensor>;
  friend class RangeController<CapacitiveSoilMoistureSensor>;
  friend class PushNotification<CapacitiveSoilMoistureSensor>;
public:
  CapacitiveSoilMoistureSensor(const String &deviceId) : SinricProDevice(deviceId, "CapacitiveSoilMoistureSensor") {};
};

#endif