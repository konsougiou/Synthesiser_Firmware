#include "ISRs.hpp"
#include "../globals.hpp"
#include "arm_math.h"

void sinewaveISR()
{
  static uint32_t phaseAcc = 0;
  static uint32_t Vouts[36] = {0};
  static float32_t timer = 0;

  // static double timers[12] = {0};

  int32_t totalVout = 0;
  int32_t Vout;
  int32_t scaledIndex;
  int32_t index;
  int32_t periodCounter;
  int32_t freq;
  float32_t theta;
  uint8_t keyCounter = 0;
  uint8_t volCorrector = 8;

  // phaseAcc += currentStepSizes[4];
  // Vout = (phaseAcc >> 24);
  // totalVout += Vout;
  for (int i = 0; i < 36; i++)
  {
    Vout = 0;
    if (currentStepSizes[i] == 0)
    {
      if (prevStepSizes[i] != 0 && reverb)
      {
        keyCounter++;
        if (decayCounters[i] == 4096)
        {
          decayCounters[i] = 0;
          prevStepSizes[i] = 0;
          internalCounters[i] = 0;
          continue;
        }
        // volCorrector = 6;
        decayCounters[i] += 1;
        // using bitwise AND instead of modulo since "reverb << 8" is a power of 2
        if ((decayCounters[i] & ((reverb << 8) - 1)) == 0)
        {
          internalCounters[i] += 1;
        }
        freq = prevStepSizes[i];
        theta = timer * freq;

        // If vol=8 and more than >3 pressed,will be 5. If vol=7 and >5 keys pressed will be 5.
        totalVout += (int32_t)((1 << (6 - internalCounters[i])) * (arm_sin_f32(theta) + 1.0));
      }
    }
    else
    {
      keyCounter++;
      // volCorrector = 7;
      //  scaledIndex = indexScaling[i];
      //  periodCounter = periodCounters[i];
      freq = currentStepSizes[i];
      // index = periodCounter/scaledIndex;
      // if(i == 0){
      //     currentStepSizes[0] == index;
      // }
      // Vout = amplifiedSineFunction[index];
      theta = timer * freq;
      // Vout = (int) (100.0 * x - 16.0 * pow(x, 3)  + 100); //+ 0.8 * pow(x, 5) + 100.0));
      // totalVout += (int) (127.0 * (sin(theta) + 1.0));

      // If vol=8 and more than >3 pressed,will be 6. If vol=7 and >5 keys pressed will be 6.
      totalVout += (int32_t)((1 << 7) * (arm_sin_f32(theta) + 1.0)); //+1 for .+0 for
      // break;
      // count ++;
      // if(count > 3) break;
    }
    // if(timers[i] == periods[i]){
    //     timers[i] = 0;
    // }
    // else{
    //     timers[i] += 0.0002;
    // }
  }
  if ((volume == 8 && keyCounter >= 3) || (volume == 7 && keyCounter >= 4))
  {
    volCorrector = 9;
    // volCorrector = 1;
  }
  totalVout = totalVout >> (volCorrector - volume);
  // totalVout = totalVout >> (8 - volume);
  // totalVout = totalVout >> (8 - volume - volCorrector);
  totalVout = min(255, (int)totalVout);
  totalVout = max(0, (int)totalVout);

  if (volume == 0)
  {
    totalVout = 0;
  }
  timer += 0.0001;

  analogWrite(OUTR_PIN, totalVout);
}