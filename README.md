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
  This task reads the incoming messages from the Receiver. The incoming messages are one of two types. There are messages containing information about keys that have been pressed in other keyboards and there are messages that indicate the rotation of knobs on other keyboards. If the message contains information about keys, then this task will access the 'currentStepSize' using a semaphore. It is worth noting that each keyspressed message contains information about the source's octave. This allows the keyboard to play the correct notes at the correct octave. Otherwise it will change the variables containing the current pitch, reverb setting and waveform mode.
#### Time Performance

### Display Thread
#### Technical Overview
This task handles the contents of what gets sent to the OLED display. In particular, it updates the connected display on the synth module with information about the current states/values of the volume, pitch, reverb and the type of wave that is being used (Sine, triangle or Sawtooth). It also has a small indicator that shows the relative position of the synth module compared to the other ones that it is connected to. It also outputs some of the relavant data recieved via CAN messages from other synth modules when a key is pressed/a knob has been rotated.
#### Time Performance

### Handshake Thread
#### Technical Overview
This task checks the keyArray to see whether there has been a detection of a keyboard to the right or the left. Based on this information, each keyboard knows whether it is in the middle, left or right on the keyboard and can infer its respective octave as well. 
#### Time Performance

### Knob Update Task
#### Technical Overview
This task trasmits information about the rotation of the knobs that control global settings. This means that it includes information about the pitch, the the waveform setting and the reverb amount. These messages are loaded on the 'msgOutQ' and are then transmitted by the CAN transmitter. 
#### Time Performance

### Mode Switch Task
#### Technical Overview
This task reads the current waveform mode that is set for all keyboards. This information is given by the Decode Task which reads the CAN messages for the knob states. Based on which waveform should be played, modeSwitch Task will schedule the correct ISR that playes the corresponding tone.
#### Time Performance

### Transmit Task
#### Technical Overview
This task handles a multitude of operations. The primary operation is to read the keyArray in order to determine which keys are being pressed locally. This then will change the currentStepSize according to the keys that are pressed and the local octave. The next thing that is done is to create a new 'TX_Message' that contains information on pressed keys from this keyboard. This message is then loaded on to the 'msqOutQ' to be transmitted by the CAN transmitter. 
#### Time Performance

## ISRs

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

### sawtoothwaveISR
#### Technical Overview
#### Time Performance

### sinewaveISR
#### Technical Overview
#### Time Performance

### trianglewaveISR
#### Technical Overview
#### Time Prformance

## Knob Class
### Description

## Other technical analysis

### Rate monotonic scheduler

### Total CPU utilisation

## Sharing & security of data

### Shared data structures & access methods

### Data access analysis & assessment of deadlock possibility
