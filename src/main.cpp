#include <Arduino.h>
#include <U8g2lib.h>
#include <math.h>
#include <STM32FreeRTOS.h>
#include <ES_CAN.h>

#include "globals.hpp"

// Pin definitions
// Row select and enable
const int RA0_PIN = D3;
const int RA1_PIN = D6;
const int RA2_PIN = D12;
const int REN_PIN = A5;

// Matrix input and output
const int C0_PIN = A2;
const int C1_PIN = D9;
const int C2_PIN = A6;
const int C3_PIN = D1;
const int OUT_PIN = D11;

// Audio analogue out
const int OUTL_PIN = A4;
const int OUTR_PIN = A3;

// Joystick analogue in
const int JOYY_PIN = A0;
const int JOYX_PIN = A1;

// Output multiplexer bits
const int DEN_BIT = 3;
const int DRST_BIT = 4;
const int HKOW_BIT = 5;
const int HKOE_BIT = 6;

// Display driver object
U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);

// Function to set outputs using key matrix
void setOutMuxBit(const uint8_t bitIdx, const bool value)
{
  digitalWrite(REN_PIN, LOW);
  digitalWrite(RA0_PIN, bitIdx & 0x01);
  digitalWrite(RA1_PIN, bitIdx & 0x02);
  digitalWrite(RA2_PIN, bitIdx & 0x04);
  digitalWrite(OUT_PIN, value);
  digitalWrite(REN_PIN, HIGH);
  delayMicroseconds(2);
  digitalWrite(REN_PIN, LOW);
}

void sampleISR()
{
  static uint32_t phaseAcc = 0;

  phaseAcc += currentStepSize;
  int32_t Vout = (phaseAcc >> 24) - 128;
  Vout = Vout >> (8 - knob3Rotation);
  if (knob3Rotation == 0)
  {
    Vout = 0;
  }

  analogWrite(OUTR_PIN, Vout + 128);
}

uint8_t readCols()
{

  uint8_t C0_read = digitalRead(C0_PIN);
  uint8_t C1_read = digitalRead(C1_PIN);
  uint8_t C2_read = digitalRead(C2_PIN);
  uint8_t C3_read = digitalRead(C3_PIN);

  return (C3_read << 3) + (C2_read << 2) + (C1_read << 1) + C0_read;
}

void setRow(uint8_t rowIdx)
{
  digitalWrite(REN_PIN, LOW);

  uint8_t msb = (rowIdx % 8) >> 2;
  uint8_t middle_bit = (rowIdx % 4) >> 1;
  uint8_t lsb = rowIdx % 2;

  digitalWrite(RA0_PIN, lsb);
  digitalWrite(RA1_PIN, middle_bit);
  digitalWrite(RA2_PIN, msb);

  digitalWrite(REN_PIN, HIGH);
}

void CAN_RX_ISR(void)
{
  uint8_t RX_Message_ISR[8];
  uint32_t ID;
  CAN_RX(ID, RX_Message_ISR);
  xQueueSendFromISR(msgInQ, RX_Message_ISR, NULL);
}

void CAN_TX_ISR(void)
{
  xSemaphoreGiveFromISR(CAN_TX_Semaphore, NULL);
}
void handshakeTask(void *pvParameters)
{

  const TickType_t xFrequency = 30 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (1)
  {

    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    setRow(5);                // Set row address
    digitalWrite(OUT_PIN, 1); // Set value to latch in DFF
    digitalWrite(REN_PIN, 1); // Enable selected row
    delayMicroseconds(3);     // Wait for column inputs to stabilise
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    keyArray[5] = readCols(); // Read column inputs
    xSemaphoreGive(keyArrayMutex);
    digitalWrite(REN_PIN, 0);

    delayMicroseconds(3);

    setRow(6);                // Set row address
    digitalWrite(OUT_PIN, 1); // Set value to latch in DFF
    digitalWrite(REN_PIN, 1); // Enable selected row
    delayMicroseconds(3);
    // Wait for column inputs to stabilise
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    keyArray[6] = readCols(); // Read column inputs
    xSemaphoreGive(keyArrayMutex);
    digitalWrite(REN_PIN, 0);
  }
}

