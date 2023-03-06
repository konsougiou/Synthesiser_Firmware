#include <Arduino.h>
#include <U8g2lib.h>
#include <STM32FreeRTOS.h> 

#include "knob.hpp"
  
extern  const uint32_t interval; //Display update interval
extern  const double frequency_ratio;
extern const uint32_t step_scaling;

extern const uint32_t stepSizes[];   

extern volatile uint32_t currentStepSize;

extern volatile uint8_t keyArray[7];

extern SemaphoreHandle_t keyArrayMutex;

extern uint8_t knob3Rotation;
extern uint8_t knob3State;

extern knob* knob3;