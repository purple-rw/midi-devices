/*

   Example for MIDI expression pedal USB adapter

   Created: 18 Dec 2022

   Author: Rhys Wong <purple_rw@yahoo.com>

*/

#include <MIDIUSB_Defs.h>
#include <MIDIUSB.h>

const int expressionChannel = 0;   // Channel 1
const int expressionCC      = 11;  // Expression CC
const int expressionPin     = 0;   // A0 pin
word expression = 0;

void setup() {
}

void loop() {
  
  word prev = expression;
  expression = (byte) (analogRead(expressionPin) >> 3);

  // send MIDI CC if value changed
  if (expression != prev) {
    controlChange(expressionChannel, expressionCC, expression);
    MidiUSB.flush();
  }
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}
