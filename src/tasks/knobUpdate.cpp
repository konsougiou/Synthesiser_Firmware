
#include "tasks.hpp"
#include "../globals.hpp"

void knobUpdateTask(void *pvParameters){
  const TickType_t xFrequency = 50 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  uint8_t localReverb;
  uint8_t prevReverb;

  uint8_t localKnob2Rotation;
  uint8_t prevKnob2Rotation;

  uint8_t localMode;
  uint8_t prevMode;
  uint8_t ran = 1;

  while (ran==1)
  {
    // vTaskDelayUntil(&xLastWakeTime, xFrequency);

    // For reverb knob
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

    westDetect = (((uint32_t)keyArray[5]) % 16) >> 3;
    eastDetect = (((uint32_t)keyArray[6]) % 16) >> 3;

    xSemaphoreGive(keyArrayMutex);
    
    // Indicating a knob change message
    TX_Message[0] = 1;
    
    // Sets the TX message field to store the reverb that was set locally
    localReverb = __atomic_load_n(&reverb, __ATOMIC_RELAXED); 
    TX_Message[1] = localReverb;

    // Sets the TX message field to store the pitch that was set locally
    localKnob2Rotation = __atomic_load_n(&knob2Rotation, __ATOMIC_RELAXED); 
    TX_Message[2] = localKnob2Rotation;

    localMode = __atomic_load_n(&mode, __ATOMIC_RELAXED); 
    TX_Message[3] = localMode;

    if ((localKnob2Rotation != prevKnob2Rotation || localReverb != prevReverb || localMode != prevMode) && !(westDetect == 1 && eastDetect == 1))
    {
      xQueueSend(msgOutQ, (const void *)TX_Message, portMAX_DELAY);
    } 

    prevReverb = localReverb;
    prevKnob2Rotation = localKnob2Rotation;
    prevMode = localMode;
    ran=2;
  }
}