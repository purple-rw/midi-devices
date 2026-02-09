## MIDI expression pedal TRS to USB adaapter

# Usage
- Connect an expression pedel with TRS jack (e.g. Yamaha FC7 or Korg EXP2) to USB host.
- Expression value (CC 11) is sent out to Channel 1.

# Components
1. Any board with AVR core, SAM core, or SAMD core. (e.g. Arduino UNO R3, Pro Micro ATMega32U4, Seeed XIAO SAMD21)
2. Female TRS jack

# Connections
- TRS tip     -->  Board pin 5V
- TRS ring    -->  Board pin A0
- TRS sleeve  -->  Board pin GND
