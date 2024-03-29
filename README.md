# Sensory Overload Stackable Synth Firmware

## Starter Info

  Use this project as the starting point for your Embedded Systems labs and coursework.
  
  [Lab Part 1](doc/LabPart1.md)
  
  [Lab Part 2](doc/LabPart2.md)

### Additional Information

  [Handshaking and auto-detection](doc/handshaking.md)
  
  [Double buffering of audio samples](doc/doubleBuffer.md)

### Video Demo

The video shows core features, reverb, polyphony, pitch control, and triangle and sine waves as well:\
https://youtu.be/pA1DBCMQ2Lw
  
## General non-technical description

  This is a stackable Synthesiser. Each individual module is configured to work in cooperation with up to 2 other boards. The boards connect to create a full keyboard of up to 3 octaves. The result is a full synth that can play stereo sound out of 3 speackers and act like a normal piano in the sense that it can play any cord, it is not limited to single notes. We have also added some other functionalities you might find in a normal synthesiser. The right-most knob on each keyboard can be used to alter the volume on each individual keyboard, this allows the user to fully customise the sound experience from this keyboard. The second knob from the right will alter the pitch of the whole keyboard. This means that all modules will move octaves acourdingly in order to keep their relative tonality. The third knob is used to alter the reverb on the whole keyboard and make the notes fade away nicely if desired.
Finally, the third knob is used to switch between sawtooth, triangle and sine output waveforms, and thus produce three distinct sounds. The pitch, reverb
and waveform mode knobs can be rotated from any of the three keyboards, and the change is preceived by all keyboards. 

