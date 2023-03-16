
#include "tasks.hpp"
#include "../globals.hpp"
#include "../utils/utils.hpp"

extern knob *knob3;
extern knob *knob2;
extern knob *knob1;

void transmitTask(void *pvParameters)
{

  const TickType_t xFrequency = 25 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint8_t localKeyArray[5];

  uint32_t ID;
  bool keyPressedPrev = false;

  uint8_t tempLocalOctave = 4;

  uint32_t localCurrentStepSizes[12] = {0};
  uint32_t prevReverb = 2;

  while (1)
  {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    uint32_t localCurrentStepSizes[12] = {0};

    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

    westDetect = (((uint32_t)keyArray[5]) % 16) >> 3;
    eastDetect = (((uint32_t)keyArray[6]) % 16) >> 3;
    
    xSemaphoreGive(keyArrayMutex);

    for (uint8_t rowIdx = 0; rowIdx < 5; rowIdx++)
    {
      setRow(rowIdx);

      delayMicroseconds(3);

      localKeyArray[rowIdx] = readCols();
    }

    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

    std::copy(std::begin(localKeyArray), std::end(localKeyArray), std::begin(keyArray));

    xSemaphoreGive(keyArrayMutex);

    uint8_t localKnob2Rotation = __atomic_load_n(&knob2Rotation, __ATOMIC_RELAXED);
    uint8_t localReverb = __atomic_load_n(&reverb, __ATOMIC_RELAXED);
    uint8_t localMode = __atomic_load_n(&mode, __ATOMIC_RELAXED);


    uint32_t tmpCurrentStepSize = 0;
    uint32_t stepScaling = freqs[knob2Rotation];
    
    bool key_pressed = false;

    TX_Message[0] = 0; // Indicating it is a key change message
    for (int i = 0; i < 3; i++)
    {

      xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

      uint8_t keyGroup = keyArray[i];

      xSemaphoreGive(keyArrayMutex);
      uint8_t key4 = (keyGroup % 16) >> 3;
      uint8_t key3 = (keyGroup % 8) >> 2;
      uint8_t key2 = (keyGroup % 4) >> 1;
      uint8_t key1 = (keyGroup % 2);
      uint8_t keyOffset = i * 4;
      TX_Message[i + 1] = 0;
      if (!key1)
      {
        localCurrentStepSizes[keyOffset] = localMode ? 1: stepSizes[keyOffset] << (localOctave - 4 + localKnob2Rotation);
        key_pressed = true;
        TX_Message[i + 1] += 1;
      }
      if (!key2 )
      {
        localCurrentStepSizes[keyOffset + 1] = localMode ? 1: stepSizes[keyOffset + 1] << (localOctave - 4 + localKnob2Rotation); 
        key_pressed = true;
        TX_Message[i + 1] += 2;
      }
      if (!key3)
      {
        localCurrentStepSizes[keyOffset + 2] = localMode ? 1: stepSizes[keyOffset + 2] << (localOctave - 4 + localKnob2Rotation);  
        key_pressed = true;
        TX_Message[i + 1] += 4;
      }
      if (!key4)
      {
        localCurrentStepSizes[keyOffset + 3] = localMode ? 1: stepSizes[keyOffset + 3] << (localOctave - 4 + localKnob2Rotation); 
        key_pressed = true;
        TX_Message[i + 1] += 8;
      }
    };

    // Need to set the local octave as well. This would mean that the local step size would also have to be 
    // altered accordingly, preferably on-the-fly
    if ((westDetect == 0) && (eastDetect == 0)) // Centre
    {
      TX_Message[4] = 5;
      tempLocalOctave = 5;
    }
    else if ((westDetect == 0) && (eastDetect == 1)) // Right
    {
      TX_Message[4] = 5;
      tempLocalOctave = 5;
    }
    else if ((westDetect == 1) && (eastDetect == 0)) // Left
    {
      TX_Message[4] = 4;
      tempLocalOctave = 4;
    }
    else 
    {
      TX_Message[4] = 4;
      tempLocalOctave = 4;
    }
    __atomic_store_n(&localOctave, tempLocalOctave, __ATOMIC_RELAXED);
    
    xSemaphoreTake(stepSizesMutex, portMAX_DELAY);

    if (localMode)
    {
      for (uint8_t i = 0; i < 12; i++){ 
        currentStepSizes[i] = localCurrentStepSizes[i] != 0;
      } 
    }
    else{
      for (uint8_t i = 0; i < 12; i++){ 
        uint8_t idx = (12*(tempLocalOctave - 4)) + i;
        currentStepSizes[idx] = localCurrentStepSizes[i]; 
        if (localCurrentStepSizes[i] != 0){
        prevStepSizes[idx] = localReverb ? localCurrentStepSizes[i] : 0;  
        decayCounters[i] = 0; 
        internalCounters[i] = 0;
      }
      }
    }

    xSemaphoreGive(stepSizesMutex);

    // if ((keyChanged || localKnob2Rotation != prevKnob2Rotation || localReverb != prevReverb) && !(westDetect == 1 && eastDetect == 1))
    if ((key_pressed || (key_pressed != keyPressedPrev)) && !(westDetect == 1 && eastDetect == 1))
    {
      // TX_Message[0] = 80;
      xQueueSend(msgOutQ, (const void *)TX_Message, portMAX_DELAY);
    }

    keyPressedPrev = key_pressed; 
    prevReverb = reverb;
 
    // For reverb knob
    knob1->updateRotation(reverb);
    
    // For pitch knob
    knob2->updateRotation(knob2Rotation); 

    // For volume knob
    knob3->updateRotation(knob3Rotation);

    // For mode
    knob0->updateRotation(mode);


  }
}
