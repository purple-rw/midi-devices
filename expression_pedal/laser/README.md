## MIDI expression pedal using laser-ranging sensor

# Usage
- Expression value (CC 11) is sent out to Channel 1 according to object distance from the time-of-flight laser-ranging sensor.  Range is from about 5cm to 45cm above the sensor.

# Components
1. Waveshare RP2040-Zero
2. STMicro VL53L0x module

# Connections
- RP2040-Zero pin 5V   -->  VL53L0x pin VIN
- RP2040-Zero pin GND  -->  VL53L0x pin GND
- RP2040-Zero pin 4    -->  VL53L0x pin SDA
- RP2040-Zero pin 5    -->  VL53L0x pin SCL
