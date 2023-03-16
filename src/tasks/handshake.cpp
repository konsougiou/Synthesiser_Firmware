
#include "tasks.hpp"
#include "../globals.hpp"
#include "../utils/utils.hpp"

void handshakeTask(void *pvParameters)
{

  const TickType_t xFrequency = 30 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint8_t ran = 1;

  while (ran==1)
  {

    // vTaskDelayUntil(&xLastWakeTime, xFrequency);

    setRow(5);                                    // Set row address
    digitalWrite(OUT_PIN, 1);                     // Set value to latch in DFF
    digitalWrite(REN_PIN, 1);                     // Enable selected row
    delayMicroseconds(3);                         // Wait for column inputs to stabilise
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    keyArray[5] = readCols();                     // Read column inputs
    xSemaphoreGive(keyArrayMutex);
    digitalWrite(REN_PIN, 0);

    delayMicroseconds(3);

    setRow(6);                                    // Set row address
    digitalWrite(OUT_PIN, 1);                     // Set value to latch in DFF
    digitalWrite(REN_PIN, 1);                     // Enable selected row
    delayMicroseconds(3);                         // Wait for column inputs to stabilise
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    keyArray[6] = readCols();                     // Read column inputs
    xSemaphoreGive(keyArrayMutex);
    digitalWrite(REN_PIN, 0);

    ran = 2;
  }
}