void scanKeysTask(void *pvParameters)
{

  const TickType_t xFrequency = 30 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint8_t localKeyArray[5];

  // uint8_t TX_Message[8] = {0};
  uint32_t ID;
  bool keyPressedPrev = false;

  // uint8_t localOctave = 4;
  uint8_t tempLocalOctave = 5;

  // step Sizes for 4th octave
  // const uint32_t localstepSizes[] = {51069198, 54102708, 57316409, 60721004, 64327831, 68148905,
  //                                    72196950, 76485448, 81028684, 85841788, 90940790, 96342673};

  // volatile uint8_t TX_Message[8] = {0};

  while (1)
  {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

    uint8_t westDetect = ((uint32_t)keyArray[5]); // % 16) >> 3;
    uint8_t eastDetect = ((uint32_t)keyArray[6]); // % 16) >> 3;

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

    uint32_t tmpCurrentStepSize = 0;
    // extracting the step size

    TX_Message[0] = 82;
    bool key_pressed = false;

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

      if (!key1)
      {
        tmpCurrentStepSize = stepSizes[keyOffset];
        key_pressed = true;
        TX_Message[2] = keyOffset;
      }
      if (!key2)
      {
        tmpCurrentStepSize = stepSizes[keyOffset + 1];
        key_pressed = true;
        TX_Message[2] = keyOffset + 1;
      }
      if (!key3)
      {
        tmpCurrentStepSize = stepSizes[keyOffset + 2];
        key_pressed = true;
        TX_Message[2] = keyOffset + 2;
      }
      if (!key4)
      {
        tmpCurrentStepSize = stepSizes[keyOffset + 3];
        key_pressed = true;
        TX_Message[2] = keyOffset + 3;
      }
    };
    // TX_Message[1] = 4;

    // Need to set the local octave as well.
    // This would mean that the local step size would also have to be altered accordingly.
    // preferably on-the-fly
    if ((westDetect == 0x7) && (eastDetect == 0x7)) // Centre
    {
      TX_Message[1] = 5;
      tempLocalOctave = 5;
    }
    else if ((westDetect == 0x7) && (eastDetect == 0xF)) // Right
    {
      TX_Message[1] = 6;
      tempLocalOctave = 6;
    }
    else if ((westDetect == 0xF) && (eastDetect == 0x7)) // Left
    {
      TX_Message[1] = 4;
      tempLocalOctave = 4;
    }
    else // Centre
    {
      TX_Message[1] = 9;
      tempLocalOctave = 9;
    }
    __atomic_store_n(&localOctave, tempLocalOctave, __ATOMIC_RELAXED);

    if (key_pressed)
    {
      TX_Message[0] = 80;
      xQueueSend(msgOutQ, (const void *)TX_Message, portMAX_DELAY);
    }
    else if (key_pressed != keyPressedPrev)
    {
      xQueueSend(msgOutQ, (const void *)TX_Message, portMAX_DELAY);
    }

    // CAN_TX(0x123, (uint8_t *)TX_Message); // Sending the CAN message with scanned keys.

    if (pressOrReceive == false)
    {
      __atomic_store_n(&currentStepSize, tmpCurrentStepSize, __ATOMIC_RELAXED);
    }
    else
    {
      // Their code comes in
      continue;
    }

    keyPressedPrev = key_pressed;
    knob3->setLimits(8, 0);

    knob3->updateRotation(knob3Rotation);
  }
}
void displayUpdateTask(void *pvParameters)
{

  const TickType_t xFrequency = 100 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  uint32_t ID;
  uint8_t RX_Message[8] = {0};

  while (1)
  {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

    uint32_t byte1 = ((uint32_t)keyArray[0]) << 8;
    uint32_t byte2 = ((uint32_t)keyArray[1]) << 4;
    uint32_t byte3 = (uint32_t)keyArray[2];
    uint8_t westDetect = ((uint32_t)keyArray[5]); // % 16) >> 3;
    uint8_t eastDetect = ((uint32_t)keyArray[6]); // % 16) >> 3;

    xSemaphoreGive(keyArrayMutex);

    u8g2.clearBuffer();                 // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font

    // u8g2.print(currentStepSize);

    u8g2.setCursor(2, 10);
    u8g2.println(byte1 + byte2 + byte3, BIN);
    u8g2.setCursor(2, 20);

    uint32_t tmpStepSize = __atomic_load_n(&currentStepSize, __ATOMIC_RELAXED);
    u8g2.println(tmpStepSize);
    u8g2.setCursor(2, 30);
    uint8_t tmpKnob3Rotation = __atomic_load_n(&knob3Rotation, __ATOMIC_RELAXED);
    u8g2.println(tmpKnob3Rotation, DEC);

    while (CAN_CheckRXLevel())
      CAN_RX(ID, RX_Message);

    u8g2.setCursor(46, 30);
    // u8g2.print((char)RX_Message[0]);
    u8g2.print(TX_Message[1]);
    // u8g2.print(RX_Message[2]);
    // u8g2.print(HAL_GetUIDw0());

    u8g2.setCursor(66, 30);
    u8g2.print(eastDetect, HEX);
    u8g2.print(westDetect, HEX);
    // u8g2.print(HAL_GetUIDw2());

    u8g2.setCursor(40, 20);
    if ((westDetect == 0x7) && (eastDetect == 0x7))
    {
      u8g2.drawStr(60, 20, "Centre");
    }
    else if ((westDetect == 0x7) && (eastDetect == 0xF))
    {
      u8g2.drawStr(60, 20, "Right");
    }
    else if ((westDetect == 0xF) && (eastDetect == 0x7))
    {
      u8g2.drawStr(60, 20, "Left");
    }
    else
    {
      u8g2.drawStr(60, 20, "-_-");
    }

    u8g2.setCursor(86, 30);
    uint32_t tmpLocalOctave = __atomic_load_n(&localOctave, __ATOMIC_RELAXED);
    u8g2.print(tmpLocalOctave, HEX);
    // TODO: octave thing in the morning

    u8g2.sendBuffer(); // transfer internal memory to the display

    digitalToggle(LED_BUILTIN); // Toggle LED
  }
}

