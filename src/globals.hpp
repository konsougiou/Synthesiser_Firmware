#include <Arduino.h>
#include <U8g2lib.h>
#include <STM32FreeRTOS.h> 

#include "knob.hpp"
  
// extern volatile uint32_t sampleRate;

extern  const uint32_t interval; //Display update interval
extern  const double frequency_ratio;
// extern volatile uint32_t step_scaling;

extern volatile uint32_t stepSizes[];  
extern char* keyOrder[];
extern volatile uint32_t freqs[];

extern volatile uint32_t currentStepSize;

extern volatile uint32_t activeKeys[];
extern volatile uint32_t currentStepSizes[12];

extern volatile uint8_t keyArray[7];
extern volatile uint32_t outVs[12];
extern volatile uint32_t lastStepSizes[12];
extern int angle;
// extern volatile int32_t totalVout;

extern SemaphoreHandle_t keyArrayMutex;
extern SemaphoreHandle_t currentStepSizesMutex;

extern const char* globalKeySymbol;

extern uint8_t knob3Rotation;
extern uint8_t knob2Rotation;
extern uint8_t knob3State;
extern uint8_t knob2State;

extern knob* knob3;
extern knob* knob2;