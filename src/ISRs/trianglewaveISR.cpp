
#include "ISRs.hpp"
#include "../globals.hpp"

void trianglewaveISR()
{
  static uint32_t phaseAccArray[36] = {0};
  static uint32_t Vouts[36] = {0};
  static uint8_t slopeSigns[36] = {1};
  uint32_t newPhaseAcc;
  int32_t totalVout = 0;
  int32_t Vout;
  int8_t slopeSign;


  for(int z=0;z<36;z++){
    Vout = 0;
    if(currentStepSizes[z]==0){
      if(prevStepSizes[z] != 0 && reverb){
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
       Vout = (phaseAccArray[z] >> 25) >> internalCounters[z];
      } 
      else{
      phaseAccArray[z]= 0;
      }
    }
    else{
      slopeSign = slopeSigns[z];
      if(slopeSign == 1){                     // Positive slope
        newPhaseAcc = phaseAccArray[z] + (currentStepSizes[z]);
        if (newPhaseAcc < phaseAccArray[z]) { // If we're about to overflow the uint32_t, then we switch slope direction downwards and continue
        slopeSigns[z] = -1;                   // Change the direction of the slope (to negative)
        phaseAccArray[z] = 4294967295;        // is equal to 0xFFFFFFFF
        }
        else{
        phaseAccArray[z] = newPhaseAcc;
        }
      }
      else{                                   //Negative slope
        newPhaseAcc = phaseAccArray[z] - (currentStepSizes[z]); 
        if (newPhaseAcc > phaseAccArray[z]) { // If we're about to underflow the uint32_t, then we switch slope direction upwards and continue
        slopeSigns[z] = 1;                    // Change the direction of the slope (to positive)
        phaseAccArray[z] = 0;                 
        }
        else{
        phaseAccArray[z] = newPhaseAcc;
        }
      }
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