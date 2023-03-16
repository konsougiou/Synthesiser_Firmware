
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

    u8g2.clearBuffer(); // clear the internal memory
    // u8g2.setFont(u8g2_font_ncenB08_tr);       // choose a suitable font
    u8g2.setFont(u8g2_font_5x7_tr); // choose a suitable font

    xSemaphoreTake(queueReceiveMutex, portMAX_DELAY);

    std::copy(std::begin(RX_Message), std::end(RX_Message), std::begin(tempRX_Message));

    xSemaphoreGive(queueReceiveMutex);

    // xSemaphoreTake(currentStepSizesMutex, portMAX_DELAY);

    // u8g2.setCursor(2, 10);
    // u8g2.print(currentStepSizes[0], HEX);
    // u8g2.setCursor(42, 10);
    // u8g2.print(currentStepSizes[1], HEX);
    // u8g2.setCursor(82, 10);
    // u8g2.print(currentStepSizes[2], HEX);
    // u8g2.setCursor(2, 20);
    // u8g2.print(prevStepSizes[0], HEX);
    // u8g2.setCursor(42, 20);
    // u8g2.print(prevStepSizes[1], HEX);
    // u8g2.setCursor(82, 20);
    // u8g2.print(prevStepSizes[2], HEX);

    // xSemaphoreGive(currentStepSizesMutex);

    u8g2.setCursor(24, 24);
    u8g2.println("Rev.");
    u8g2.drawFrame(25, 25, 15, 15);
    u8g2.setCursor(30, 32);
    uint8_t tmpReverb = __atomic_load_n(&reverb, __ATOMIC_RELAXED);
    u8g2.println(tmpReverb, HEX);

    u8g2.setCursor(55, 24);
    u8g2.println("Pitch");
    u8g2.drawFrame(60, 25, 15, 15);
    u8g2.setCursor(65, 32);
    uint8_t tmpKnob2Rotation = __atomic_load_n(&knob2Rotation, __ATOMIC_RELAXED);
    u8g2.println(tmpKnob2Rotation, DEC);

    u8g2.setCursor(95, 24);
    u8g2.println("Vol");
    u8g2.drawFrame(95, 25, 15, 15);
    u8g2.setCursor(100, 32);
    // u8g2.drawFrame(x, y, size, size);
    uint8_t tmpKnob3Rotation = __atomic_load_n(&knob3Rotation, __ATOMIC_RELAXED);
    u8g2.println(tmpKnob3Rotation, HEX);

    while (CAN_CheckRXLevel())
      CAN_RX(ID, RX_Message);

    u8g2.setCursor(45, 10);
    u8g2.println("Octave:");

    if (tempRX_Message[3] != 0)
    {
      u8g2.println(tempRX_Message[3]);
    }
    else
    {
      u8g2.println(0);
    }

    u8g2.setCursor(50, 30);

    u8g2.setCursor(40, 20);

    if ((westDetect == 0) && (eastDetect == 0))
    {
      u8g2.drawStr(112, 10, "oCo");
    }
    else if ((westDetect == 0) && (eastDetect == 1))
    {
      u8g2.drawStr(112, 10, "oRo");
    }
    else if ((westDetect == 1) && (eastDetect == 0))
    {
      u8g2.drawStr(112, 10, "oLo");
    }
    else
    {
      u8g2.drawStr(112, 10, "-_-");
    }

    // u8g2.setFont(u8g2_font_4x6_mr);
    // u8g2.drawStr(12, 6, "SNSRYOVRLD");

    // transfer internal memory to the display
    u8g2.sendBuffer();

    digitalToggle(LED_BUILTIN); // Toggle LED
  }
}