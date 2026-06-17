# NERS — Neonatal Emergency Response Swaddle

**An integrated, low-cost device for newborn resuscitation in low-resource health facilities.**

> Birth asphyxia is the leading cause of neonatal mortality in Kenya, responsible for 31.6% of all newborn deaths. In the "Golden Minute" after birth, every 30-second delay in starting ventilation raises mortality risk by 16%. NERS exists to remove the delays caused by fragmented equipment during that minute.

![Status](https://img.shields.io/badge/status-proof%20of%20concept-yellow)
![Platform](https://img.shields.io/badge/microcontroller-Arduino%20Nano%20V3.0-00979D)
![License](https://img.shields.io/badge/license-TBD-lightgrey)

---

## Table of Contents
- [Overview](#overview)
- [The Problem](#the-problem)
- [How NERS Works](#how-ners-works)
- [Hardware](#hardware)
- [Firmware](#firmware)
- [Bill of Materials](#bill-of-materials)
- [Results](#results)
- [Repository Structure](#repository-structure)
- [Limitations & Future Work](#limitations--future-work)
- [Team](#team)
- [Acknowledgments](#acknowledgments)
- [References](#references)

---

## Overview

The **Neonatal Emergency Response Swaddle (NERS)** is a single, wearable device that delivers three critical newborn resuscitation interventions simultaneously the moment a non-breathing infant is wrapped in it:

1. **Thermal regulation** — non-contact temperature sensing with embedded warming
2. **Airway clearance** — automated peristaltic suction
3. **Chest stimulation** — rhythmic, bilateral compression

All three modes are independently controlled by an **Arduino Nano V3.0**, activated via labelled push buttons, and monitored in real time on an OLED display. The goal is to eliminate the device-to-device transition time that currently costs lives in under-resourced delivery rooms.

This project was developed through the **Medical Device Innovation Summer Program** at Kenyatta University's **Centre for Design, Innovation & Engineering (CDIE)** lab, in partnership with **Rice 360° Institute for Global Health Technologies**, following a clinical immersion at **Thika Level 5 Hospital**, Kiambu County, Kenya.

A working prototype has been built and validated on a neonatal mannequin, with a total bill of materials cost of **KES 5,937 (~USD 46)**.

## The Problem

In most Kenyan public health facilities, a single birth attendant must manage three separate resuscitation tasks — thermal regulation, airway clearance, and chest stimulation — using three separate, non-integrated pieces of equipment, under acute time pressure and often with limited staff. Each transition between devices costs precious seconds within the Golden Minute.

Key figures motivating this project:
- ~31,000 neonatal deaths in Kenya in 2024 (20.7 per 1,000 live births)
- 16% increase in mortality risk for every 30-second delay in initiating ventilation
- 17.5% of Kenyan neonatal admissions arrive hypothermic, a state linked to a 35% increase in odds of death
- Only 32.7% of Kenyan newborns receive all recommended components of quality neonatal care after birth

NERS targets the fragmentation and operator-dependency at the root of these delays.

## How NERS Works

| Subsystem | Function | Core Components |
|---|---|---|
| **Thermal Regulation** | Continuous non-contact monitoring of neonatal surface temperature with embedded warming | MLX90614ESF IR sensor + heating strip |
| **Airway Clearance** | Controlled negative-pressure suction to clear the airway | 28BYJ-48 stepper motor in a peristaltic pump configuration + silicone tubing |
| **Chest Stimulation** | Rhythmic, bilateral compression replicating the two-thumb encircling technique | Two counter-rotating 28BYJ-48 stepper motors + padded compression elements |
| **Control & Feedback** | Independent mode activation and real-time status display | Arduino Nano V3.0 + 3 push buttons + 0.96" SSD1306 OLED |

Each mode can be activated independently or run concurrently — matching real resuscitation demand, where a birth attendant may need warming, suction, and compression all at once. The temperature sensor (I2C address `0x5A`) and OLED display (I2C address `0x3C`) share a single I2C bus without conflict.

## Hardware

- **Microcontroller:** Arduino Nano V3.0 (ATmega328P)
- **Actuation:** 3× 28BYJ-48 5VDC unipolar stepper motors, each driven via a ULN2003 Darlington driver board
- **Sensing:** MLX90614ESF non-contact infrared thermometer (±0.5°C accuracy, I2C)
- **Display:** 0.96" SSD1306 128×64 OLED (I2C)
- **Input:** 3 debounced push buttons (Warming / Suction / Massage)
- **Suction path:** 3D-printed peristaltic pump head + silicone tubing + neonatal suction catheter
- **Structure:** 3D-printed (PLA) motor mounts and pump housing, integrated into a fabric swaddle

Full pin mapping and component datasheets are documented in the project report (Appendices A & B).

## Firmware

Developed in the Arduino IDE (C/C++) using a cooperative multitasking architecture based on `millis()` timing rather than interrupts:

- **Button polling** with a 300 ms debounce window; each press toggles its mode independently
- **Suction motor** stepped via half-step sequence at 1 ms/step (~0.49 rev/s)
- **Massage motors** counter-rotate (one forward, one reverse) in synchronized steps to produce a squeezing motion
- **Temperature sampling** every 4 seconds, with on-screen alerts (`! LOW` / `OK` / `! HIGH`) based on WHO hypothermia thresholds

**Libraries used:**
- `Wire.h` — I2C communication
- `Adafruit_GFX.h` / `Adafruit_SSD1306.h` — OLED display driver
- `Adafruit_MLX90614.h` — temperature sensor driver

## Bill of Materials

| Component | Qty | Unit Price (KES) | Total (KES) |
|---|---|---|---|
| GY-906 MLX90614ESF IR Temperature Sensor | 1 | 1,500 | 1,500 |
| 28BYJ-48 Stepper Motor 5VDC | 3 | 300 | 900 |
| ULN2003 Stepper Motor Driver Board | 3 | 150 | 450 |
| SSD1306 0.96" I2C OLED Display | 1 | 600 | 600 |
| Arduino Nano V3.0 | 1 | 1,000 | 1,000 |
| Power Supply Adapter 5V 3A | 1 | 600 | 600 |
| Silicone Pipe, Breadboard, Veroboard, Jumper Wires, LEDs, Resistors | — | — | 887 |
| **Total** | | | **5,937 (~USD 46)** |

All components sourced from a single local supplier (PixelElectric, Nairobi), keeping the device locally manufacturable without import dependency.

## Results

All three subsystems were validated independently and concurrently on a neonatal mannequin:

- ✅ Accurate temperature readings and correct alert states across room-temperature and body-temperature ranges
- ✅ Confirmed peristaltic suction wave through silicone tubing with clean activation/deactivation
- ✅ Coordinated, rhythmic bilateral chest compression with no inter-subsystem interference
- ✅ Stable, conflict-free shared I2C bus operation
- ✅ Full concurrent (3-mode) operation validated with no system failure

Full performance tables are available in the project report (Chapter 4).

## Repository Structure

```
.
├── firmware/          # Arduino C/C++ source code
├── hardware/          # Schematics, pin mapping, 3D models (pump head, compression pads, mounts)
├── docs/              # Full project report, grant proposal, datasheets
└── README.md
```
*(Adjust to match your actual repo layout.)*

## Limitations & Future Work

This is a **validated proof of concept**, not yet a clinical device. Current limitations and the planned next steps:

- **Force & pressure calibration** — compression depth and suction pressure are not yet instrumented or calibrated against ILCOR clinical targets
- **Biocompatibility** — PLA and fabric will be replaced with medical-grade silicone for skin contact
- **Custom PCB** — current Veroboard assembly will be replaced with a compact, reliable custom PCB
- **Vital signs monitoring** — future integration of a pulse oximetry module (e.g., MAX30102) for heart rate and SpO₂
- **Clinical validation** — usability studies with real birth attendants, ethical approval, and eventual Kenya Pharmacy and Poisons Board regulatory review

This project has received no prior external funding beyond CDIE program support. A Catalyst Grant application is in progress to fund the calibration and PCB-integration phase as a stage gate toward clinical usability testing.

## Team

| Name | Role | Institution |
|---|---|---|
| **Cedric J. Murage Mwangi** | Hardware & Firmware Lead | Kenyatta University |
| **Hilda Kalino** | Systems & Sustainability Lead | Malawi University of Science and Technology |
| **Keturah Kirabo** | Clinical & Validation Lead | Mbarara University of Science and Technology |

Brought together through the **Medical Device Innovation Summer Program** hosted at Kenyatta University's CDIE lab.

## Acknowledgments

With thanks to the KU CDIE team — Dr. June Madete, Dr. Ken Iloka, Eubrey Mitchy, Stacy Awinja — and interns Antonius Waka and Abigael Njeri, for their support throughout this project. Thanks also to the staff at **Thika Level 5 Hospital** for their hospitality and for allowing the clinical immersion that inspired NERS.

## References

Key sources informing this work include UNICEF and KDHS neonatal mortality data, Mwangi et al. (2018) on time-to-ventilation outcomes, Wainaina et al. (2024) on neonatal hypothermia in Kenya, and the 2023/2025 ILCOR Neonatal Life Support guidelines. Full citations are available in the project report.

---

*For the complete technical writeup, including literature review, methodology, and discussion, see the full project report in this repository.*
