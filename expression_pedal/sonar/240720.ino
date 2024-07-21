#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <AceSorting.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

/* Hardware Connections

        Wemos S2 mini pin 1  -->  HY-SRF05 sonar module pin TRIG
        Wemos S2 mini pin 2 -->  HY-SRF05 sonar module pin ECHO
        Wemos S2 mini pin 33 -->  SSD1306 display pin SDA
        Wemos S2 mini pin 35 -->  SSD1306 display pin SCL
 */

using ace_sorting::shellSortKnuth;

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels

// Declaration for TRIG and ECHO pins of HY-SRF05 sonar module
#define TRIG_PIN 2
#define ECHO_PIN 1

// Declaration for buffer size of sonar return values
#define BUFSIZE 25
unsigned int buf[BUFSIZE];
#define VAL_MIN 500
#define VAL_MAX 2100

// define LIMIT_STEP to limit sensor value change
#define LIMIT_STEP
#define STEP_MAX ((VAL_MAX-VAL_MIN)/6)  // 1/6 of range

// define CC value to send out
#define CC 11
#define CHANNEL 1

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On Wemos S2 mini:        33(SDA), 35(SCL)
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// USB MIDI object
Adafruit_USBD_MIDI usb_midi;

// Create a new instance of the Arduino MIDI Library,
// and attach usb_midi as the transport.
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

void setup() {
  // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  usb_midi.setStringDescriptor("TinyUSB MIDI");

  // Initialize MIDI, and listen to all MIDI channels
  // This will also call usb_midi's begin()
  MIDI.begin(MIDI_CHANNEL_OMNI);

  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  // initialize buf
  for (unsigned int i = 0; i < BUFSIZE; i++) {
    buf[i] = -1;
  }
}

void buf_push(unsigned int num) {
  for (unsigned int i = 0; i < BUFSIZE - 1; i++) {
    buf[i] = buf[i + 1];
  }
  unsigned prev = buf[BUFSIZE - 2];
#ifdef LIMIT_STEP
  if (prev != -1) {
    if (num > prev && num - prev > STEP_MAX) {
      num = prev + STEP_MAX;
    } else if (num < prev && prev - num > STEP_MAX) {
      num = prev - STEP_MAX;
    }
  }
#endif
  buf[BUFSIZE - 1] = num;
}

unsigned int buf_median() {
  unsigned int sorted[BUFSIZE];
  memcpy(sorted, buf, BUFSIZE * sizeof(unsigned int));
  shellSortKnuth(sorted, BUFSIZE);

  unsigned int mid = BUFSIZE / 2;
  return (BUFSIZE & 1) ? sorted[mid] : (sorted[mid] + sorted[mid - 1]) / 2;
}

unsigned int numlen(unsigned int num) {
  if (num > 999)  { return 4; }
  if (num > 99)   { return 3; }
  if (num > 9)    { return 2; }
  return 1;
}

unsigned int out_prev = -1;
void output(unsigned int num, unsigned Vmin, unsigned Vmax) {
  if (num == out_prev) {
    return;
  }
  digitalWrite(LED_BUILTIN, HIGH);
  unsigned int exp = map(num, Vmin, Vmax, 127, 0);         // convert to 7-bit for MIDI
  MIDI.sendControlChange(CC, exp, CHANNEL);
  show_value(num, Vmin, Vmax);
  digitalWrite(LED_BUILTIN, LOW);
  out_prev = num;
}

void show_value(unsigned int num, unsigned Vmin, unsigned Vmax) {
  display.clearDisplay();
  unsigned int bar = map(num, Vmin, Vmax, 0, display.width());
  if (bar) {
    display.fillRect(0, 0, bar, display.height(), WHITE);
  }
  display.setTextSize(2);               // Large 2:1 pixel scale
  display.setTextColor(INVERSE);
  unsigned int d = numlen(max(Vmin, Vmax));
  unsigned int s = d - numlen(num);     // prepend spaces (assume 3 digits)
  display.setCursor((display.width()-d*12)/2+s*12, (display.height()-14)/2);
  display.cp437(true);                  // Use full 256 char 'Code Page 437' font
  display.print(num);
  display.display();
}

void loop() {
  #ifdef TINYUSB_NEED_POLLING_TASK
  // Manual call tud_task since it isn't called by Core's background
  TinyUSBDevice.task();
  #endif

  // not enumerated()/mounted() yet: nothing to do
  if (!TinyUSBDevice.mounted()) {
    return;
  }

  // get distance from sonar
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(20);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH);
  if (duration > 10) {
    buf_push(constrain(duration, VAL_MIN, VAL_MAX));
    output(buf_median(), VAL_MIN, VAL_MAX);
  }
}