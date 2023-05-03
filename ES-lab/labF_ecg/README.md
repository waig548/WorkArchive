# Lab Final ECG

The main functions of this program includes:
* Recording of ECG with variable sampling frequency (saves raw data as a file on a SD card)
* Real time heart beat detection and heart rate estimation (outputs to an OLED display)

This is essentially a big mix of previous labs, you can see part of my first try of tackling this lab in [prototype](./prototype) folder. I thought I would need 2 boards to complete this, then I found that using 2 boards seems to be troublesome (some how you can't use I2C with SPI at the same time), so I had to make do with only one board, but thanksfully all parts appears to be working in harmony, no port collisions, inteferences, etc. There was a slight issue with the heart beat detection part, I wasn't able to make it tune the beat threshold by itself (and I didn't have the board and required parts with me while writing this program), I had to tune it with my team mates with live experiments. But overall, I think it turned out nicely.
