
#include "../globals.hpp"

void decodeTask(void *pvParameters)
{
  const TickType_t xFrequency = 25 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  uint8_t tempReverb;
  uint8_t tempKnob2Rotation;
  uint8_t tempRX_Message[8] = {0};
  uint8_t localKnob2Rotation;

  while (1)
  {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    
    xQueueReceive(msgInQ, tempRX_Message, portMAX_DELAY);

    uint32_t localCurrentStepSizes[12] = {0};

    uint8_t localReverb;

    xSemaphoreTake(queueReceiveMutex, portMAX_DELAY);

    std::copy(std::begin(tempRX_Message), std::end(tempRX_Message), std::begin(RX_Message));

    xSemaphoreGive(queueReceiveMutex);
    
    localKnob2Rotation = __atomic_load_n(&knob2Rotation, __ATOMIC_RELAXED); 

    if (!(westDetect == 1 && eastDetect == 1)){
      if (RX_Message[4] == 0){
      uint32_t RX_Octave = RX_Message[3];
      for(int i = 0; i < 3; i++){
        uint32_t keyGroup = RX_Message[i];
        uint8_t key4 = (keyGroup % 16) >> 3;
        uint8_t key3 = (keyGroup % 8) >> 2;
        uint8_t key2 = (keyGroup % 4) >> 1;
        uint8_t key1 = (keyGroup % 2);
        uint8_t keyOffset = i * 4;
        if (key1)
        {
          localCurrentStepSizes[keyOffset] = stepSizes[keyOffset] << (RX_Octave - 4 + localKnob2Rotation);
        }
        if (key2)
        {
          localCurrentStepSizes[keyOffset + 1] = stepSizes[keyOffset + 1] << (RX_Octave - 4 + localKnob2Rotation);
        }
        if (key3)
        {
          localCurrentStepSizes[keyOffset + 2] = stepSizes[keyOffset + 2] << (RX_Octave - 4 + localKnob2Rotation);
        }
        if (key4)
        {
          localCurrentStepSizes[keyOffset + 3] = stepSizes[keyOffset + 3] << (RX_Octave - 4 + localKnob2Rotation);
        }
      }
        localReverb = __atomic_load_n(&reverb, __ATOMIC_RELAXED);

        xSemaphoreTake(currentStepSizesMutex, portMAX_DELAY);

        for (uint8_t i = 0; i < 12; i++){ 
            uint8_t idx = (12*(RX_Octave - 4)) + i;
            currentStepSizes[idx] = localCurrentStepSizes[i]; 
            if (localCurrentStepSizes[i] != 0){
            prevStepSizes[idx] = localReverb ? localCurrentStepSizes[i] : 0;
            decayCounters[i] = 0; 
            internalCounters[i] = 0;
          }
        }
        
      xSemaphoreGive(currentStepSizesMutex);
      }
      else{
      tempReverb = RX_Message[1];
      __atomic_store_n(&reverb, tempReverb, __ATOMIC_RELAXED);

      tempKnob2Rotation = RX_Message[2];
      __atomic_store_n(&knob2Rotation, tempKnob2Rotation, __ATOMIC_RELAXED);
      } 
    }
  } 
}