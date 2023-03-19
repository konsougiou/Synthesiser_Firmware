#include "ISRs.hpp"
#include "../globals.hpp"

void trianglewavev2ISR()
{
  static uint32_t phaseAccArray[36] = {0};
  static uint32_t Vouts[36] = {0};
  static uint8_t slopeSigns[36] = {1};
  static double timers[36] = {0};
  uint32_t newPhaseAcc;
  int32_t freq;
  int32_t totalVout = 0;
  int32_t Vout;
  int8_t slopeSign;
  int32_t x;


  for(int z=0;z < 36 ;z++){
    Vout = 0;
    if(currentStepSizes[z]){
      slopeSign = slopeSigns[z];
      if(slopeSign == 1){                     // Positive slope
        freq = 40.7 * currentStepSizes[z];
        x = timers[z] * freq;
        if (x > 255){ 
            slopeSigns[z] = -1;
            timers[z] = 0.0001;
            x = 255; 
        }
        else{
            timers[z] += 0.0001;
        }
      }
      else{                                   //Negative slope
        freq = 40.7 * currentStepSizes[z]; 
        x = 255 - (timers[z] * freq);
        if (x < 0){ 
            slopeSigns[z] = 1;
            timers[z] = 0.0001;
            x = 0;
        } else{
            timers[z] += 0.0001;
        }
      }
      Vout = x;
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