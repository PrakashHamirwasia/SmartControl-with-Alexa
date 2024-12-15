// Uncomment the following line to enable serial debug output
// #define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
  #define DEBUG_ESP_PORT Serial
  #define NODEBUG_WEBSOCKETS
  #define NDEBUG
#endif

#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
  #include <WiFi.h>
#endif

#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <SinricProTemperaturesensor.h>

#include "DHT.h" // https://github.com/markruys/arduino-DHT
#include "credentials.h"
#include "fauxmoESP.h"

// -----------------------------------------------------------------------------
// Constants
#define BAUD_RATE         115200 // Change baudrate as per need
#define EVENT_WAIT_TIME   20000  // Default event wait time
#define PUMP_PIN        27
#define bulbPin         25     // GPIO pin for SMART bulbPin
#define bulbDeviceId          "Smart_bulb" // FauxMo device name
#define dhtPin           32     // DHT sensor pin for ESP32            
#define serialBaudRate   115200 // FauxMo baud rate

// Soil Sensor Variables (unchanged)
const int VERY_DRY = 3000; 
const int VERY_WET = 1200; 
const int NEITHER_DRY_OR_WET = 2000; 
const int DRY_PUSH_NOTIFICATION_THRESHHOLD = 3080; 
const int UNPLUGGED = 735;
const int soilMoistureSensorPin = 34;
int soil1 = 0;

int lastSoilMoisture = 0;
String lastSoilMoistureStr = "";


CapacitiveSoilMoistureSensor& soilMoistureSensor= SinricPro[SMS_ID];

// -----------------------------------------------------------------------------
// Setup Functions

// Setup WiFi connection
void wifiSetup() {
  Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

// Sinric Pro setup for Switch, Temperature Sensor, and Soil Sensor
void setupSinricPro() {
  SinricPro.onConnected([]() { Serial.printf("[SinricPro] Connected\r\n"); });
  SinricPro.onDisconnected([]() { Serial.printf("[SinricPro] Disconnected\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);

  // Setup Switch device
  SinricProSwitch& pumpSwitch = SinricPro[pump_device];
  pumpSwitch.onPowerState([](const String& deviceId, bool& state) {
    digitalWrite(PUMP_PIN, state ? LOW : HIGH);
    return true;
  });

  // Setup Temperature Sensor
  SinricProTemperaturesensor& tempHumiditySensor = SinricPro[TEMP_SENSOR_ID];

  // Restore device states (if required)
  // SinricPro.restoreDeviceStates(true);
}

// FauxMo setup for Alexa
fauxmoESP fauxmoServer;

void setupFauxMo() {
  fauxmoServer.createServer(true); // Create a web server for discovery
  fauxmoServer.setPort(80);        // Use port 80 for Gen3 devices
  fauxmoServer.enable(true);       // Enable FauxMo

  fauxmoServer.addDevice(bulbDeviceId);

  fauxmoServer.onSetState([](unsigned char device_id, const char* device_name, bool state, unsigned char value) {
    Serial.printf("[FAUXMO] Device '%s' state: %s value: %d\n", device_name, state ? "ON" : "OFF", value);
    if (strcmp(device_name, bulbDeviceId) == 0) {
      digitalWrite(bulbPin, state ? LOW : HIGH);
    }
  });
}

// Setup DHT sensor
DHT dht;

// -----------------------------------------------------------------------------
// Sensor Handlers

// Add this global variable


void handleCapacitiveSoilMoistureSensor() {
  if (!SinricPro.isConnected()) return;

  static unsigned long last_millis;
  unsigned long current_millis = millis();
  if (last_millis && current_millis - last_millis < EVENT_WAIT_TIME) return;
  last_millis = current_millis;

  int soilMoisture = analogRead(soilMoistureSensorPin);
  soil1 = map(soilMoisture, VERY_WET, VERY_DRY, 100, 0); // Map the sensor reading to percentage
  soil1 = constrain(soil1, 0, 100); // Constrain the value between 0 and 100

  Serial.printf("Soil Moisture: %d, as a percentage: %d%%\r\n", soilMoisture, soil1);

  if (isnan(soilMoisture)) {
    Serial.printf("Capacitive Soil Moisture Sensor reading failed!\r\n");
    return;
  }
  
  if (soil1 < 40) { // Check if the soil moisture percentage is less than 40
    Serial.println("Soil is dry. Turning on the pump...");
    
    // Turn on the pump using SinricPro
    digitalWrite(PUMP_PIN,LOW);
    delay(60000);
    digitalWrite(PUMP_PIN,HIGH);

  } else {
    Serial.println("Soil moisture is sufficient. Pump is off.");
  }

  delay(500);
  soilMoistureSensor.sendRangeValueEvent("rangeInstance1", soil1); // Send the soil moisture percentage to SinricPro

  

  lastSoilMoisture = soilMoisture;
}


void handleTemperaturesensor() {
  if (!SinricPro.isConnected()) return;

  static unsigned long last_millis;
  unsigned long current_millis = millis();
  if (last_millis && current_millis - last_millis < EVENT_WAIT_TIME) return;
  last_millis = current_millis;

  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.printf("DHT reading failed!\r\n");
    return;
  }

  SinricProTemperaturesensor& tempHumiditySensor = SinricPro[TEMP_SENSOR_ID];
  tempHumiditySensor.sendTemperatureEvent(temperature, humidity);
}

// -----------------------------------------------------------------------------
// Main Setup and Loop

void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(bulbPin, OUTPUT);
  digitalWrite(PUMP_PIN, HIGH);
  digitalWrite(bulbPin, HIGH);

  dht.setup(dhtPin);
  wifiSetup();
  setupSinricPro();
  setupFauxMo();
}

void loop() {
  SinricPro.handle();
  fauxmoServer.handle();
  handleCapacitiveSoilMoistureSensor();
  handleTemperaturesensor();

  
}
