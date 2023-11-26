#include "header.h"
#include "firebasefunctions.cpp"
#include "sensorfunction.cpp"

void wifiSetup(){
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.mode(WIFI_STA);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void taskDataSerial(void* parameter) {
  while (1) {
    if (xSemaphoreTake(cSemaphore, portMAX_DELAY)) {
      getReadings();
      xSemaphoreGive(cSemaphore);
    }
    else {
      Serial.println("Failed to take semaphore");
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void taskSegment(void* parameter) {
  while (1) {
    if (xSemaphoreTake(cSemaphore, portMAX_DELAY)) {
      Seven_Segment();
      xSemaphoreGive(cSemaphore);
    }
    else {
      Serial.println("Failed to take semaphore");
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void taskServo(void* parameter) {
  while (1) {
    if (xSemaphoreTake(cSemaphore, portMAX_DELAY)) {
      Open_Bin();
      xSemaphoreGive(cSemaphore);
    }
    else {
      Serial.println("Failed to take semaphore");
    }
  }
}

void taskPowerSwitch(void* parameter) {
  while (1) {
    if (xSemaphoreTake(cSemaphore, portMAX_DELAY)) {
      powerSwitching();
      xSemaphoreGive(cSemaphore);
    }
    else {
      Serial.println("Failed to take semaphore");
    }
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

void taskFirebase(void* parameter) {
  while (1) {
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      sendFirebase();
      xSemaphoreGive(mutex);
    }
    else {
      Serial.println("Failed to take mutex");
    }
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

void setup() {
  Serial.begin(115200);

  // Relay
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);

  // UltraSonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // IR
  pinMode(IR_PIN, INPUT);

  // Servo
  servo.attach(SERVO_PIN);
  servo.write(0); // Close the bin

  // HX711
  Serial.println("Initializing Scale....");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calib);
  scale.tare();
  Serial.println("Scale Initialized");
  // 7 Segment
  Serial.println("Initializing 7 Segment....");
  disp.Initialize(15);
  disp.Clear();
  Serial.println("7 Segment Initialized");

  // GPS
  ss.begin(GPS_BAUD);

  wifiSetup();
  
  // NTP
  timeClient.begin();
  timeClient.update();
  timeClient.setTimeOffset(25200);

  firebaseSetup();
  delay(15000); // Untuk nunggu token dari firebase

  xTaskCreate(taskFirebase, "Firebase", configMINIMAL_STACK_SIZE + 20480, NULL, 0, NULL);
  xTaskCreate(taskDataSerial, "Serial", configMINIMAL_STACK_SIZE + 4096, NULL, 0, NULL);
  xTaskCreate(taskPowerSwitch, "Power", configMINIMAL_STACK_SIZE + 2048, NULL, 0, NULL);
  xTaskCreate(taskSegment, "Segment", configMINIMAL_STACK_SIZE + 2048, NULL, 0, NULL);
  xTaskCreate(taskServo, "Servo", configMINIMAL_STACK_SIZE + 2048, NULL, 0, NULL);
}

void loop() {
  
}
