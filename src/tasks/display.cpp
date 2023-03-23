
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
    // vTaskDelayUntil(&xLastWakeTime, xFrequency);

    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

    uint32_t byte1 = ((uint32_t)keyArray[0]) << 8;
    uint32_t byte2 = ((uint32_t)keyArray[1]) << 4;
    uint32_t byte3 = (uint32_t)keyArray[2];
    westDetect = (((uint32_t)keyArray[5]) % 16) >> 3;
    eastDetect = (((uint32_t)keyArray[6]) % 16) >> 3;

    xSemaphoreGive(keyArrayMutex);

    // //u8g2.clearBuffer();             // clear the internal memory
                                    // //u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    //u8g2.setFont(u8g2_font_5x7_tr); // choose a suitable font

    xSemaphoreTake(queueReceiveMutex, portMAX_DELAY);

    std::copy(std::begin(RX_Message), std::end(RX_Message), std::begin(tempRX_Message));

    xSemaphoreGive(queueReceiveMutex);

    ////u8g2.setCursor(4, 24);
    //u8g2.println("Mode");
    //u8g2.drawFrame(5, 25, 15, 15);
    //u8g2.setCursor(10, 32);
    uint8_t localMode = __atomic_load_n(&mode, __ATOMIC_RELAXED);
    //u8g2.println(localMode, HEX);

    //u8g2.setCursor(39, 24);
    //u8g2.println("Rev.");
    //u8g2.drawFrame(40, 25, 15, 15);
    //u8g2.setCursor(45, 32);
    uint8_t localReverb = __atomic_load_n(&reverb, __ATOMIC_RELAXED);
    //u8g2.println(localReverb, HEX);

    //u8g2.setCursor(70, 24);
    //u8g2.println("Pitch");
    //u8g2.drawFrame(75, 25, 15, 15);
    //u8g2.setCursor(80, 32);
    uint8_t localPitch = __atomic_load_n(&pitch, __ATOMIC_RELAXED);
    //u8g2.println(localPitch, HEX);

    //u8g2.setCursor(112, 24);
    //u8g2.println("Vol");
    //u8g2.drawFrame(112, 25, 15, 15);
    //u8g2.setCursor(118, 32);
    uint8_t localKnob3Rotation = __atomic_load_n(&volume, __ATOMIC_RELAXED);
    //u8g2.println(localKnob3Rotation, DEC);

    // while (CAN_CheckRXLevel())
    //   CAN_RX(ID, RX_Message);

    //u8g2.setCursor(45, 10);
    //u8g2.println("Octave:");

    if (tempRX_Message[4] != 0)
    {
      //u8g2.println(tempRX_Message[3]);
    }
    else
    {
      //u8g2.println(0);
    }

    //u8g2.setCursor(40, 20);

    if ((westDetect == 0) && (eastDetect == 0))
    {
      //u8g2.drawStr(112, 10, ">C<");
    }
    else if ((westDetect == 0) && (eastDetect == 1))
    {
      //u8g2.drawStr(112, 10, ">R<");
    }
    else if ((westDetect == 1) && (eastDetect == 0))
    {
      //u8g2.drawStr(112, 10, ">L<");
    }
    else
    {
      //u8g2.drawStr(112, 10, "U_U");
    }

    // transfer internal memory to the display
    // //u8g2.sendBuffer();

    digitalToggle(LED_BUILTIN); // Toggle LED
    break;
  }
}