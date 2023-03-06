#include <Arduino.h>
#include <U8g2lib.h>
#include <math.h>
#include <STM32FreeRTOS.h> 

#include "globals.hpp"
  
// volatile uint32_t sampleRate = 22000;

const uint32_t interval = 100; //Display update interval
const double frequency_ratio = pow(2.0, 1.0/12.0);
// volatile uint32_t step_scaling = pow(2, 32) / 22000;
// const uint32_t stepSizes[] = {51069198, 54102708, 57316409, 60721004, 64327831, 68148905, 
// 72196950, 76485448, 81028684, 85841788, 90940790, 96342673};
// volatile uint32_t stepSizes [] = {51149156, 54077543, 57396381, 60715220, 64424509, 68133799, 71647864, 76528508, 81018701, 85899346, 90975216, 96441538};
volatile uint32_t stepSizes [] = {262,277,294,311,330,349,367,392,415,440,466,494};
// volatile uint32_t stepSizes [] = {step_scaling*262, step_scaling*277, step_scaling*294, step_scaling*311, step_scaling*330, step_scaling*349, step_scaling*367, step_scaling*392, step_scaling*415, step_scaling*440, step_scaling*466, step_scaling*494};
char* keyOrder [] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};

volatile uint32_t currentStepSize = 0;



// THIS IS CONST INSTEAD OF VOLATILE MIGHT CAUSE PROBLEMS???
const char* globalKeySymbol = 0;




volatile uint8_t keyArray[7];

SemaphoreHandle_t keyArrayMutex;

uint8_t knob3Rotation = 0;
uint8_t knob2Rotation = 0;

knob* knob3 = new knob(3);
knob* knob2 = new knob(2);