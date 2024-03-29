
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
  bool localMiddleKeyboardFound;

  uint8_t prevWestDetect = 0;
  uint8_t prevEastDetect = 0;

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

    uint8_t localPitch = __atomic_load_n(&pitch, __ATOMIC_RELAXED);
    uint8_t localReverb = __atomic_load_n(&reverb, __ATOMIC_RELAXED);
    uint8_t localMode = __atomic_load_n(&mode, __ATOMIC_RELAXED);

    uint8_t loadedLocalOctave = __atomic_load_n(&localOctave, __ATOMIC_RELAXED);

    uint32_t tmpCurrentStepSize = 0;

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
        localCurrentStepSizes[keyOffset] = localMode == 1 ? (note_frequencies[keyOffset] << (loadedLocalOctave - 4 + localPitch)) : stepSizes[keyOffset] << (loadedLocalOctave - 4 + localPitch);
        key_pressed = true;
        TX_Message[i + 1] += 1;
      }
      if (!key2)
      {
        localCurrentStepSizes[keyOffset + 1] = localMode == 1 ? (note_frequencies[keyOffset + 1] << (loadedLocalOctave - 4 + localPitch)) : stepSizes[keyOffset + 1] << (loadedLocalOctave - 4 + localPitch);
        key_pressed = true;
        TX_Message[i + 1] += 2;
      }
      if (!key3)
      {

        localCurrentStepSizes[keyOffset + 2] = localMode == 1 ? (note_frequencies[keyOffset + 2] << (loadedLocalOctave - 4 + localPitch)) : stepSizes[keyOffset + 2] << (loadedLocalOctave - 4 + localPitch);
        key_pressed = true;
        TX_Message[i + 1] += 4;
      }
      if (!key4)
      {
        localCurrentStepSizes[keyOffset + 3] = localMode == 1 ? (note_frequencies[keyOffset + 3] << (loadedLocalOctave - 4 + localPitch)) : stepSizes[keyOffset + 3] << (loadedLocalOctave - 4 + localPitch);
        key_pressed = true;
        TX_Message[i + 1] += 8;
      }
    };
    localMiddleKeyboardFound = __atomic_load_n(&middleKeyboardFound, __ATOMIC_RELAXED);
    // Need to set the local octave as well. This would mean that the local step size would also have to be
    // altered accordingly, preferably on-the-fly
    if ((westDetect == 0) && (eastDetect == 0)) // Centre
    {
      TX_Message[4] = 5;
      tempLocalOctave = 5;
    }
    else if ((westDetect == 0) && (eastDetect == 1)) // Right
    {
      if (localMiddleKeyboardFound)
      {
        TX_Message[4] = 6;
        tempLocalOctave = 6;
      }
      else
      {
        TX_Message[4] = 5;
        tempLocalOctave = 5;
      }
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

    for (uint8_t i = 0; i < 12; i++)
    {
      uint8_t idx = (12 * (tempLocalOctave - 4)) + i;
      currentStepSizes[idx] = localCurrentStepSizes[i];
      if (localCurrentStepSizes[i] != 0)
      {
        prevStepSizes[idx] = localReverb ? localCurrentStepSizes[i] : 0;
        decayCounters[i] = 0;
        internalCounters[i] = 0;
      }
    }

    xSemaphoreGive(stepSizesMutex);

    if (((key_pressed || (key_pressed != keyPressedPrev)) && !(westDetect == 1 && eastDetect == 1)) || (westDetect != prevWestDetect && eastDetect != prevEastDetect))
    {
      xQueueSend(msgOutQ, (const void *)TX_Message, portMAX_DELAY);
    }

    keyPressedPrev = key_pressed;
    prevReverb = reverb;
    prevWestDetect = westDetect;
    prevEastDetect = eastDetect;
  }
}
