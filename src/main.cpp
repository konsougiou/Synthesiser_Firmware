

#include "globals.hpp"
#include "tasks/tasks.hpp"
#include "ISRs/ISRs.hpp"
#include "utils/utils.hpp"

// UNCOMMENT definitions below to disable tasks from being measured for their execution time.
#define DISABLE_ISR_SAWTOOTH
#define DISABLE_ISR_TRIANGLE
#define DISABLE_ISR_SINE
#define DISABLE_TRANSMIT_THREAD
#define DISABLE_DISPLAY_THREAD
#define DISABLE_DECODE_THREAD
// #define DISABLE_CANTX_THREAD
#define DISABLE_HANDSHAKE_THREAD
#define DISABLE_KNOB_UPDATE_THREAD
#define DISABLE_MODESWITCH_THREAD

/*
CAN Format [8 bytes] (For key press):
0: 0 (representing keychange)
1: First 4 keys
2: Second 4 keys
3: Last 4 keys
4: Octave
5:
6:
7:
*/

/*
CAN Format [8 bytes]  (For knob change):
0: 1 (representing knob rotation)
1: Reverb
2: Pitch
3: mode
4:
5:
6:
7:
*/

void setup()
{
    // Following code is run once:

    msgInQ = xQueueCreate(384, 8);
    msgOutQ = xQueueCreate(384, 8);

    // Set limits for each knob independently
    knob3->setLimits(8, 0);
    knob2->setLimits(2, 0);
    knob1->setLimits(2, 0);
    knob0->setLimits(2, 0);

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

#ifndef DISABLE_ISR_SAWTOOTH
    sawtoothwaveSampleTimer->setOverflow(22000, HERTZ_FORMAT);
    sawtoothwaveSampleTimer->attachInterrupt(sawtoothwaveISR);
    sawtoothwaveSampleTimer->resume();
#endif

#ifndef DISABLE_ISR_TRIANGLE
    trianglewaveSampleTimer->setOverflow(22000, HERTZ_FORMAT);
    trianglewaveSampleTimer->attachInterrupt(trianglewaveISR);
#endif

#ifndef DISABLE_ISR_SINE
    sinewaveSampleTimer->setOverflow(10000, HERTZ_FORMAT);
    sinewaveSampleTimer->attachInterrupt(sinewaveISR);
#endif

#ifndef DISABLE_TRANSMIT_THREAD
    TaskHandle_t transmitHandle = NULL;
    xTaskCreate(
        transmitTask, /* Function that implements the task */
        "scanKeys",   /* Text name for the task */
        256,          /* Stack size in words, not bytes */
        NULL,         /* Parameter passed into the task */
        2,            /* Task priority */
        &transmitHandle);
#endif

#ifndef DISABLE_DISPLAY_THREAD
    TaskHandle_t displayUpdateHandle = NULL;
    xTaskCreate(

        displayUpdateTask, /* Function that implements the task */
        "updateDisplay",   /* Text name for the task */
        256,               /* Stack size in words, not bytes */
        NULL,              /* Parameter passed into the task */
        1,                 /* Task priority */
        &displayUpdateHandle);
#endif

#ifndef DISABLE_DECODE_THREAD
    TaskHandle_t decodeTaskHandle = NULL;
    xTaskCreate(
        decodeTask,      /* Function that implements the task */
        "decodeMessage", /* Text name for the task */
        256,             /* Stack size in words, not bytes */
        NULL,            /* Parameter passed into the task */
        2,               /* Task priority */
        &decodeTaskHandle);
#endif

#ifndef DISABLE_CANTX_THREAD
    TaskHandle_t CAN_TX_TaskHandle = NULL;
    xTaskCreate(
        CAN_TX_Task,      /* Function that implements the task */
        "CAN_TX_Message", /* Text name for the task */
        256,              /* Stack size in words, not bytes */
        NULL,             /* Parameter passed into the task */
        1,                /* Task priority */
        &CAN_TX_TaskHandle);
#endif

#ifndef DISABLE_HANDSHAKE_THREAD
    TaskHandle_t handshakeTaskHandle = NULL;
    xTaskCreate(
        handshakeTask,   /* Function that implements the task */
        "decodeMessage", /* Text name for the task */
        256,             /* Stack size in words, not bytes */
        NULL,            /* Parameter passed into the task */
        1,               /* Task priority */
        &handshakeTaskHandle);
#endif

#ifndef DISABLE_KNOB_UPDATE_THREAD
    TaskHandle_t knobUpdateTaskHandle = NULL;
    xTaskCreate(
        knobUpdateTask, /* Function that implements the task */
        "knobUpdate",   /* Text name for the task */
        128,            /* Stack size in words, not bytes */
        NULL,           /* Parameter passed into the task */
        1,              /* Task priority */
        &knobUpdateTaskHandle);
#endif

#ifndef DISABLE_MODESWITCH_THREAD
    TaskHandle_t modeSwitchTaskHandle = NULL;
    xTaskCreate(
        modeSwitchTask, /* Function that implements the task */
        "switchMode",   /* Text name for the task */
        128,            /* Stack size in words, not bytes */
        NULL,           /* Parameter passed into the task */
        1,              /* Task priority */
        &modeSwitchTaskHandle);
#endif

    // Create the mutex for each semaphore that will be used and assign its handle in the setup function
    keyArrayMutex = xSemaphoreCreateMutex();
    queueReceiveMutex = xSemaphoreCreateMutex();
    stepSizesMutex = xSemaphoreCreateMutex();
    decodeStepSizesMutex = xSemaphoreCreateMutex();

    CAN_TX_Semaphore = xSemaphoreCreateCounting(3, 3);

    CAN_Init(false);
    setCANFilter(0x123, 0x7ff);
    CAN_RegisterRX_ISR(CAN_RX_ISR);
    CAN_RegisterTX_ISR(CAN_TX_ISR);
    CAN_Start();

    // vTaskStartScheduler();

#ifndef DISABLE_ISR_SAWTOOTH
    uint32_t startTime = micros();
    for (int iter = 0; iter < 32; iter++)
    {
        sawtoothwaveISR();
    }
    Serial.print("sawtoothwaveISR exec time: ");
    Serial.println(micros() - startTime);
    while (1)
        ;
#endif

#ifndef DISABLE_ISR_TRIANGLE
    uint32_t startTime = micros();
    for (int iter = 0; iter < 32; iter++)
    {
        trianglewaveISR();
    }
    Serial.print("trianglewaveISR exec time: ");
    Serial.println(micros() - startTime);
    while (1)
        ;
#endif

#ifndef DISABLE_ISR_SINE
    uint32_t startTime = micros();
    for (int iter = 0; iter < 32; iter++)
    {
        sinewaveISR();
    }
    Serial.print("sinewaveISR exec time: ");
    Serial.println(micros() - startTime);
    while (1)
        ;
#endif

#ifndef DISABLE_TRANSMIT_THREAD
    uint32_t startTime = micros();
    for (int iter = 0; iter < 32; iter++)
    {
        transmitTask(NULL);
    }
    Serial.print("transmitTask exec time: ");
    Serial.println(micros() - startTime);
    while (1)
        ;
#endif

#ifndef DISABLE_DISPLAY_THREAD
    uint32_t startTime = micros();
    for (int iter = 0; iter < 32; iter++)
    {
        displayUpdateTask(NULL);
    }
    Serial.print("displayUpdateTask exec time: ");
    Serial.println(micros() - startTime);
    while (1)
        ;
#endif

#ifndef DISABLE_DECODE_THREAD
    uint32_t startTime = micros();
    for (int iter = 0; iter < 32; iter++)
    {
        decodeTask(NULL);
    }
    Serial.print("decodeTask exec time: ");
    Serial.println(micros() - startTime);
    while (1)
        ;
#endif

#ifndef DISABLE_CANTX_THREAD
    uint32_t startTime = micros();
    for (int iter = 0; iter < 32; iter++)
    {
        CAN_TX_Task(NULL);
    }
    Serial.print("CAN_TX_Task exec time: ");
    Serial.println(micros() - startTime);
    while (1)
        ;
#endif

#ifndef DISABLE_HANDSHAKE_THREAD
    uint32_t startTime = micros();
    for (int iter = 0; iter < 32; iter++)
    {
        handshakeTask(NULL);
    }
    Serial.print("handshakeTask exec time: ");
    Serial.println(micros() - startTime);
    while (1)
        ;
#endif

#ifndef DISABLE_KNOB_UPDATE_THREAD
    uint32_t startTime = micros();
    for (int iter = 0; iter < 32; iter++)
    {
        knobUpdateTask(NULL);
    }
    Serial.print("knobUpdateTask exec time: ");
    Serial.println(micros() - startTime);
    while (1)
        ;
#endif

#ifndef DISABLE_MODESWITCH_THREAD
    uint32_t startTime = micros();
    for (int iter = 0; iter < 32; iter++)
    {
        modeSwitchTask(NULL);
    }
    Serial.print("modeSwitchTask exec time: ");
    Serial.println(micros() - startTime);
    while (1)
        ;
#endif
}

void loop()
{
}
