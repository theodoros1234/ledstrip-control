# ledstrip-control
This is some code I wrote in 2020 to control the LED strip I bought during the pandemic. It's not by any means tidy, since I put it together quickly and I intend to re-write it from scratch.

This repository contains:
- Arduino firmware that controls a non-addressible LED strip by receiving commands from a serial connection
- Bash script that sends receives a parameter and sends it to the Arduino
- Python script that monitors a PulseAudio source and changes the LED strip brightness accordingly (to make the LED strip flash with music)
- C++ code that changes the color of the LED strip depending on what's displayed on the screen
