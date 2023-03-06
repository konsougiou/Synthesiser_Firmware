#include <Arduino.h>
#include <U8g2lib.h>
#include <STM32FreeRTOS.h> 

#include "globals.hpp"

knob::knob(uint8_t number):  knobNumber(number){
    upperLimit = 8;
    lowerLimit = 0;
    knobState = 0;
}

void knob::updateRotation(uint8_t& knobRotation){
  xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

  uint8_t knobCols = keyArray[3];
  
  xSemaphoreGive(keyArrayMutex);  

  uint8_t lastTransition = 0;
  uint8_t localKnobRotation = __atomic_load_n(&knobRotation, __ATOMIC_RELAXED);
  
  uint8_t knob3B = ((knobCols) % 4) >> 1;
  uint8_t knob3A = knobCols % 2;

  uint8_t newKnobState = (knob3B << 1) + knob3A;

  if (newKnobState == knobState + 1){
    if (newKnobState == 3){
      localKnobRotation -= 1;
      lastTransition = -1;
    }
    else{
      localKnobRotation += 1;
      lastTransition = 1;
    }
  }
  else if(newKnobState == knobState - 1){
    if (newKnobState == 2){
      localKnobRotation += 1;
      lastTransition = 1;
    }
    else{
      localKnobRotation -= 1;
      lastTransition = -1;
    }
  }
  else if (newKnobState == !knobState){
    localKnobRotation += lastTransition;
  }
  knobState = newKnobState;

  if(localKnobRotation <= upperLimit && localKnobRotation >= lowerLimit){
    __atomic_store_n(&knobRotation, localKnobRotation, __ATOMIC_RELAXED);
  }

}

void knob::setLimits(uint8_t upper, uint8_t lower){
    upperLimit = upper;
    lowerLimit = lower;
}




