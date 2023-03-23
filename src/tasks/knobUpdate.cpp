
#include "tasks.hpp"
#include "../globals.hpp"

void knobUpdateTask(void *pvParameters)
{
  const TickType_t xFrequency = 50 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  uint8_t localReverb;
  uint8_t prevReverb;

  uint8_t localPitch;
  uint8_t prevPitch;

  uint8_t localMode;
  uint8_t prevMode;

  while (1)
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
    localPitch = __atomic_load_n(&pitch, __ATOMIC_RELAXED);
    TX_Message[2] = localPitch;

    localMode = __atomic_load_n(&mode, __ATOMIC_RELAXED);
    TX_Message[3] = localMode;

    if (((localPitch != prevPitch || localReverb != prevReverb || localMode != prevMode) && !(westDetect == 1 && eastDetect == 1)) || 1)
    {
      xQueueSend(msgOutQ, (const void *)TX_Message, portMAX_DELAY);
    }

    prevReverb = localReverb;
    prevPitch = localPitch;
    prevMode = localMode;

    // // For reverb knob
    // knob1->updateRotation(reverb);

    // // For pitch knob
    // knob2->updateRotation(pitch);

    // // For volume knob
    // knob3->updateRotation(volume);

    // // For mode
    // knob0->updateRotation(mode);
    break;
  }
}