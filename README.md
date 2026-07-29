# STM32 IoT Room Monitor

A DHT11 temperature/humidity sensor connected to an STM32 Nucleo-F446RE, which streams readings over I2C to a Raspberry Pi Zero 2W. The Pi runs a local web app so room conditions can be checked from any device on the home network.

## Overview

```
[DHT11] --GPIO--> [STM32 Nucleo-F446RE] --I2C--> [Raspberry Pi Zero 2W] --WiFi--> [Browser: phone / desktop]
                     (I2C slave)                (I2C master, FastAPI +
                                                  WebSocket server)
```

- The STM32 polls the DHT11 sensor and acts as an **I2C slave**, exposing the latest reading over a simple register map.
- The Pi Zero 2W acts as the **I2C master**, polling the STM32 once per second.
- A **FastAPI** backend on the Pi broadcasts each reading to connected clients over a **WebSocket**, so the dashboard updates live without polling from the browser.
- Any device on the same local network can view current temperature and humidity by visiting the Pi's IP address.

## Features

- Live temperature and humidity readings, updated every second
- No cloud dependency — everything runs locally on the home network
- Checksum validation on sensor readings
- Simple, lightweight web dashboard (no page refresh needed)

## Hardware

| Component | Details |
|---|---|
| Microcontroller | STM32 Nucleo-F446RE |
| Sensor | DHT11 temperature/humidity sensor |
| Single-board computer | Raspberry Pi Zero 2W |
| Misc | Breadboard, jumper wires, 2x 4.7kΩ pull-up resistors (I2C) |

## Software / stack

- **Firmware:** STM32CubeIDE, STM32 HAL
- **Backend:** Python, FastAPI, `smbus2`, WebSockets, `uvicorn`
- **Frontend:** HTML/JS (live dashboard) — see [Roadmap](#roadmap) for planned React upgrade

## Wiring

I2C1 on the STM32 is configured on **PB6 (SCL)** and **PB7 (SDA)**.

| Signal | STM32 Nucleo-F446RE | Raspberry Pi Zero 2W |
|---|---|---|
| SCL | PB6 | GPIO3 (physical pin 5) |
| SDA | PB7 | GPIO2 (physical pin 3) |
| GND | GND | GND |

Both SCL and SDA have 4.7kΩ pull-up resistors to the 3.3V rail. **Do not connect the boards' power rails together** — each board is powered independently; only the signal lines and ground are shared.

## I2C register map

The STM32 exposes the following 5-byte block starting at register `0x00`, matching the DHT11's native output format:

| Byte | Value |
|---|---|
| 0 | Humidity (integer part) |
| 1 | Humidity (decimal part) |
| 2 | Temperature (integer part) |
| 3 | Temperature (decimal part) |
| 4 | Checksum |

STM32 I2C slave address: `0x28`

## Setup

### STM32 firmware

1. Open the project in STM32CubeIDE
2. Flash to the Nucleo-F446RE
3. Confirm readings over the debug UART (USART2)

### Raspberry Pi

1. Enable I2C:
   ```bash
   sudo raspi-config
   # Interface Options -> I2C -> Yes
   ```
2. Install dependencies:
   ```bash
   sudo apt install -y i2c-tools python3-venv python3-full
   python3 -m venv venv
   source venv/bin/activate
   pip install smbus2 fastapi uvicorn
   ```
3. Confirm the STM32 is visible on the bus:
   ```bash
   i2cdetect -y 1
   # should show address 28
   ```
4. Run the server:
   ```bash
   uvicorn main:app --host 0.0.0.0 --reload
   ```
5. From any device on the same network, visit:
   ```
   http://<pi-ip-address>:8000
   ```
   Find the Pi's IP with `hostname -I`.

## Roadmap

- [ ] Rebuild the dashboard in React + Recharts (matching the styling of my [Lichess Player Tracker](https://stvndai.github.io))
- [ ] Add SQLite persistence for historical trend charts
- [ ] Run as a systemd service for auto-start on boot
- [ ] Reconnect/last-seen indicator on the frontend if the STM32 or WebSocket connection drops

## License

MIT
