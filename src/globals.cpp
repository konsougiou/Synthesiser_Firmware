#include <Arduino.h>
#include <U8g2lib.h>
#include <math.h>
#include <STM32FreeRTOS.h>

#include "globals.hpp"

const uint32_t interval = 100; // Display update interval
const double frequency_ratio = pow(2.0, 1.0 / 12.0);
const uint32_t step_scaling = pow(2, 32) / 22000;

// const uint32_t stepSizes[] = {51069198, 54102708, 57316409, 60721004, 64327831, 68148905,
//                               72196950, 76485448, 81028684, 85841788, 90940790, 96342673};
volatile uint32_t stepSizes [] = {262,277,294,311,330,349,367,392,415,440,466,494};
volatile uint32_t freqs [] = {2000, 5000, 10000, 17000, 22000, 28000, 30000, 40000, 50000};


volatile uint8_t localOctave = 4;

volatile uint32_t currentStepSize = 0;

volatile uint8_t keyArray[7];
volatile uint8_t chordArray[12] = {0};

// volatile uint8_t TX_Message[8] = {0};

QueueHandle_t msgInQ;
QueueHandle_t msgOutQ;

uint32_t currentStepSizes[36] = {0};

uint8_t RX_Message[8] = {0};
uint8_t TX_Message[8] = {0};

uint8_t westDetect;
uint8_t eastDetect;
uint8_t knob3press;
uint8_t knob2press;
uint8_t knob1press;
uint8_t knob0press;

bool chordRecord = false;
bool chordPlay = false;

SemaphoreHandle_t keyArrayMutex;
SemaphoreHandle_t queueReceiveMutex;
SemaphoreHandle_t currentStepSizesMutex;
SemaphoreHandle_t decodeStepSizesMutex;

SemaphoreHandle_t CAN_TX_Semaphore;

bool pressOrReceive = false; // False == Receive, True == Press

uint8_t knob3Rotation = 6;
uint8_t knob2Rotation = 4;

knob *knob3 = new knob(3);
knob *knob2 = new knob(2);