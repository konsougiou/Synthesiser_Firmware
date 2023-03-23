
#include "ISRs.hpp"
#include "../globals.hpp"

void CAN_RX_ISR(void)
{
  uint8_t RX_Message_ISR[8];
  uint32_t ID;
  // CAN_RX(ID, RX_Message_ISR);
  // xQueueSendFromISR(msgInQ, RX_Message_ISR, NULL);
}