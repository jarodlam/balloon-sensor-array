/**
 * sensorarray.ino
 * Firmware for balloon/glider hybrid ESP32-CAM sensor array
 * Requires Adafruit Unified Sensor, MPU6050, DHT, and BME280 libraries
 * 
 * Jarod Lam 2020
 */

#include <Wire.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_BME280.h"
#include "Adafruit_MPU6050.h"
#include "dht.h"

#include "esp_camera.h"
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// Sleep interval in microseconds
#define SLEEP_DURATION 1 * 1000000

// I2C settings
#define I2C_SDA_PIN 15
#define I2C_SCL_PIN 13

// DHT22 settings
#define DHTPIN 14
#define DHTTYPE DHT22

// MPU6050 settings

// BME280 settings
#define SEALEVELPRESSURE_HPA (1013.25)

// Serial settings
#define SERIAL_START_STRING "<"
#define SERIAL_END_STRING ">"
#define SERIAL_DELIMITER ":"
#define IMAGE_START_STRING "START JPEG IMAGE"
#define IMAGE_END_STRING "END JPEG IMAGE"

// Camera settings
#define CAMERA_EVERY_NUM_LOOPS 10

// Create sensor objects
dht DHT;
Adafruit_MPU6050 mpu;
Adafruit_BME280 bme;

// Create loop counter for camera
int loopCounter = 0;

// Puts ESP32 into low-power mode for duration microseconds
void lightSleep(long duration) {
  esp_sleep_enable_timer_wakeup(duration);
  esp_light_sleep_start();
}

// Send sensor value over serial using standard format <Key:Value>
void sendSensorValue(const char* key, float value) {
  Serial.print(SERIAL_START_STRING);
  Serial.print(key);
  Serial.print(SERIAL_DELIMITER);
  Serial.print(value);
  Serial.println(SERIAL_END_STRING);
}

void setup() {
  // Init serial
  Serial.begin(115200);
  while(!Serial);

  // Init I2C
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // Init MPU6050
  bool mpuStatus = mpu.begin(MPU6050_I2CADDR_DEFAULT, &Wire);
  if (!mpuStatus) {
    Serial.println("Failed to find MPU6050!");
  }

  // Init BME280
  //bool bmeStatus = bme.begin(BME280_ADDRESS, &Wire);
  bool bmeStatus = bme.begin(0x76, &Wire);
  if (!bmeStatus) {
    Serial.println("Failed to find BME280!");
  }

  // Init camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA; // QVGA,CIF,VGA,SVGA,XGA,SXGA,UXGA
  config.jpeg_quality = 10;          // 10,12
  config.fb_count = 2;
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  // Wait for serial to finish
  delay(100);
}

void loop() {
  // Get DHT22 readings
  int dhtChk = DHT.read22(DHTPIN);
  
  // Get MPU6050 readings
  sensors_event_t mpuA, mpuG, mpuT;
  mpu.getEvent(&mpuA, &mpuG, &mpuT);

  // Get BME280 readings
  sensors_event_t bmeT, bmeP, bmeH;
  bme.getTemperatureSensor()->getEvent(&bmeT);
  bme.getPressureSensor()->getEvent(&bmeP);
  bme.getHumiditySensor()->getEvent(&bmeH);

  // Print readings
  if (dhtChk == DHTLIB_OK) {
    sendSensorValue("Temp1", DHT.temperature);
  }
  sendSensorValue("Temp2", bmeT.temperature);
  sendSensorValue("Temp3", mpuT.temperature);
  
  if (dhtChk == DHTLIB_OK) {
    sendSensorValue("Temp1", DHT.humidity);
  }
  sendSensorValue("Hum2", bmeH.relative_humidity);

  sendSensorValue("AccelX", mpuA.acceleration.x);
  sendSensorValue("AccelY", mpuA.acceleration.y);
  sendSensorValue("AccelZ", mpuA.acceleration.z);

  sendSensorValue("GyroX", mpuG.gyro.x);
  sendSensorValue("GyroY", mpuG.gyro.y);
  sendSensorValue("GyroZ", mpuG.gyro.z);

  // Take picture every n loops
  loopCounter++;
  if (loopCounter > CAMERA_EVERY_NUM_LOOPS) {
    loopCounter = 0;

    // Get image from framebuffer
    camera_fb_t *fb = esp_camera_fb_get();
    const char *fbData = (const char *)fb->buf;
    size_t fbLen = fb->len;

    // Send image over serial
    sendSensorValue("ImgBytes", fbLen);
    Serial.println(IMAGE_START_STRING);
    for (int i = 0; i < fbLen; i++) {
      Serial.print(fbData[i]);
    }
    Serial.println(IMAGE_END_STRING);
    Serial.println();

    esp_camera_fb_return(fb);
  }
  
  // Wait for serial to finish, then sleep
  delay(1000);
  //lightSleep(SLEEP_DURATION);
}
