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

## List of Functionalities

* ScanKeys
* DecodeTask
* SampleISR
* Display