void decodeTask(void *pvParameters)
{
  const TickType_t xFrequency = 80 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  // bool pressOrReceive = false; // False == Receive, True == Press
  uint8_t tempRX_Message[8] = {0};

  while (1)
  {
    // vTaskDelayUntil(&xLastWakeTime, xFrequency);
    xQueueReceive(msgInQ, tempRX_Message, portMAX_DELAY);

    xSemaphoreTake(queueReceiveMutex, portMAX_DELAY);

    std::copy(std::begin(tempRX_Message), std::end(tempRX_Message), std::begin(RX_Message));

    xSemaphoreGive(queueReceiveMutex);

    if (RX_Message[0] == 0x50)
    {
      pressOrReceive = true;
      uint32_t dTmpCurrentStepSize = stepSizes[RX_Message[2]] << ((RX_Message[1]) - 4);
      __atomic_store_n(&currentStepSize, dTmpCurrentStepSize, __ATOMIC_RELAXED);
    }
    else
    {
      pressOrReceive = false;
      // Line below is essentially this: currentStepSize = 0;
      __atomic_store_n(&currentStepSize, 0, __ATOMIC_RELAXED);
    }
  }
}

void CAN_TX_Task(void *pvParameters)
{
  uint8_t msgOut[8];
  while (1)
  {
    xQueueReceive(msgOutQ, msgOut, portMAX_DELAY);
    xSemaphoreTake(CAN_TX_Semaphore, portMAX_DELAY);
    CAN_TX(0x123, msgOut);
  }
}

