
#include "../globals.hpp"

void sampleISR()
{
  static uint32_t phaseAccArray[36] = {0};
  static uint32_t Vouts[36] = {0};
  int32_t totalVout = 0;
  int32_t Vout;
  for(int z=0;z<36;z++){
    Vout = 0;
    if(currentStepSizes[z]==0){
      if(prevStepSizes[z] != 0){
        if (decayCounters[z] == 22000){
          decayCounters[z] = 0; 
          prevStepSizes[z] = 0;
          internalCounters[z] = 0;
          continue;
        }

        decayCounters[z] += 1; 

        if (decayCounters[z] % (2750 * reverb) == 0){
          internalCounters[z] += 1;
        } 
       phaseAccArray[z] += prevStepSizes[z]; 
       Vout = (phaseAccArray[z] >> 24) >> internalCounters[z];
      } 
      else{
      phaseAccArray[z]= 0;
      }
    }
    else{
      phaseAccArray[z] += currentStepSizes[z];
      Vout = (phaseAccArray[z] >> 24);
    }
    totalVout += Vout;
  }
  totalVout = totalVout >> (8 - knob3Rotation);
  totalVout = min(255, (int) totalVout);
  totalVout = max(0, (int) totalVout);

  if (knob3Rotation == 0){
    totalVout = 0;
  }
  

  
  analogWrite(OUTR_PIN, totalVout);
}