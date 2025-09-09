#include "vision.h"
#include <Arduino.h>

#include "esp_camera.h"

Vision::Vision() {
  // Setup the Camera Module
  // camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; // Always start with JPEG for OV2640
}

Vision::~Vision() {
    if (initialized) {
        esp_camera_deinit();
    }
}

bool Vision::initialize() {
  // Init with high quality if enough PSRAM
  if(psramFound()){
    Serial.println("Found psram!"); // Changed to println for new line
    config.frame_size = FRAMESIZE_SVGA; // Start with a smaller frame size like SVGA or XGA for initial testing
                                         // QVGA might be too small if you plan to scale up.
    config.jpeg_quality = 8; // Lower quality for initial test, closer to 0 is higher quality, 63 is lowest
    config.fb_count = 2;
  } else {
    Serial.println("No psram!"); // Changed to println
    config.frame_size = FRAMESIZE_96X96; // Very small for no PSRAM, confirm this works
    config.jpeg_quality = 10; // Still reasonable for small images
    config.fb_count = 1;
  }

  // Debugging: Print config settings
  Serial.println("Attempting camera init with config:");
  Serial.printf("  XCLK Freq: %d\n", config.xclk_freq_hz);
  Serial.printf("  Pixel Format: %d\n", config.pixel_format);
  Serial.printf("  Frame Size: %d\n", config.frame_size);
  Serial.printf("  JPEG Quality: %d\n", config.jpeg_quality);
  Serial.printf("  FB Count: %d\n", config.fb_count);

  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    // You can look up esp_err_t codes in ESP-IDF documentation for more specific reasons.
    // Common errors:
    // ESP_ERR_CAMERA_NOT_DETECTED (0x20000 + 1)
    // ESP_ERR_CAMERA_NO_MEM (0x20000 + 2)
    // ESP_ERR_CAMERA_TIMEOUT (0x20000 + 3)
    return false; // Exit if camera initialization fails
  }

  
  initialized = true;
  Serial.println("Camera initialized successfully");
  return true;
}

camera_fb_t* Vision::captureFrame() {
    if (!initialized) {
        Serial.println("Camera not initialized!");
        return nullptr;
    }

    Serial.println("Taking photo...");
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed!");
        return nullptr;
    }
    Serial.printf("Captured frame: %d bytes, resolution: %dx%d\n",
                  fb->len, fb->width, fb->height);
    return fb;
}

bool Vision::isInitialized() const {
    return initialized;
}

void Vision::cleanup(camera_fb_t* fb) {
    if (fb) {
        esp_camera_fb_return(fb);
    }
}