/**
 * sensorarray.ino
 * Firmware for balloon/glider hybrid ESP32-CAM sensor array
 * Requires Adafruit Unified Sensor, MPU6050, DHT, and BME280 libraries
 * 
 * Jarod Lam 2020
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>
#include <DHT_U.h>

// Sleep interval in microseconds
const uint32_t SLEEP_DURATION = 1 * 1000000;

// I2C settings
#define I2C_SDA_PIN 15
#define I2C_SCL_PIN 13

// DHT22 settings
#define DHTPIN 14
#define DHTTYPE DHT22

// MPU6050 settings

// BME280 settings
#define SEALEVELPRESSURE_HPA (1013.25)

// Create sensor objects
DHT_Unified dht(DHTPIN, DHTTYPE);
Adafruit_MPU6050 mpu;
Adafruit_BME280 bme;

// Light sleep function
void lightSleep(long duration) {
    esp_sleep_enable_timer_wakeup(duration);
    esp_light_sleep_start();
}

// Calculate vector magnitude
float abs3(float x, float y, float z) {
  return sqrt(x*x + y*y + z*z);
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
  bool bmeStatus = bme.begin(BME280_ADDRESS, &Wire);
  if (!bmeStatus) {
    Serial.println("Failed to find BME280!");
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
  Serial.print("DHT: ");
  Serial.print(dhtT.temperature);
  Serial.print("°C, ");
  Serial.print(dhtT.relative_humidity);
  Serial.print("%");
  Serial.println();

  Serial.print("MPU6050: ");
  Serial.print(mpuT.temperature);
  Serial.print("°C, ");
  Serial.print(abs3(mpuA.acceleration.x, mpuA.acceleration.y, mpuA.acceleration.z));
  Serial.print("m/s^2, ");
  Serial.print(abs3(mpuG.gyro.x, mpuG.gyro.z, mpuG.gyro.z));
  Serial.print("rad/s");
  Serial.println();

  Serial.println();
  
  // Wait for serial to finish, then sleep
  delay(100);
  lightSleep(SLEEP_DURATION);
}
