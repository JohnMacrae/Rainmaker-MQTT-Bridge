void MQ_Setup() {
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials("myusername", "mypassword");
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      xTimerStop(mqttReconnectTimer, 0);  // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  uint16_t packetIdSub = NULL;
  //MQTT Subscription to the status topics
  packetIdSub = mqttClient.subscribe("stat/demoLights/POWER", 2);
  packetIdSub = mqttClient.subscribe("mon/homebattery", 2);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
}
void onMqttPublish(uint16_t packetId) {
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  // Parse and act on status messages

  if (strcmp(topic, "stat/demolights/POWER") == 0) {
    Serial.println("OutsideLights Matched");
    char msg[50] = "";
    strncpy(msg, payload, len);
    msg[sizeof(payload)] = '\0';
    Serial.printf("msg: %s\n", msg);
    front_lights.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, strToBool(msg));
  }
  else if (strcmp(topic, "mon/homebattery") == 0) {
    Serial.println("Battery Match");
    char msg[50] = "";
    strncpy(msg, payload, len);
    msg[sizeof(payload)] = '\0';
    Serial.printf("msg: %s\n", msg);
    Solar.updateAndReportParam("Battery", strToFloat(msg));
    Bcharge = strToFloat(msg);
  }
}
