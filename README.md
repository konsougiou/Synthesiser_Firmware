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
  This Task reads the incoming messages from the Receiver. The incoming messages are one of two types. There are messages containing information about keys that have been pressed in other keyboards and there are messages that indicate the rotation of knobs on other keyboards. If the message contains information about keys, then this task will access the currentStepSize using a semaphore. It is worth noting that each keyspressed message contains information about the source's octave. This allows the keyboard to play the correct notes at the correct octave. Otherwise it will change the variables containing the current pitch, reverb setting and waveform mode.
#### Time Performance

### Display Thread
#### Technical Overview
#### Time Performance

### Handshake Thread
  This Task 
#### Technical Overview
#### Time Performance

### Knob Update Task
#### Technical Overview
#### Time Performance

### Mode Switch Task
#### Technical Overview
#### Time Performance

### Transmit Task
#### Technical Overview
#### Time Performance

## ISRs

### canRXisr
#### Technical Overview
#### Time Performance

### canTX
#### Technical Overview
#### Time Performance

### canTXisr 
#### Technical Overview
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

## Other technical analysis

### Rate monotonic scheduler

### Total CPU utilisation

## Sharing & security of data

### Shared data structures & access methods

### Data access analysis & assessment of deadlock possibility
