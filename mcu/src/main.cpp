#include <Arduino.h>
#include <WiFiMulti.h>
#include "wifi_ssid.h"
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

WiFiMulti wifi_multi;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(3000);

  wifi_multi.addAP(wifi1_ssid, wifi1_password);
  wifi_multi.addAP(wifi2_ssid, wifi2_password);

  Serial.print("Connecting to WiFi");
  while (wifi_multi.run() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  init_camera();

  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  delay(3000);
}
