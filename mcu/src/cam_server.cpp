#include <Arduino.h>
#include <esp_http_server.h>
#include <memory>
#include <cstring>
#include "cam_server.h"

#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    21
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27
#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      19
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM       5
#define Y2_GPIO_NUM       4
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22
#define PART_BOUNDARY   "frame"

static const char _STREAM_CONTENT_TYPE[] = "multipart/x-mixed-replace; boundary=" PART_BOUNDARY;
static const char _STREAM_BOUNDARY[] = "--" PART_BOUNDARY "\r\n";
static const char _STREAM_PART[] = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t server;

void init_camera()
{
  Serial.println("Initializing camera device...");

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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QXGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if(psramFound()){
    config.frame_size = FRAMESIZE_QXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

  sensor_t * cam = esp_camera_sensor_get();
  cam->set_framesize(cam, FRAMESIZE_QVGA);

  Serial.println("Camera initialized...");

  Serial.println("Starting camera server...");
  httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
  http_config.server_port = 80;
  //http_config.max_uri_handlers = 16;

  if (httpd_start(&server, &http_config) != ESP_OK) {
    Serial.println("Failed to start camera server!");
    return;
  }

  httpd_uri_t capture_uri = {
    .uri      = "/capture",
    .method   = HTTP_GET,
    .handler  = capture_handler,
    .user_ctx = NULL,
  };

  httpd_uri_t stream_uri = {
    .uri      = "/stream",
    .method   = HTTP_GET,
    .handler  = stream_handler,
    .user_ctx = NULL,
  };

  httpd_register_uri_handler(server, &capture_uri);
  httpd_register_uri_handler(server, &stream_uri);
  Serial.println("Camera server started!");
}

esp_err_t capture_handler(httpd_req_t* req) {
  Serial.println("Capturing image...");

  camera_fb_t* fb = esp_camera_fb_get();
  
  if (!fb) {
    Serial.println("Could not get camera framebuffer!");
    return ESP_FAIL;
  }

  if (fb->format != PIXFORMAT_JPEG) {
    esp_camera_fb_return(fb);
    Serial.println("Wrong framebuffer format. Must be JPEG!");
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");

  //char buf[64];
  //snprintf(buf, 64, "%ld.%06ld", fb->timestamp.tv_sec, fb->timestamp.tv_usec);
  //httpd_resp_set_hdr(req, "X-Timestamp", (const char *)buf);

  uint8_t* img_buf = (uint8_t*)std::malloc(fb->len);
  if (!img_buf) {
    esp_camera_fb_return(fb);
    Serial.println("Failed to allocate temporary buffer");
    return ESP_FAIL;
  }

  std::memcpy(img_buf, fb->buf, fb->len);

  auto err = httpd_resp_send(req, (const char*)img_buf, fb->len);
  if (err != ESP_OK) {
    esp_camera_fb_return(fb);
    Serial.printf("Cannot send JPEG! Code: %d\n", err);
    return ESP_FAIL;
  }

  esp_camera_fb_return(fb);
  Serial.printf("Image captured and sent! (size: %u)\n", fb->len);

  return ESP_OK;
}

esp_err_t stream_handler(httpd_req_t* req) {
  camera_fb_t* fb = nullptr;
  httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  char buf[128];

  while (true) {
    fb = esp_camera_fb_get();

    if (!fb) {
      Serial.println("Could not get camera framebuffer!");
      return ESP_FAIL;
    }

    if (fb->format != PIXFORMAT_JPEG) {
      esp_camera_fb_return(fb);
      Serial.println("Wrong framebuffer format. Must be JPEG!");
      return ESP_FAIL;
    }

    size_t chunk_len = snprintf(buf, 128, _STREAM_PART, fb->len);
    auto err = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    if (err == ESP_OK)
      err = httpd_resp_send_chunk(req, buf, chunk_len);
    if (err == ESP_OK)
      err = httpd_resp_send_chunk(req, (const char*)fb->buf, fb->len);
    if (err == ESP_OK)
      err = httpd_resp_send_chunk(req, "\r\n", strlen("\r\n"));

    if (err != ESP_OK) {
      esp_camera_fb_return(fb);
      Serial.printf("Cannot send JPEG! Code: %d\n", err);
      return ESP_OK;
    }
    
    esp_camera_fb_return(fb);
    delay(42);
  }

  return ESP_OK;
}