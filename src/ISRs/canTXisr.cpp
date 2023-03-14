
#include "ISRs.hpp"
#include "../globals.hpp"

void CAN_TX_ISR(void)
{
  xSemaphoreGiveFromISR(CAN_TX_Semaphore, NULL);
}