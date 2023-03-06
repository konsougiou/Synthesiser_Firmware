#include <Arduino.h>
#include <U8g2lib.h>
#include <math.h>
#include <STM32FreeRTOS.h>

#include "globals.hpp"

//Pin definitions
  //Row select and enable
  const int RA0_PIN = D3;
  const int RA1_PIN = D6;
  const int RA2_PIN = D12;
  const int REN_PIN = A5;

  //Matrix input and output
  const int C0_PIN = A2;
  const int C1_PIN = D9;
  const int C2_PIN = A6;
  const int C3_PIN = D1;
  const int OUT_PIN = D11;

  //Audio analogue out
  const int OUTL_PIN = A4;
  const int OUTR_PIN = A3;

  //Joystick analogue in
  const int JOYY_PIN = A0;
  const int JOYX_PIN = A1;

  //Output multiplexer bits
  const int DEN_BIT = 3;
  const int DRST_BIT = 4;
  const int HKOW_BIT = 5;
  const int HKOE_BIT = 6;

//Display driver object
U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);

//Function to set outputs using key matrix
void setOutMuxBit(const uint8_t bitIdx, const bool value) {
      digitalWrite(REN_PIN,LOW);
      digitalWrite(RA0_PIN, bitIdx & 0x01);
      digitalWrite(RA1_PIN, bitIdx & 0x02);
      digitalWrite(RA2_PIN, bitIdx & 0x04);
      digitalWrite(OUT_PIN,value);
      digitalWrite(REN_PIN,HIGH);
      delayMicroseconds(2);
      digitalWrite(REN_PIN,LOW);
}

void sampleISR() {
  static uint32_t phaseAcc = 0;

  phaseAcc += currentStepSize;
  int32_t Vout = (phaseAcc >> 24) - 128;
  Vout = Vout >> (8 - knob3Rotation);
  if (knob3Rotation == 0){
    Vout = 0;
  }
  analogWrite(OUTR_PIN, Vout + 128);
  }

uint8_t readCols(){

  uint8_t C0_read = digitalRead(C0_PIN);
  uint8_t C1_read = digitalRead(C1_PIN);
  uint8_t C2_read = digitalRead(C2_PIN);
  uint8_t C3_read = digitalRead(C3_PIN);
  
  return (C3_read << 3) + (C2_read << 2) + (C1_read << 1) + C0_read;
}

void setRow(uint8_t rowIdx){
  digitalWrite(REN_PIN, LOW);

  uint8_t msb = (rowIdx % 8) >> 2;
  uint8_t middle_bit = (rowIdx % 4) >> 1;
  uint8_t lsb = rowIdx % 2;
  
  digitalWrite(RA0_PIN, lsb);
  digitalWrite(RA1_PIN, middle_bit);
  digitalWrite(RA2_PIN, msb);

  digitalWrite(REN_PIN, HIGH); 
}

void scanKeysTask(void * pvParameters) {

  const TickType_t xFrequency = 30/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint8_t localKeyArray[7];

  while (1) {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );

    for(uint8_t rowIdx = 0; rowIdx < 5; rowIdx++){
      setRow(rowIdx);

      delayMicroseconds(3);

      localKeyArray[rowIdx] = readCols();
   }

    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

    std::copy(std::begin(localKeyArray), std::end(localKeyArray), std::begin(keyArray));

    xSemaphoreGive(keyArrayMutex);
 
    uint32_t tmpCurrentStepSize = 0;
    //extracting the step size
    for(int i = 0; i < 3; i++){
      
      xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
     
      uint8_t keyGroup = keyArray[i];

      xSemaphoreGive(keyArrayMutex);
      uint8_t key4 = (keyGroup % 16) >> 3; 
      uint8_t key3 = (keyGroup % 8) >> 2;
      uint8_t key2 = (keyGroup % 4) >> 1;
      uint8_t key1 = (keyGroup % 2);
      uint8_t keyOffset = i * 4;
      if (!key1){
        tmpCurrentStepSize = stepSizes[keyOffset];
      }
      if (!key2){
        tmpCurrentStepSize = stepSizes[keyOffset + 1];
      }
      if (!key3){
        tmpCurrentStepSize = stepSizes[keyOffset + 2];
      }    
      if (!key4){
        tmpCurrentStepSize = stepSizes[keyOffset + 3];
      }
    };

  __atomic_store_n(&currentStepSize, tmpCurrentStepSize, __ATOMIC_RELAXED);

  knob3->setLimits(8, 0);

  knob3->updateRotation(knob3Rotation);
  }
}
void displayUpdateTask(void * pvParameters) {

  const TickType_t xFrequency = 100/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );

    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

    uint32_t byte1 = ((uint32_t) keyArray[0]) << 8;
    uint32_t byte2 = ((uint32_t) keyArray[1]) << 4;   
    uint32_t byte3 = (uint32_t) keyArray[2]; 

    xSemaphoreGive(keyArrayMutex);   

    u8g2.clearBuffer();         // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font      

  //u8g2.print(currentStepSize);

    u8g2.setCursor(2,10);
    u8g2.println(byte1 + byte2 + byte3, HEX);
    u8g2.setCursor(2,20);

    uint32_t tmpStepSize = __atomic_load_n(&currentStepSize, __ATOMIC_RELAXED);
    u8g2.println(tmpStepSize);
    u8g2.setCursor(2,30);
    uint8_t tmpKnob3Rotation = __atomic_load_n(&knob3Rotation, __ATOMIC_RELAXED);
    u8g2.println(tmpKnob3Rotation, DEC);

    u8g2.sendBuffer(); // transfer internal memory to the display 

    digitalToggle(LED_BUILTIN);   //Toggle LED
  }
}

void setup() {
  // put your setup code here, to run once:

  //Set pin directions
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

  //Initialise display
  setOutMuxBit(DRST_BIT, LOW);  //Assert display logic reset
  delayMicroseconds(2);
  setOutMuxBit(DRST_BIT, HIGH);  //Release display logic reset
  u8g2.begin();
  setOutMuxBit(DEN_BIT, HIGH);  //Enable display power supply

  TIM_TypeDef *Instance = TIM1;
  HardwareTimer *sampleTimer = new HardwareTimer(Instance);
  //Initialise UART
  sampleTimer->setOverflow(22000, HERTZ_FORMAT);
  sampleTimer->attachInterrupt(sampleISR);
  sampleTimer->resume();

  TaskHandle_t scanKeysHandle = NULL;
  xTaskCreate(
  scanKeysTask,		/* Function that implements the task */
  "scanKeys",		/* Text name for the task */
  64,      		/* Stack size in words, not bytes */
  NULL,			/* Parameter passed into the task */
  2,			/* Task priority */
  &scanKeysHandle );

  TaskHandle_t displayUpdateHandle = NULL;
  xTaskCreate(
  displayUpdateTask,		/* Function that implements the task */
  "updateDisplay",		/* Text name for the task */
  256,      		/* Stack size in words, not bytes */
  NULL,			/* Parameter passed into the task */
  1,			/* Task priority */
  &scanKeysHandle);

  //Create the mutex and assign its handle in the setup function
  keyArrayMutex = xSemaphoreCreateMutex();

  vTaskStartScheduler();
  }

void loop() {

}

