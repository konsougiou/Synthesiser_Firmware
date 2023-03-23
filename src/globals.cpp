
#include "globals.hpp"

TIM_TypeDef *Instance1 = TIM1;
HardwareTimer *sawtoothwaveSampleTimer = new HardwareTimer(Instance1);

TIM_TypeDef *Instance2 = TIM2;
HardwareTimer *sinewaveSampleTimer = new HardwareTimer(Instance2);

TIM_TypeDef *Instance3 = TIM15;
HardwareTimer *trianglewaveSampleTimer = new HardwareTimer(Instance3);

const uint32_t interval = 100; // Display update interval
const double frequency_ratio = pow(2.0, 1.0 / 12.0);
const uint32_t step_scaling = pow(2, 32) / 22000;

const uint32_t stepSizes[12] = {25534599, 27051354, 28658204, 30360502, 32163915, 34074452, 36098475, 38242724, 40514342, 42920894, 45470395, 48171336};

const uint32_t note_frequencies[12] = {1617, 1713, 1815, 1923, 2037, 2158, 2286, 2422, 2566, 2719, 2880, 3051};
// double periods[12] = {0.00382021, 0.00360601, 0.00340382, 0.00321297, 0.00303282, 0.00286277, 0.00270226, 0.00255075, 0.00240773, 0.00227273, 0.0021453, 0.00202501};

volatile uint8_t localOctave = 4;

volatile uint32_t currentStepSize = 0;

volatile uint32_t activeKeys[] = {0};
uint32_t currentStepSizes[36] = {0};

// THIS IS CONST INSTEAD OF VOLATILE MIGHT CAUSE PROBLEMS???
const char *globalKeySymbol = 0;

volatile uint8_t keyArray[7];

QueueHandle_t msgInQ;
QueueHandle_t msgOutQ;

uint32_t prevStepSizes[36] = {0};

uint8_t RX_Message[8] = {0};
uint8_t TX_Message[8] = {0};

volatile uint8_t westDetect;
volatile uint8_t eastDetect;

SemaphoreHandle_t keyArrayMutex;
SemaphoreHandle_t queueReceiveMutex;
SemaphoreHandle_t stepSizesMutex;
SemaphoreHandle_t decodeStepSizesMutex;

SemaphoreHandle_t CAN_TX_Semaphore;

bool pressOrReceive = false; // False == Receive, True == Press

volatile uint8_t volume = 6;

volatile uint8_t pitch = 0;

volatile uint8_t reverb = 0;

volatile uint8_t mode = 0;

volatile bool middleKeyboardFound = false;

volatile bool moreThanThree = false;

uint32_t decayCounters[36] = {0};
uint32_t internalCounters[36] = {0};

knob *knob3 = new knob(3); // Knobs methods are atomic internally so no need for sephamore / atomic operations when using these instances KS for da report.
knob *knob2 = new knob(2);
knob *knob1 = new knob(1);
knob *knob0 = new knob(0);

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
