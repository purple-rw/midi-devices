## MIDI expression pedal using sonar

# Usage
- Expression value (CC 11) is sent out to Channel 1 according to object distance from the sonar sensor.  Range is from about 9cm to 32cm above the sensor.

# Components
1. Wemos ESP32-S2 mini
2. SSD1306 LED display (128x32)
3. HY-SRF05 sonar module

# Connections
- Wemos S2 mini pin 1  -->  HY-SRF05 sonar module pin TRIG
- Wemos S2 mini pin 2  -->  HY-SRF05 sonar module pin ECHO
- Wemos S2 mini pin 33 -->  SSD1306 display pin SDA
- Wemos S2 mini pin 35 -->  SSD1306 display pin SCL
