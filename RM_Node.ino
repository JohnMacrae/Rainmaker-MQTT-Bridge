void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx) {
  const char *device_name = device->getDeviceName();
  const char *param_name = param->getParamName();
  Serial.printf("Received value = %d for %s - param: %s\n", val.val.i, device_name, param_name);

  if ((strcmp(param_name, "Power") == 0) && (strcmp(device_name, "FrontLight") == 0)) {
    Serial.printf("Received value = %d for %s - %s\n", val.val.b, device_name, param_name);
    bool state = val.val.b;
    if (state) {
      mqttClient.publish("cmnd/demolights/POWER", 1, true, "1");
      Serial.println("Sent ON");
    } else {
      mqttClient.publish("cmnd/demolights/POWER", 1, true, "0");
      Serial.println("Sent OFF");
    }
    param->updateAndReport(val);
  }

  else if ((strcmp(param_name, "Battery") == 0) && (strcmp(device_name, "Solar") == 0)) {
    Serial.printf("Received value = %f for %s - %s\n", val.val.f, device_name, param_name);
    float  state = val.val.f;
    param->updateAndReport(val);
  }
}


void node_configure() {
  /*
      Create the Node
  */
  Node my_node;
  my_node = RMaker.initNode("Home");

  front_lights.addNameParam();
  front_lights.addPowerParam(DEFAULT_POWER_MODE);
  front_lights.assignPrimaryParam(front_lights.getParamByName(ESP_RMAKER_DEF_POWER_NAME));
  front_lights.addCb(write_callback);
  my_node.addDevice(front_lights);

  Solar.addNameParam();
  Solar.addTempratureParam(Bcharge, "Battery");
  Solar.assignPrimaryParam(Solar.getParamByName("Battery"));
  my_node.addDevice(Solar);

  /*
    Provisioning via BLE or WiFI
  */
  WiFi.onEvent(sysProvEvent);
#if CONFIG_IDF_TARGET_ESP32
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name);
#else
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name);
#endif
}
/*
   Say how you want provisioning done
*/
void sysProvEvent(arduino_event_t *sys_event) {
  switch (sys_event->event_id) {
    case ARDUINO_EVENT_PROV_START:
#if CONFIG_IDF_TARGET_ESP32
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on BLE\n", service_name, pop);
      printQR(service_name, pop, "ble");
#else
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on SoftAP\n", service_name, pop);
      printQR(service_name, pop, "softap");
#endif
      break;
  }
}
