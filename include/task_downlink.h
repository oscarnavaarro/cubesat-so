#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern TaskHandle_t hDownlink;
void vTaskDownlink(void *pvParameters);
