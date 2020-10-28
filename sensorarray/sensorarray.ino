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
#include "DHT_U.h"

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

// Camera settings
#define CAMERA_EVERY_NUM_LOOPS 10
int loopCounter = 0;

// Create sensor objects
DHT_Unified dht(DHTPIN, DHTTYPE);
Adafruit_MPU6050 mpu;
Adafruit_BME280 bme;


// Puts ESP32 into low-power mode for duration microseconds
void lightSleep(long duration) {
    esp_sleep_enable_timer_wakeup(duration);
    esp_light_sleep_start();
}

void setup() {
  // Init serial
  Serial.begin(115200);
  while(!Serial);

  // Init I2C
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // Init DHT22
  dht.begin();

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
  sensors_event_t dhtT;
  dht.temperature().getEvent(&dhtT);
  
  // Get MPU6050 readings
  sensors_event_t mpuA, mpuG, mpuT;
  mpu.getEvent(&mpuA, &mpuG, &mpuT);

  // Get BME280 readings
  sensors_event_t bmeT, bmeP, bmeH;
  bme.getTemperatureSensor()->getEvent(&bmeT);
  bme.getPressureSensor()->getEvent(&bmeP);
  bme.getHumiditySensor()->getEvent(&bmeH);

  // Print readings
  Serial.print("Temp1:");
  Serial.println(dhtT.temperature);
  Serial.print("Temp2:");
  Serial.println(bmeT.temperature);
  Serial.print("Temp3:");
  Serial.println(mpuT.temperature);

  Serial.print("Hum1:");
  Serial.println(dhtT.relative_humidity);
  Serial.print("Hum2:");
  Serial.println(bmeH.relative_humidity);

  Serial.print("AccelX:");
  Serial.println(mpuA.acceleration.x);
  Serial.print("AccelY:");
  Serial.println(mpuA.acceleration.y);
  Serial.print("AccelZ:");
  Serial.println(mpuA.acceleration.z);
  
  Serial.print("GyroX:");
  Serial.println(mpuG.gyro.x);
  Serial.print("GyroY:");
  Serial.println(mpuG.gyro.y);
  Serial.print("GyroZ:");
  Serial.println(mpuG.gyro.z);

  // Take picture every n loops
  loopCounter++;
  if (loopCounter > CAMERA_EVERY_NUM_LOOPS) {
    loopCounter = 0;

    // Get image from framebuffer
    camera_fb_t *fb = esp_camera_fb_get();
    const char *fbData = (const char *)fb->buf;
    size_t fbLen = fb->len;

    // Send image over serial
    Serial.print("ImgBytes:");
    Serial.println(fbLen);
    Serial.println("START JPEG IMAGE");
    for (int i = 0; i < fbLen; i++) {
      Serial.print(fbData[i]);
    }
    Serial.println("END JPEG IMAGE");
    Serial.println();

    esp_camera_fb_return(fb);
  }
  
  // Wait for serial to finish, then sleep
  delay(1000);
  //lightSleep(SLEEP_DURATION);
}
