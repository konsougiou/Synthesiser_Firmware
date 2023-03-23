# ES-synth-starter

  Use this project as the starting point for your Embedded Systems labs and coursework.
  
  [Lab Part 1](doc/LabPart1.md)
  
  [Lab Part 2](doc/LabPart2.md)

## Additional Information

  [Handshaking and auto-detection](doc/handshaking.md)
  
  [Double buffering of audio samples](doc/doubleBuffer.md)

## Execution Times (worse-case in microsecs)

  knobUpdate - 255
  handshake - 1493
  decode - 771
  modeSwitch - 862
  keyDetect - 3562
  display - 444 (589792 w any u8g2 stuff, so had to remove em)
  canTx - 5
  

  sawtoothwaveISR - 755
  sinewaveISR - 836
  trianglewaveISR - 839
  canRXisr - 5
  canTXisr - 5



