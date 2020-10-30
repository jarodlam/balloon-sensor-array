/**
 * serialcamera.ino
 * Test ESP-32 JPEG image parameters over serial
 * 
 * Jarod Lam 2020
 */

#include "esp_camera.h"

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

#define NUM_FRAMESIZES 7
#define NUM_QUALITIES 2

// Camera stuff
camera_config_t config;
framesize_t frameSizes[] = {FRAMESIZE_QVGA, FRAMESIZE_CIF,
  FRAMESIZE_VGA, FRAMESIZE_SVGA, FRAMESIZE_XGA, 
  FRAMESIZE_SXGA, FRAMESIZE_UXGA};
int qualities[] = {10, 12};

// Serial settings
#define SERIAL_START_STRING "<"
#define SERIAL_END_STRING ">"
#define SERIAL_DELIMITER ":"
#define IMAGE_START_STRING "START JPEG IMAGE"
#define IMAGE_END_STRING "END JPEG IMAGE"

// Send sensor value over serial using standard format <Key:Value>
void sendSensorValue(const char* key, float value) {
  Serial.print(SERIAL_START_STRING);
  Serial.print(key);
  Serial.print(SERIAL_DELIMITER);
  Serial.print(value);
  Serial.println(SERIAL_END_STRING);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

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
  config.frame_size = FRAMESIZE_UXGA; 
  config.jpeg_quality = 0;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  esp_camera_deinit();
  
  digitalWrite(PWDN_GPIO_NUM, LOW);
  delay(50);
  digitalWrite(PWDN_GPIO_NUM, HIGH);
  delay(50);

}

void loop() {

  for (int q = 0; q < NUM_QUALITIES; q++) {
    for (int f = 0; f < NUM_FRAMESIZES; f++) {
      // Parameters to test
      config.frame_size = frameSizes[f]; 
      config.jpeg_quality = qualities[q];

      sendSensorValue("FrameSize", frameSizes[f]);
      sendSensorValue("Quality", qualities[q]);

      // Init camera
      esp_err_t err = esp_camera_init(&config);
      if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
      }

      // Take photo
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

      // Cleanup
      esp_camera_fb_return(fb);
      esp_camera_deinit();
      
      digitalWrite(PWDN_GPIO_NUM, LOW);
      delay(50);
      digitalWrite(PWDN_GPIO_NUM, HIGH);
      delay(50);
    }
  }
  
  delay(1000);
}
