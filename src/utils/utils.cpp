
#include "../globals.hpp"

void setRow(uint8_t rowIdx)
{
  digitalWrite(REN_PIN, LOW);

  uint8_t msb = (rowIdx % 8) >> 2;
  uint8_t middle_bit = (rowIdx % 4) >> 1;
  uint8_t lsb = rowIdx % 2;

  digitalWrite(RA0_PIN, lsb);
  digitalWrite(RA1_PIN, middle_bit);
  digitalWrite(RA2_PIN, msb);

  digitalWrite(REN_PIN, HIGH);
}

uint8_t readCols()
{

  uint8_t C0_read = digitalRead(C0_PIN);
  uint8_t C1_read = digitalRead(C1_PIN);
  uint8_t C2_read = digitalRead(C2_PIN);
  uint8_t C3_read = digitalRead(C3_PIN);

  return (C3_read << 3) + (C2_read << 2) + (C1_read << 1) + C0_read;
}

void setOutMuxBit(const uint8_t bitIdx, const bool value)
{
  digitalWrite(REN_PIN, LOW);
  digitalWrite(RA0_PIN, bitIdx & 0x01);
  digitalWrite(RA1_PIN, bitIdx & 0x02);
  digitalWrite(RA2_PIN, bitIdx & 0x04);
  digitalWrite(OUT_PIN, value);
  digitalWrite(REN_PIN, HIGH);
  delayMicroseconds(2);
  digitalWrite(REN_PIN, LOW);
}