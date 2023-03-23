
#include "globals.hpp"
//#include "knob.hpp"

knob::knob(uint8_t number):  knobNumber(number){
    upperLimit = 8;
    lowerLimit = 0;
    knobState = 0;
    keyArrayIdx = 4 - (number / 2);
}
 
void knob::updateRotation(volatile uint8_t& knobRotation){
  xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

  uint8_t knobCols = keyArray[keyArrayIdx];
  
  xSemaphoreGive(keyArrayMutex);  

  uint8_t lastTransition = 0;
  uint8_t localKnobRotation = __atomic_load_n(&knobRotation, __ATOMIC_RELAXED);
  
  uint8_t knobB = (knobNumber % 2) ? (knobCols % 4) >> 1 : (knobCols % 16) >> 3;
  uint8_t knobA = (knobNumber % 2) ? (knobCols % 2) : (knobCols % 8) >> 2;

  uint8_t newKnobState = (knobB << 1) + knobA;

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