void setup()
{
  // put your setup code here, to run once:

  msgInQ = xQueueCreate(36, 8);
  msgOutQ = xQueueCreate(36, 8);

  // Set pin directions
  pinMode(RA0_PIN, OUTPUT);
  pinMode(RA1_PIN, OUTPUT);
  pinMode(RA2_PIN, OUTPUT);
  pinMode(REN_PIN, OUTPUT);
  pinMode(OUT_PIN, OUTPUT);
  pinMode(OUTL_PIN, OUTPUT);
  pinMode(OUTR_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(C0_PIN, INPUT);
  pinMode(C1_PIN, INPUT);
  pinMode(C2_PIN, INPUT);
  pinMode(C3_PIN, INPUT);
  pinMode(JOYX_PIN, INPUT);
  pinMode(JOYY_PIN, INPUT);

  // Initialise display
  setOutMuxBit(DRST_BIT, LOW); // Assert display logic reset
  delayMicroseconds(2);
  setOutMuxBit(DRST_BIT, HIGH); // Release display logic reset
  u8g2.begin();
  setOutMuxBit(DEN_BIT, HIGH); // Enable display power supply

  TIM_TypeDef *Instance = TIM1;
  HardwareTimer *sampleTimer = new HardwareTimer(Instance);
  // Initialise UART
  sampleTimer->setOverflow(22000, HERTZ_FORMAT);
  sampleTimer->attachInterrupt(sampleISR);
  sampleTimer->resume();

  TaskHandle_t scanKeysHandle = NULL;
  xTaskCreate(
      scanKeysTask, /* Function that implements the task */
      "scanKeys",   /* Text name for the task */
      64,           /* Stack size in words, not bytes */
      NULL,         /* Parameter passed into the task */
      2,            /* Task priority */
      &scanKeysHandle);

  TaskHandle_t displayUpdateHandle = NULL;
  xTaskCreate(
      displayUpdateTask, /* Function that implements the task */
      "updateDisplay",   /* Text name for the task */
      256,               /* Stack size in words, not bytes */
      NULL,              /* Parameter passed into the task */
      1,                 /* Task priority */
      &scanKeysHandle);

  TaskHandle_t decodeTaskHandle = NULL;
  xTaskCreate(
      decodeTask,      /* Function that implements the task */
      "decodeMessage", /* Text name for the task */
      256,             /* Stack size in words, not bytes */
      NULL,            /* Parameter passed into the task */
      1,               /* Task priority */
      &decodeTaskHandle);

  TaskHandle_t CAN_TX_TaskHandle = NULL;
  xTaskCreate(
      CAN_TX_Task,      /* Function that implements the task */
      "CAN_TX_Message", /* Text name for the task */
      256,              /* Stack size in words, not bytes */
      NULL,             /* Parameter passed into the task */
      1,                /* Task priority */
      &CAN_TX_TaskHandle);

  TaskHandle_t handshakeTaskHandle = NULL;
  xTaskCreate(
      handshakeTask,   /* Function that implements the task */
      "decodeMessage", /* Text name for the task */
      256,             /* Stack size in words, not bytes */
      NULL,            /* Parameter passed into the task */
      1,               /* Task priority */
      &decodeTaskHandle);

  // Create the mutex and assign its handle in the setup function
  keyArrayMutex = xSemaphoreCreateMutex();
  queueReceiveMutex = xSemaphoreCreateMutex();

  CAN_TX_Semaphore = xSemaphoreCreateCounting(3, 3);

  CAN_Init(false);
  setCANFilter(0x123, 0x7ff);
  CAN_RegisterRX_ISR(CAN_RX_ISR);
  CAN_RegisterTX_ISR(CAN_TX_ISR);
  CAN_Start();

  vTaskStartScheduler();
}

void loop()
{
}
