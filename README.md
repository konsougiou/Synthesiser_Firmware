# Sensory Overload Stackable Synth Firmware

## Starter Info

  Use this project as the starting point for your Embedded Systems labs and coursework.
  
  [Lab Part 1](doc/LabPart1.md)
  
  [Lab Part 2](doc/LabPart2.md)

### Additional Information

  [Handshaking and auto-detection](doc/handshaking.md)
  
  [Double buffering of audio samples](doc/doubleBuffer.md)
  
## General non-technical description

  This is a stackable Synthesiser. Each individual module is configured to work in cooperation with up to 2 other boards. The boards connect to create a full keyboard of up to 3 octaves. The result is a full synth that can play stereo sound out of 3 speackers and act like a normal piano in the sense that it can play any cord, it is not limited to single notes. We have also added some other functionalities you might find in a normal synthesiser. The right-most knob on each keyboard can be used to alter the volume on each individual keyboard, this allows the user to fully customise the sound experience from this keyboard. The second knob from the right will alter the pitch of the whole keyboard. This means that all modules will move octaves acourdingly in order to keep their relative tonality. Finally, the third knob is used to alter the reverb on the whole keyboard and make the notes fade away nicely if desired.  

## Tasks

### Decode Thread
#### Technical Overview
  This task reads the incoming messages from the Receiver. The incoming messages are one of two types. There are messages containing information about keys that have been pressed in other keyboards and there are messages that indicate the rotation of knobs on other keyboards. If the message contains information about keys, then this task will access the `currentStepSize` using a semaphore. It is worth noting that each keyspressed message contains information about the source's octave. This allows the keyboard to play the correct notes at the correct octave. Otherwise it will change the variables containing the current pitch, reverb setting and waveform mode.
#### Time Performance

### Display Thread
#### Technical Overview
This task handles the contents of what gets sent to the OLED display. In particular, it updates the connected display on the synth module with information about the current states/values of the volume, pitch, reverb and the type of wave that is being used (Sine, triangle or Sawtooth). It also has a small indicator that shows the relative position of the synth module compared to the other ones that it is connected to. It also outputs some of the relavant data recieved via CAN messages from other synth modules when a key is pressed/a knob has been rotated.
#### Time Performance

### Handshake Thread
#### Technical Overview
This task checks the `keyArray` to see whether there has been a detection of a keyboard to the right or the left. Based on this information, each keyboard knows whether it is in the middle, left or right on the keyboard and can infer its respective octave as well. 
#### Time Performance

### Knob Update Task
#### Technical Overview
This task trasmits information about the rotation of the knobs that control global settings. This means that it includes information about the pitch, the the waveform setting and the reverb amount. These messages are loaded on the `msgOutQ` and are then transmitted by the CAN transmitter. 
#### Time Performance

### Mode Switch Task
#### Technical Overview
This task reads the current waveform mode that is set for all keyboards. This information is given by the Decode Task which reads the CAN messages for the knob states. Based on which waveform should be played, modeSwitch Task will schedule the correct ISR that playes the corresponding tone.
#### Time Performance

### Transmit Task
#### Technical Overview
This task handles a multitude of operations. The primary operation is to read the keyArray in order to determine which keys are being pressed locally. This then will change the currentStepSize according to the keys that are pressed and the local octave. The next thing that is done is to create a new `TX_Message` that contains information on pressed keys from this keyboard. This message is then loaded on to the `msqOutQ` to be transmitted by the CAN transmitter. 
#### Time Performance

## CAN ISRs

### canRXisr
#### Technical Overview
This ISR gets the message data using the `CAN_RX()` module and then places the data into the queue using `xQueueSendFromISR()`.	The ISR is called whenever a CAN message is received. 
#### Time Performance

### canTX
#### Technical Overview
This task's sole duty is to obtain a message from the queue and take the semaphore before sending a message.
`this is a TASK, not an ISR`
#### Time Performance

### canTXisr 
#### Technical Overview
This ISR has one purpose and that is to give the semaphore each time a mailbox becomes available. 
#### Time Performance

## waveform ISRs

Different ISRs that output sawtooth, triangle and sine waves were implemented, each producing a distinct sound. They all read from the
currentStepSize array that contains the stepSizes of the notes that are currently pressed in each of its entries (0 otherwise), and perform mathematical
calculations to produce a differnt Vout for each wave. Since these interrupts were executed very frequently, extra caution was put in to making their critical paths more performant. More on this is explained in the individual ISR sections. In addition to reading from the `currentStepSizes` array, they also read the `previousStepSizes` array in order to produce a delay effect which is controlled by one of the knobs and allows the sound to slowly fade.

In all the ISRs the waveforms of all the notes that were played at a particular instance were superimposed in order to produce a polyphony effect.
Additionally, the output voltage was kept between 0 and 255 in order to avoid distortion.

### sawtoothwaveISR
#### Technical Overview

This ISR was activated when the waveform was set to sawtooth. It uses an array of size 36 (`uint32_t`)  to store the 
phase accumulators for everyone note (12 * 3 for each keyboard). It added the `currentStepSize` corresponding to each note (if it was being pressed) to each accumulator which overflowed at a given rate, (based on the step size, i.e the note frequency) and thus produced a sawtooth waveform. 
For the delay/reverb effect, the accumulator read the `previousStepSizes` array instead, and the ISR kept a timer of when the key was let go in order to output a slowly diminishing waveform.
As mentioned above, slow operation were avoided inside this ISR, and especially in the loop that iterates over the 36 different notes (i.e current step sizes). For example, the modulo operator was performed using a binary AND, and was only done using powers of two (`x % 64 <=> x & (64 - 1)`).

#### Time Performance

### trianglewaveISR

#### Technical Overview

This ISR was activated when the waveform was set to triangle wave. Its operation is very similar to the sawtooth ISR. There are two main differences.
Firstly, a state was kept for each of the notes (using a 36 `uint32_t` array) which indicated whether its waveform had an upward or a downward slope at the time. Additionally, the current step sizes were multiplied by 2 (using a bitshift) since in order to achieve the same period as the sawtooth wave,
the transition from 0 to 255 had to happen twice as fast (since it also had to go back to 0 in a period).

#### Time Prformance

### sinewaveISR

This ISR was activated when the waveform was set to sine wave. This interrupt operates somewhat differntly to the other two. Although it also uses the current and previous step sizes (for delay) arrays, when this waveform mode is active they have been populated by the keyDetect and decode tasks with angular frequencies instead.
These are used to calculate the sine function at each time instance. A static timer was thus used, which was incremented in each call of the interrupt by 0.1 ms (1 over the frequency with which it was called). Initially, both the `"cmath"` `std::sin()` function and a lookup table were tested. The former was too slow to support polyphony, and the latter produced lower quality sound. The implementation that was found to have a good balance of performance and quality was the `"arm_math.h"` library `arm_sin_f32()` function which effectivly utilizes the FPU found in the Arm Cortex-M4 core. This proved performant enough to support both polyphony and delay, as well as smooth output.

#### Technical Overview
#### Time Performance

## Knob Class
### Description

## Other technical analysis

### Rate monotonic scheduler

### Total CPU utilisation

## Sharing & security of data

### Shared data structures & access methods

### Data access analysis & assessment of deadlock possibility
