#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern TaskHandle_t hUplink;
void vTaskUplink(void *pvParameters);
