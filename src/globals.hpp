#include <Arduino.h>
#include <U8g2lib.h>
#include <STM32FreeRTOS.h>
#include <math.h>
#include <numeric>
#include <STM32FreeRTOS.h>
#include <ES_CAN.h>

#include "knob.hpp"

extern const uint32_t interval; // Display update interval
extern const double frequency_ratio;
extern const uint32_t step_scaling;

extern const uint32_t stepSizes[];
extern volatile uint32_t freqs[];

extern volatile uint8_t localOctave;

extern volatile uint32_t currentStepSize;

extern uint32_t currentStepSizes[36];
extern uint32_t prevStepSizes[36];

extern volatile uint8_t keyArray[7];

extern SemaphoreHandle_t keyArrayMutex;
extern SemaphoreHandle_t queueReceiveMutex;
extern SemaphoreHandle_t currentStepSizesMutex;
extern SemaphoreHandle_t decodeStepSizesMutex;

extern SemaphoreHandle_t CAN_TX_Semaphore;

extern bool pressOrReceive; // False == Receive, True == Press

extern QueueHandle_t msgInQ;
extern QueueHandle_t msgOutQ;

extern uint8_t RX_Message[8];
extern uint8_t TX_Message[8];

extern uint8_t knob3Rotation;
extern uint8_t knob3State;
extern uint8_t knob2Rotation;

extern uint8_t westDetect;
extern uint8_t eastDetect;

extern uint8_t reverb;

extern uint32_t decayCounters[36];
extern uint32_t internalCounters[36];

extern knob *knob3;
extern knob *knob2;
extern knob *knob1;

// Pin definitions
// Row select and enable
extern const int RA0_PIN;
extern const int RA1_PIN;
extern const int RA2_PIN;
extern const int REN_PIN;

// Matrix input and output
extern const int C0_PIN;
extern const int C1_PIN;
extern const int C2_PIN;
extern const int C3_PIN;
extern const int OUT_PIN;

// Audio analogue out
extern const int OUTL_PIN;
extern const int OUTR_PIN;

// Joystick analogue in
extern const int JOYY_PIN;
extern const int JOYX_PIN;

// Output multiplexer bits
extern const int DEN_BIT;
extern const int DRST_BIT;
extern const int HKOW_BIT;
extern const int HKOE_BIT;

// Display driver object
extern U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2;
