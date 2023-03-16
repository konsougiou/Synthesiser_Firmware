#include "ISRs.hpp"
#include "../globals.hpp"
// #include <fastmath.h>

void sinewaveISR()
{
  static uint32_t phaseAcc = 0;
  static uint32_t Vouts[36] = {0};
  static double time = 0;
  //static double timers[12] = {0};


  int32_t totalVout = 0;
  int32_t Vout;
  int32_t scaledIndex;
  int32_t index;
  int32_t periodCounter;
  int32_t freq;

  //phaseAcc += currentStepSizes[4];
  //Vout = (phaseAcc >> 24);
  //totalVout += Vout;  
  for (int i = 0; i < 36; i++){
    Vout = 0;
    if (currentStepSizes[i]){
      // scaledIndex = indexScaling[i]; 
      // periodCounter = periodCounters[i];
      freq = currentStepSizes[i];
      // index = periodCounter/scaledIndex;
      // if(i == 0){
      //     currentStepSizes[0] == index;
      // }
      //Vout = amplifiedSineFunction[index];
      double x = time * freq;
      //Vout = (int) (100.0 * x - 16.0 * pow(x, 3)  + 100); //+ 0.8 * pow(x, 5) + 100.0));
      totalVout =+ (int) (127.0 * (sin(x) + 1.0));
      break;
    }
    // if(timers[i] == periods[i]){
    //     timers[i] = 0;
    // } 
    // else{
    //     timers[i] += 0.0002;
    // }
    
  }
  totalVout = totalVout >> (8 - knob3Rotation);
  totalVout = min(255, (int) totalVout);
  totalVout = max(0, (int) totalVout);

  if (knob3Rotation == 0){
    totalVout = 0;
  }
  time += 0.0002;

  analogWrite(OUTR_PIN, totalVout);
}