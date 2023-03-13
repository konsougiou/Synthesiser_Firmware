
#include "globals.hpp"

const uint32_t interval = 100; // Display update interval
const double frequency_ratio = pow(2.0, 1.0 / 12.0);
const uint32_t step_scaling = pow(2, 32) / 22000;

const uint32_t stepSizes[] = {25534599,27051354,28658204,30360502,32163915,34074452,36098475,38242724,40514342,42920894,45470395,48171336};
                              
volatile uint32_t freqs [] = {2000, 5000, 10000, 17000, 22000, 28000, 30000, 40000, 50000};

volatile uint8_t localOctave = 4;

volatile uint32_t currentStepSize = 0;

volatile uint8_t keyArray[7];

QueueHandle_t msgInQ;
QueueHandle_t msgOutQ;

uint32_t currentStepSizes[36] = {0};
uint32_t prevStepSizes[36] = {0};

uint8_t RX_Message[8] = {0};
uint8_t TX_Message[8] = {0};

uint8_t westDetect;
uint8_t eastDetect;

SemaphoreHandle_t keyArrayMutex;
SemaphoreHandle_t queueReceiveMutex;
SemaphoreHandle_t currentStepSizesMutex;
SemaphoreHandle_t decodeStepSizesMutex;

SemaphoreHandle_t CAN_TX_Semaphore;

bool pressOrReceive = false; // False == Receive, True == Press

uint8_t knob3Rotation = 6;

uint8_t knob2Rotation = 0;

uint8_t reverb = 2;

uint32_t decayCounters[36] = {0};
uint32_t internalCounters[36] = {0};

knob *knob3 = new knob(3);
knob *knob2 = new knob(2);
knob *knob1 = new knob(1);

// Pin definitions
// Row select and enable
const int RA0_PIN = D3;
const int RA1_PIN = D6;
const int RA2_PIN = D12;
const int REN_PIN = A5;

// Matrix input and output
const int C0_PIN = A2;
const int C1_PIN = D9;
const int C2_PIN = A6;
const int C3_PIN = D1;
const int OUT_PIN = D11;

// Audio analogue out
const int OUTL_PIN = A4;
const int OUTR_PIN = A3;

// Joystick analogue in
const int JOYY_PIN = A0;
const int JOYX_PIN = A1;

// Output multiplexer bits
const int DEN_BIT = 3;
const int DRST_BIT = 4;
const int HKOW_BIT = 5;
const int HKOE_BIT = 6;

// Display driver object
U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);