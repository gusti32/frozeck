#pragma once

#include <esp_camera.h>
#include <esp_http_server.h>

void init_camera();
esp_err_t capture_handler(httpd_req_t* req);
esp_err_t stream_handler(httpd_req_t* req);