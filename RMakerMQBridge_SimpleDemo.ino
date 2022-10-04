#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"
#include <WiFi.h>
extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

#include <Wire.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
static int gpio_0 = 0;

void node_configure();
#define DEFAULT_POWER_MODE false

float Bcharge = 0;

//Rainmaker Device Names - Types//
static Device front_lights("FrontLight", ESP_RMAKER_DEVICE_LIGHTBULB , NULL);
static Device Solar("Solar", "esp.device.temperature-sensor", NULL);
/*
   Provisioning Service Name
*/
const char *service_name = "PROV_1234";
const char *pop = "abcd1234";

#include <AsyncMqttClient.h>
#define MQTT_HOST IPAddress(192, 168, 199, 8)
#define MQTT_PORT 1883

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

int strToInt(char* str)
{
  int res = 0;
  if (strcmp( "ON", str) == 0) {
    res = 100;
  }
  return res;
}

float strToFloat(char* str)
{
  return atof(str);
}

bool strToBool(char* str)
{
  bool res = false;
  if (strcmp( "ON", str) == 0) {
    res = true;
  }
  return res;
}
void setup()
{
  Serial.begin(115200);

  node_configure();
  MQ_Setup();

  RMaker.start();
}

void loop()
{

  if (digitalRead(gpio_0) == LOW) { //Push button pressed

    // Key debounce handling
    vTaskDelay(100);
    Serial.printf("Pressed");
    int startTime = millis();
    while (digitalRead(gpio_0) == LOW) delay(50);
    int endTime = millis();

    if ((endTime - startTime) > 10000) {
      // If key pressed for more than 10secs, reset all
      Serial.printf("Reset to factory.\n");
      RMakerFactoryReset(2);
    } else if ((endTime - startTime) > 3000) {
      Serial.printf("Reset Wi - Fi.\n");
      // If key pressed for more than 3secs, but less than 10, reset Wi-Fi
      RMakerWiFiReset(2);
    }
  }
}
