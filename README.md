<h1 align="center">GyroBass 🥙🎸</h1>
<h3 align="center" >Motion-Controlled Embedded Audio Effects on STM32. </h3>
<br>

## Overview

GyroBass is a real-time digital signal processing (DSP) designed to be embedded directly into the body of a bass guitar.
Powered by the Daisy Seed (STM32H7), it bridges physical permormance and sound design by using motion as a modulation source.

The core feature is an LSM6DS3 Gyroscope/Accelerometer, which maps the player's physical movements (tilting the guitar) to filter cutoff frequency. This creates a "wah-wah" effect, that responds intuitevily to instrument's position, removing the need for a food pedal.

## Features

- Motion Control: I2C driver implementation for the LSM6DS3 sensor to read tilt data in real-time.

- Dynamic DSP Chain:
  - Input Conditioning: Headroom management and clean blend for bass frequencies.
  - Overdrive: Digital symmetrical soft clipping with gain staging.
  - State Variable Filter (SVF): Tilt-controlled frequency sweep (160Hz - 3350Hz).
  - Mode Switching: Toggle between Low-Pass, Band-Pass, and Notch filter topologies via hardware button.

## 🚀 Motivation
This project serves as a practical exploration of embedded audio programming. It demonstrates:

- I2C Sensor Interface: Custom implementation for the LSM6DS3 gyroscope.

- Real-time Audio Processing: Non-blocking, interrupt-driven signal chain.

- Managing DSP signal chains in C++.