The volume setting can be swiped from 0 to 8.
The reverb level can be swiped from 0 to 2.
The pitch setting can take values from 0 to 2 for the sawtooth and triangle waves (it increases the octave of each keyboard by one) and from 1 to 0 
fot the sine wave (the sinewave ISR sampling rate was smaller and thus couldn't detect some of the frequencies from the highest Octave)

## How to measure Execution Time

1. Switch to PERFORMANCE branch (https://github.com/konsougiou/Synthesiser_Firmware/tree/PERFORMANCE)
2. Remove definition of #ifndef of task to be tested (e.g remove "#define DISABLE_DECODE_THREAD" to test the decodeTask)
3. Flash code to synthesizer
4. Go into serial monitor and press reset button
5. The execution time of the task should print in the serial monitor
6. Make sure to readd definition removed and repeat for other tasks

## Tasks

### keyDetect Task
#### Technical Overview
This task handles two main operations. The primary operation is to read the keyArray in order to determine which keys are being pressed locally. This then will change the currentStepSize according to the keys that are pressed and the local octave. The next thing that is done is to create a new `TX_Message` that contains information on pressed keys from this keyboard. This message is then loaded on to the `msqOutQ` to be transmitted by the CAN transmitter. The format of `TX_Message` is the following:

index | information
---|---
0| message type (0 in this case, representing key press event)
1| First 4 keys
2| Second 4 keys
3| Last 4 keys
4| local Octave
5| -
6| -
7| -

Each key was represented by one bit in the four least significant bits of three `uint32_t`s (indexes 1-3 of the message). If the key was pressed, the corresponding bit was set high, and low otherwise.

A new message was sent only if a key change was detected (at least one pressed <-> unpressed transition).

#### Time Performance
Worst runtime: 110.09 μs \
Minimum initiation interval: 0.2 ms

### Decode Thread
#### Technical Overview
  This task reads the incoming messages from the Receiver. The incoming messages are one of two types. There are messages containing information about keys that have been pressed in other keyboards and there are messages that indicate the rotation of knobs on other keyboards. If the message contains information about keys, then this task will access the `currentStepSize` using a semaphore. Each keyspressed message contains information about the source keyboards octave, and based on this, either the first, middle or last 12 elements of the 36 `uint32_t` sized step sizes array will be populated. This allows the keyboard to play the correct notes at the correct octave. If the received message indicates a knob rotation, it will change the variables containing the current pitch, reverb setting and waveform mode based on changes happening on other keyboards. This happens in order to keep these three 
settings consistent throughout all keyboards.
#### Time Performance
Worst runtime: 24.78 μs\
Minimum initiation interval: 0.2 ms

### Display Thread
#### Technical Overview
This task handles the contents of what gets sent to the OLED display. In particular, it updates the connected display on the synth module with information about the current states/values of the volume, pitch, reverb and the type of wave that is being used (Sine, triangle or Sawtooth). It also has a small indicator that shows the relative position of the synth module compared to the other ones that it is connected to. It also outputs some of the relavant data recieved via CAN messages from other synth modules when a key is pressed/a knob has been rotated.
#### Time Performance
Worst runtime: 14.41 μs\
Minimum initiation interval: 0.8 ms

### Handshake Thread
#### Technical Overview
This task checks the `keyArray` to see whether there has been a detection of a keyboard to the right or the left. Based on this information, each keyboard knows whether it is in the middle, left or right on the keyboard and can infer its respective octave as well. 
#### Time Performance
Worst runtime: 46.28 μs\
Minimum initiation interval: 0.4 ms

### Knob Update Task
#### Technical Overview
This task trasmits information about the rotation of the knobs that control global settings in the form of another `TX_Message` array. This means that it includes information about the pitch, the the waveform setting and the reverb amount. These messages are loaded on the `msgOutQ` and are then transmitted by the CAN transmitter. It is also in charge of calling the `updateRotation()` function for each one of the four knob instances (including volume which wasn't mentioned before since it is local and thus not transimtted). The format of `TX_Message` for this task is the following:

index | information
---|---
0| message type (1 in this case, representing knob rotation event)
1| Reverb
2| Pitch
3| Wave mode
4| -
5| -
6| -
7| -

#### Time Performance
Worst runtime: 11.72 μs\
Minimum initiation interval: 0.4 ms

### Mode Switch Task
#### Technical Overview
This task reads the current waveform mode that is set for all keyboards. This information is given by the Decode Task which reads the CAN messages for the knob states. Based on which waveform should be played, modeSwitch Task will schedule the correct ISR that playes the corresponding tone.
#### Time Performance
Worst runtime: 27.78 μs\
Minimum initiation interval: 0.4 ms

## CAN ISRs

### canRXisr
#### Technical Overview
This ISR gets the message data using the `CAN_RX()` module and then places the data into the queue using `xQueueSendFromISR()`.	The ISR is called whenever a CAN message is received. 
#### Time Performance
Worst runtime: 0 μs

### canTX
#### Technical Overview
This task's sole duty is to obtain a message from the queue and take the semaphore before sending a message.
`this is a TASK, not an ISR`
#### Time Performance
Worst runtime: 0.72 μs

### canTXisr 
#### Technical Overview
This ISR has one purpose and that is to give the semaphore each time a mailbox becomes available. 
#### Time Performance
Worst runtime: 0μs

## waveform ISRs

Different ISRs that output sawtooth, triangle and sine waves were implemented, each producing a distinct sound. They all read from the
currentStepSize array that contains the stepSizes of the notes that are currently pressed in each of its entries (0 otherwise), and perform mathematical
calculations to produce a differnt Vout for each wave. Since these interrupts were executed very frequently, extra caution was put in to making their critical paths more performant. More on this is explained in the individual ISR sections. In addition to reading from the `currentStepSizes` array, they also read the `previousStepSizes` array in order to produce a delay effect which is controlled by one of the knobs and allows the sound to slowly fade.

In all the ISRs the waveforms of all the notes that were played at a particular instance were superimposed in order to produce a polyphony effect.
Additionally, the output voltage was kept between 0 and 255 in order to avoid distortion.

It is also important to mention that each ISR was in charge of outputing a waveform for keys coming from all three keyboards, and thus all 
include a loop that iterates through all 36 keys.

### sawtoothwaveISR
#### Technical Overview

This ISR was activated when the waveform was set to sawtooth. It uses an array of size 36 (`uint32_t`)  to store the 
phase accumulators for every note (12 * 3 for each keyboard). It adds the `currentStepSize` corresponding to each note (if it was being pressed) to each accumulator which overflowed at a given rate, (based on the step size, i.e the note frequency) and thus produced a sawtooth waveform. 
For the delay/reverb effect, the accumulator read the `previousStepSizes` array instead, and the ISR kept a timer of when the key was let go in order to output a slowly diminishing waveform.
As mentioned above, slow operation were avoided inside this ISR, and especially in the loop that iterates over the 36 different notes (i.e current step sizes). For example, the modulo operator was performed using a binary AND, and was only done using powers of two (`x % (2^n) <=> x & (2^n - 1)`).

#### Time Performance
Worst runtime: 16.13 μs


### trianglewaveISR

#### Technical Overview

This ISR was activated when the waveform was set to triangle wave. Its operation is very similar to the sawtooth ISR. There are two main differences.
Firstly, a state was kept for each of the notes (using a 36 `uint32_t` array) which indicated whether its waveform had an upward or a downward slope at the time. Additionally, the current step sizes were multiplied by 2 (using a bitshift) since in order to achieve the same period as the sawtooth wave,
the transition from 0 to 255 had to happen twice as fast (since it also had to go back to 0 in a period).

#### Time Performance
Worst runtime: 15.72 μs

### sinewaveISR
#### Technical Overview
This ISR was activated when the waveform was set to sine wave. This interrupt operates somewhat differntly to the other two. Although it also uses the current and previous step sizes (for delay) arrays, when this waveform mode is active they have been populated by the keyDetect and decode tasks with angular frequencies instead.
These are used to calculate the sine function at each time instance. A static timer was thus used, which was incremented in each call of the interrupt by 0.1 ms (1 over the frequency with which it was called). Initially, both the `"cmath"` `std::sin()` function and a lookup table were tested. The former was too slow to support polyphony, and the latter produced lower quality sound. The implementation that was found to have a good balance of performance and quality was the `"arm_math.h"` library `arm_sin_f32()` function which effectivly utilizes the FPU found in the Arm Cortex-M4 core. This proved performant enough to support both polyphony and delay, as well as smooth output.

#### Time Performance
Worst runtime: 51.22 μs

### Timing Analysis

| Task              | Initiation Interval | Execution Time | RMS Priority | $\lceil \frac{\tau_n}{\tau_i} \rceil$ | $\lceil \frac{\tau_n}{\tau_i} \rceil {T_i}$ | $\frac{T_i}{\tau_i}$ |
|-------------------|---------------------|----------------|--------------|--------------------------------------|-----------------------------------------|------------|
| displayUpdateTask | 100                 | 14.41          | 2            | 1                                    | 14.41                                   | 0.1441%           |
| knobUpdateTask    | 50                  | 11.72          | 3            | 2                                    | 23.44                                   | 0.2344%           |
| handshakeTask     | 50                  | 46.28          | 1            | 2                                    | 92.56                                   | 0.9256%           |
| decodeTask        | 25                  | 24.78          | 6            | 4                                    | 99.12                                   | 0.9912%           |
| modeSwitchTask    | 50                  | 27.78          | 4            | 2                                    | 55.56                                   | 0.5556%           |
| transmitTask      | 25                  | 110.09         | 7            | 4                                    | 440.36                                  | 4.4036%           |
| sawtoothwaveISR   | 1/22                | 16.13          | -            | 2200                                 | 35486                                   | -           |
| sinewaveISR       | 1/10                | 51.22          | -            | 1000                                 | 51220                                   | -           |
| trianglewaveISR   | 1/22                | 15.72          | -            | 2200                                 | 34584                                   | -           |
|                   |                     |                |              | Total                                | 51945μs                                 | 0.51945%          |

## Sharing & security of data

Data structures that were accessed by more that one task or ISR were defined in a globals.cpp file
All `int` and `uint` globals were declared as volatile and were only written and read to uing atomic load and store operations

Global array accesses in tasks were done using semaphores. They were loaded into local arrays inside a semahpore, and stored into again
using a semaphore. When the whole array was going to be loaded/stored into this was all done in a single semaphore, rather than doing it for every item individually. This resulted to less overheads when taking and giving the semaphores.

Some semaphores were used for mulitple accessing mutliple arrays, since some arrays are very commonly accessed simultaneously in the same loops.

The following is a list of all the shared variables and data structures that we used, along with any semaphore that was used for safe acces and modification:

When it came to the knob instances, all methods perform all their internal operations atomically, so there was no need for atomic operations or mutexes when calling their methods in different tasks.

Thread safe queues from FreeRTOS:\
QueueHandle_t msgInQ;\
QueueHandle_t msgOutQ;

uint8_t volume;\
uint8_t pitch;\
uint8_t mode;\
uint8_t reverb;\
uint8_t westDetect;\
uint8_t eastDetect;\
uint8_t localOctave;\
bool middleKeyboardFound;

\
uint8_t RX_Message[8];\
\
SemaphoreHandle_t keyArrayMutex:\
  uint8_t keyArray[7];

SemaphoreHandle_t stepSizesMutex:\
  uint32_t currentStepSizes[36];\
  uint32_t prevStepSizes[36];\
  uint32_t decayCounters[36];\
  uint32_t internalCounters[36];\
  uint32_t stepSizes[12];


SemaphoreHandle_t queueReceiveMutex:\
  uint8_t RX_Message[8];


SemaphoreHandle_t CAN_TX_Semaphore:\
  Counting semaphore that allowed up to three owners. Given by CAN_TX_ISR when a mailbox becomes
  available, and taken by CAN_TX_Task right before transmission.




