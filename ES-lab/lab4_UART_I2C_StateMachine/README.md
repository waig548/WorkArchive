# Lab4 UART I2C State Machine

This lab requires 2 devices, one is the master device, which is connected to a computer terminal via UART, the other is the slave, which is a state machine connected to the master via I2C.

Users can send signals to the master from the terminal, and it passes them to the slave device, then relays state change signals from the slave back to the terminal. 
