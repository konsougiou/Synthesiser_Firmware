
#include "tasks.hpp"
#include "../globals.hpp"

void modeSwitchTask(void *pvParameters){
  const TickType_t xFrequency = 50 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  uint8_t localMode;
  uint8_t prevLocalMode;

 uint8_t prevMode = 0;

  while (1)
  {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    // Sets the TX message field to store the reverb that was set locally
    localMode = __atomic_load_n(&mode, __ATOMIC_RELAXED); 

    if(prevMode != localMode){
        if (localMode == 1){
            sampleTimer->pause();

            // Emptying the array of the previous step sizes in order to avoid the sampleISR
            // intermpreting them as reverb in the next time mode 0 (sawtooth wave) is entered.
            xSemaphoreTake(stepSizesMutex, portMAX_DELAY);
            std::fill(std::begin(prevStepSizes), std::end(prevStepSizes), 0);
            xSemaphoreGive(stepSizesMutex);

            delayMicroseconds(3);

            sinewaveSampleTimer->resume();
        }
        else{

            sinewaveSampleTimer->pause();

            delayMicroseconds(3);

            sampleTimer->resume();
        }
    }
    prevMode = localMode;
  }
}