
#include "globals.hpp"
#include "tasks/tasks.hpp"
#include "ISRs/ISRs.hpp"
#include "utils/utils.hpp"
//#include "knob.hpp"

/*
CAN Format [8 bytes] (For key press):
0: First 4 keys
1: Second 4 keys
2: Last 4 keys
3: Octave
4: KeyChange (Boolean Value)
5: 
6: 
7: 
*/


/*
CAN Format [8 bytes]  (For knob change):
0: 
1: Reverb
2: Knob2Rotation (Pitch)
3:
4: KeyChange (Boolean Value)
5: 
6: 
7: 
*/

void setup()
{
  // Following code is run once:

  msgInQ = xQueueCreate(36, 8);
  msgOutQ = xQueueCreate(36, 8);
  
  knob3->setLimits(8, 0);
  knob2->setLimits(3, 0);
  knob1->setLimits(2, 0);

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
  setOutMuxBit(DRST_BIT, LOW);  // Assert display logic reset
  delayMicroseconds(2);
  setOutMuxBit(DRST_BIT, HIGH); // Release display logic reset
  u8g2.begin();
  setOutMuxBit(DEN_BIT, HIGH);  // Enable display power supply

  TIM_TypeDef *Instance = TIM1;
  HardwareTimer *sampleTimer = new HardwareTimer(Instance);

  // Initialise UART
  sampleTimer->setOverflow(22000, HERTZ_FORMAT);
  sampleTimer->attachInterrupt(sampleISR);
  sampleTimer->resume();

  TaskHandle_t transmitHandle = NULL;
  xTaskCreate(
      transmitTask,       /* Function that implements the task */
      "scanKeys",         /* Text name for the task */
      256,                /* Stack size in words, not bytes */
      NULL,               /* Parameter passed into the task */
      2,                  /* Task priority */
      &transmitHandle);

  TaskHandle_t displayUpdateHandle = NULL;
  xTaskCreate(
      displayUpdateTask,  /* Function that implements the task */
      "updateDisplay",    /* Text name for the task */
      256,                /* Stack size in words, not bytes */
      NULL,               /* Parameter passed into the task */
      1,                  /* Task priority */
      &displayUpdateHandle);

  TaskHandle_t decodeTaskHandle = NULL;
  xTaskCreate(
      decodeTask,         /* Function that implements the task */
      "decodeMessage",    /* Text name for the task */
      256,                /* Stack size in words, not bytes */
      NULL,               /* Parameter passed into the task */
      2,                  /* Task priority */
      &decodeTaskHandle);

  TaskHandle_t CAN_TX_TaskHandle = NULL;
  xTaskCreate(
      CAN_TX_Task,        /* Function that implements the task */
      "CAN_TX_Message",   /* Text name for the task */
      256,                /* Stack size in words, not bytes */
      NULL,               /* Parameter passed into the task */
      1,                  /* Task priority */
      &CAN_TX_TaskHandle);

  TaskHandle_t handshakeTaskHandle = NULL;
  xTaskCreate(
      handshakeTask,      /* Function that implements the task */
      "decodeMessage",    /* Text name for the task */
      256,                /* Stack size in words, not bytes */
      NULL,               /* Parameter passed into the task */
      1,                  /* Task priority */
      &handshakeTaskHandle);

  TaskHandle_t knobUpdateTaskHandle = NULL;
  xTaskCreate(
      knobUpdateTask,     /* Function that implements the task */
      "knobUpdate",       /* Text name for the task */
      128,                /* Stack size in words, not bytes */
      NULL,               /* Parameter passed into the task */
      1,                  /* Task priority */
      &knobUpdateTaskHandle);
    
  // Create the mutex for each semaphore that will be used and assign its handle in the setup function
  keyArrayMutex = xSemaphoreCreateMutex();
  queueReceiveMutex = xSemaphoreCreateMutex();
  currentStepSizesMutex = xSemaphoreCreateMutex();
  decodeStepSizesMutex = xSemaphoreCreateMutex();

  CAN_TX_Semaphore = xSemaphoreCreateCounting(3, 3);

  knob3->setLimits(8, 0);
  knob1->setLimits(2, 0);
  knob2->setLimits(3, 0);

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
