
#include "ISRs.hpp"
#include "../globals.hpp"

void CAN_TX_Task(void *pvParameters)
{
  const TickType_t xFrequency = 60 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint8_t msgOut[8];
  while (1)
  {
    // xQueueReceive(msgOutQ, msgOut, portMAX_DELAY);
    // xSemaphoreTake(CAN_TX_Semaphore, portMAX_DELAY);
    // CAN_TX(0x123, msgOut);
    break;
  }
}