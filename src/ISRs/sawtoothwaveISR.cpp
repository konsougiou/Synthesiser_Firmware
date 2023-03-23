
#include "ISRs.hpp"
#include "../globals.hpp"

void sawtoothwaveISR()
{
  static uint32_t phaseAccArray[36] = {0};
  int32_t totalVout = 0;
  int32_t Vout;
  for(int z=0;z<36;z++){
    Vout = 0;
    if(currentStepSizes[z]==0){
      if(prevStepSizes[z] != 0 && reverb){
        if (decayCounters[z] == 16384){
          decayCounters[z] = 0; 
          prevStepSizes[z] = 0;
          internalCounters[z] = 0;
          continue;
        }

        decayCounters[z] += 1; 
        // using bitwise AND instead of modulo since "reverb << 11" is a power of 2
        if ((decayCounters[z] & ((reverb << 11) - 1)) == 0){
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
      phaseAccArray[z] += currentStepSizes[z];
      Vout = (phaseAccArray[z] >> 24);
    }
    totalVout += Vout;
  }
  totalVout = totalVout >> (8 - volume);
  totalVout = min(255, (int) totalVout);
  totalVout = max(0, (int) totalVout);

  if (volume == 0){
    totalVout = 0;
  }
  
  analogWrite(OUTR_PIN, totalVout);
}
