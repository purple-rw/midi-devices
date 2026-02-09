#include <Adafruit_VL53L0X.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

/* Hardware Connections
        RP2040 Zero pin 4 SDA -->  VL53L0 pin SDA
        RP2040 Zero pin 5 SCL -->  VL53L0 pin SCL
 */

#define VAL_MIN   50                    // 50mm above sensor
#define VAL_MAX  450                    // 450mm above sensor
#define STEP_MAX ((VAL_MAX-VAL_MIN)/5)  // limit value to 1/5 of range

#define CC 11                           // MIDI continuous control number
#define CHANNEL 1                       // MIDI output channel

Adafruit_VL53L0X laser = Adafruit_VL53L0X();                    // instantiate the time of flight distance sensor
Adafruit_USBD_MIDI usb_midi;                                    // instantiate MIDI over USB transport
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);       // instantiate MIDI object and attach usb_midi as the transport

void setup() {
  if (!TinyUSBDevice.isInitialized()) TinyUSBDevice.begin(0);   // needed by core without built-in support e.g. mbed rp2040
  usb_midi.setStringDescriptor("TinyUSB MIDI");                 // set MIDI descriptor
  MIDI.begin(MIDI_CHANNEL_OMNI);                                // initialize MIDI and listen to all channels; also call usb_midi.begin()

  if (!laser.begin()) for (;;) ;        // loop forever if init failed
  laser.startRangeContinuous();         // set time of flight sensor for continuous measurement
}

uint16_t smooth(uint16_t num) {
  static uint16_t prev = -1;
  if (prev != (uint16_t)-1) {
    if (num > prev && num - prev > STEP_MAX) num = prev + STEP_MAX;
    if (num < prev && prev - num > STEP_MAX) num = prev - STEP_MAX;
  }
  return prev = num;
}

void loop() {
#ifdef TINYUSB_NEED_POLLING_TASK
  TinyUSBDevice.task();                 // manually call if not done by core's background
#endif
  if (!TinyUSBDevice.mounted()) return; // do nothing if not enumerated/mounted

  static uint16_t prev = -1;                                    // previous 7-bit value
  if (laser.isRangeComplete()) {
    uint16_t dist = laser.readRange();                          // get distance reading
    dist = smooth(constrain(dist, VAL_MIN, VAL_MAX));           // smooth out input value
    uint16_t val = map(dist, VAL_MIN, VAL_MAX, 127, 0);         // convert to 7-bit for MIDI
    if (val != prev) MIDI.sendControlChange(CC, val, CHANNEL);  // send out MIDI CC if value changed
    prev = val;
  }
}
