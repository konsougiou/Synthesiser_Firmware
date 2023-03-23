
#include "tasks.hpp"
#include "../globals.hpp"

void displayUpdateTask(void *pvParameters)
{

  const TickType_t xFrequency = 100 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  uint32_t ID;
  uint8_t tempRX_Message[8] = {0};

  while (1)
  {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

    uint32_t byte1 = ((uint32_t)keyArray[0]) << 8;
    uint32_t byte2 = ((uint32_t)keyArray[1]) << 4;
    uint32_t byte3 = (uint32_t)keyArray[2];
    westDetect = (((uint32_t)keyArray[5]) % 16) >> 3;
    eastDetect = (((uint32_t)keyArray[6]) % 16) >> 3;

    xSemaphoreGive(keyArrayMutex);

    u8g2.clearBuffer();                 // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font

    xSemaphoreTake(queueReceiveMutex, portMAX_DELAY);

    std::copy(std::begin(RX_Message), std::end(RX_Message), std::begin(tempRX_Message));

    xSemaphoreGive(queueReceiveMutex);

    xSemaphoreTake(stepSizesMutex, portMAX_DELAY);

    u8g2.setCursor(2, 10);
    u8g2.print(currentStepSizes[0], HEX);
    u8g2.setCursor(42, 10);
    u8g2.print(currentStepSizes[1], HEX);
    u8g2.setCursor(82, 10);
    u8g2.print(currentStepSizes[2], HEX);
    u8g2.setCursor(2, 20);
    u8g2.print(prevStepSizes[0], HEX);
    u8g2.setCursor(42, 20);
    u8g2.print(prevStepSizes[1], HEX);
    u8g2.setCursor(82, 20);
    u8g2.print(prevStepSizes[2], HEX);

    xSemaphoreGive(stepSizesMutex);

    u8g2.setCursor(72, 30);
    uint8_t localMode = __atomic_load_n(&mode, __ATOMIC_RELAXED);
    u8g2.println(localMode, HEX);

    u8g2.setCursor(82, 30);
    uint8_t localReverb = __atomic_load_n(&reverb, __ATOMIC_RELAXED);
    u8g2.println(localReverb ,HEX);

    u8g2.setCursor(92, 30);
    uint8_t localPitch = __atomic_load_n(&pitch, __ATOMIC_RELAXED);
    u8g2.println(localPitch ,HEX);

    u8g2.setCursor(102, 30);
    uint8_t localKnob3Rotation = __atomic_load_n(&volume, __ATOMIC_RELAXED);
    u8g2.println(localKnob3Rotation, DEC);

    while (CAN_CheckRXLevel())
      CAN_RX(ID, RX_Message);

    u8g2.setCursor(42, 20);
    u8g2.setCursor(50, 30);

    u8g2.setCursor(40, 20);
    if ((westDetect == 0) && (eastDetect == 0))
    {
      u8g2.drawStr(80, 20, "Centre");
    }
    else if ((westDetect == 0) && (eastDetect == 1))
    {
      u8g2.drawStr(80, 20, "Right");
    }
    else if ((westDetect == 1) && (eastDetect == 0))
    {
      u8g2.drawStr(80, 20, "Left");
    }
    else
    {
      u8g2.drawStr(80, 20, "-_-");
    }
    
    // transfer internal memory to the display
    u8g2.sendBuffer(); 

    digitalToggle(LED_BUILTIN); // Toggle LED
  }
}