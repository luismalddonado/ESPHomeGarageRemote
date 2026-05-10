# Functional Specification: loracoche Module (Meshtastic Version)

## 1. Overview
The `loracoche` module (specifically the `loracochemeshtastic` variant) acts as a **LoRa-to-BLE Gateway**. It is designed to be installed in a vehicle to receive long-range commands (LoRa) from a home automation system and relay them via Bluetooth Low Energy (BLE) to the `puertagaraje` module.

The module follows a modular architecture using ESPHome substitutions for hardware-specific and protocol-specific parameters.

## 2. System Objectives
- **Long-Range Trigger**: Receive encrypted or plain-text commands (default: `cmdopen`) over LoRa to trigger the garage door.
- **Proximity Detection**: Monitor status packets (default: `pingcasa`) to determine vehicle proximity.
- **Secure BLE Relay**: Connect as an authenticated client to the `puertagaraje` BLE server.
- **Visual Feedback**: Real-time status updates via an SSD1306 OLED display.
- **Power Efficiency**: Optimized Wi-Fi and radio settings for vehicle-powered operation.

## 3. Hardware Architecture
### 3.1 Components
- **Microcontroller**: ESP32-S3 (8MB Flash).
- **LoRa Transceiver**: Semtech SX1262 (via SPI).
- **Display**: SSD1306 OLED (128x64) via I2C.

### 3.2 Modular Configuration (Substitutions)
Most hardware and protocol settings are centralized for easy maintenance:
- **Radio**: LoRa Frequency (`868.92MHz`), Command Strings (`cmdopen`, `pingcasa`).
- **Bluetooth**: Target MAC Address, Service UUID (`180F`), Char UUIDs (`1801`, `1802`).
- **Pins**: Fully mapped for SPI (LoRa), I2C (OLED), and Vext control.

## 4. Functional Requirements
### 4.1 LoRa Communication
- **Protocol**: Raw LoRa (SX126x).
- **Payload Logic**:
    - **Command Open**: Matches `cmd_open_str` to set `abrir_puerta`.
    - **Proximity Ping**: Matches `ping_casa_str` to update `last_home_event_time`.
- **Formatting**: Uses shared `helpers.h` (`format_payload`) for consistent debugging and logging of packets.

### 4.2 BLE Client & Reliability
- **Connection Logic**: 
    - Attempts connection only when a trigger is pending.
    - **Backoff Strategy**: Initial retries every 5s for 3 attempts; if failing, backs off to a 60s retry interval to preserve the BLE stack and power.
- **Persistence**: If a LoRa command is received while disconnected, it is queued via `ble_command_pending`.

### 4.3 Power & Thermal Management
- **Wi-Fi Optimization**: 
    - `power_save_mode: light` enabled.
    - Reduced `output_power: 17dB` to minimize heat generation in enclosed vehicle environments.

## 5. Global Variables & Logic
| ID | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `abrir_puerta` | `int` | `0` | Trigger flag for opening the garage. |
| `ble_command_pending`| `bool` | `false` | Indicates a command is waiting for connection. |
| `ble_retry_count` | `int` | `0` | Tracks consecutive failed connection attempts. |
| `ble_last_attempt_time`| `uint32_t`| `0` | Timestamp of the last BLE connection attempt. |
| `max_wait_time_home_event`| `int` | `120` | Timeout (s) to consider the vehicle "FAR". |
| `open_door_command_duration`| `int`| `0` | Tracks server-side replay time (synced via BLE). |

## 6. Sequence of Operation (cmdopen)
1. LoRa packet received; `format_payload` converts it to string.
2. If it matches `cmd_open_str`, `abrir_puerta` is set.
3. Interval (5s) checks `abrir_puerta`.
4. If disconnected, checks `ble_last_attempt_time` against backoff delay (5s or 60s).
5. If delay elapsed, increments `ble_retry_count` and attempts `connect`.
6. On successful connection, `ble_retry_count` resets, and pending write is executed.
