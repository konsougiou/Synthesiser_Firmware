
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
        if (localMode == 2){ // Sine wave
            // We pause the timers for both sawtooth and sine to let triangle resume alone
            sawtoothwaveSampleTimer->pause();

            delayMicroseconds(3);

            sinewaveSampleTimer->pause();
            // Loading the array of the previous step sizes with zeros in order to avoid the next ISR that is resumed from
            // intermpreting them as reverb 
            xSemaphoreTake(stepSizesMutex, portMAX_DELAY);
            std::fill(std::begin(prevStepSizes), std::end(prevStepSizes), 0);
            xSemaphoreGive(stepSizesMutex);

            delayMicroseconds(3);

            trianglewaveSampleTimer->resume();
        } 
        else if (localMode == 0){ // Sawtooth wave
            
            trianglewaveSampleTimer->pause();
            
            delayMicroseconds(3);

            sinewaveSampleTimer->pause();

            xSemaphoreTake(stepSizesMutex, portMAX_DELAY);
            std::fill(std::begin(prevStepSizes), std::end(prevStepSizes), 0);
            xSemaphoreGive(stepSizesMutex);

            delayMicroseconds(3);

            sawtoothwaveSampleTimer->resume();
        }
        else{

            trianglewaveSampleTimer->pause();
            
            delayMicroseconds(3);

            sawtoothwaveSampleTimer->pause();           

            delayMicroseconds(3);
            //sinewaveISR doesn't use the previous step sizes so no need to empty the array
            sinewaveSampleTimer->resume();
        }
    }
    prevMode = localMode;
    break;
  }
}