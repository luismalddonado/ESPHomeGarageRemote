# Functional Specification: puertagaraje Module

## 1. Overview
The `puertagaraje` module is an ESP32-based IoT device designed to act as a universal garage door remote cloner and controller. It bridges high-level control interfaces (Bluetooth Low Energy and ESPHome/Home Assistant) with low-level RF communication (433.92MHz).

## 2. System Objectives
- **Signal Learning**: Capture and store raw RF signals from existing garage door remotes.
- **Signal Replay**: Transmit learned or default RF signals to actuate garage door motors.
- **Persistent Storage**: Retain learned signals across power cycles using Non-Volatile Storage (NVS).
- **Remote Control**: Provide a BLE interface for mobile apps and gateway modules (`loracoche`) to trigger actions.

## 3. Hardware Architecture
### 3.1 Components
- **Microcontroller**: ESP32 (esp-idf framework).
- **RF Transceiver**: Texas Instruments CC1101.
- **Antenna**: Tuned for 433.92MHz.

### 3.2 Connectivity (Wiring)
| CC1101 Pin | ESP32 Pin | Function |
| :--- | :--- | :--- |
| VCC | 3.3V | Power Supply |
| GND | GND | Ground |
| SCK | GPIO18 | SPI Clock |
| MOSI | GPIO23 | SPI Master Out Slave In |
| MISO | GPIO19 | SPI Master In Slave Out |
| CSN | GPIO5 | SPI Chip Select |
| GDO0 | GPIO4 | RF Transmit (TX) |
| GDO2 | GPIO2 | RF Receive (RX) |

## 4. Functional Requirements
### 4.1 RF Communication
- **Frequency**: 433.92 MHz.
- **Modulation**: ASK/OOK (Amplitude Shift Keying).
- **Encoding**: Raw pulse width representation.
- **Transmission Logic**: Upon trigger, the signal is replayed repeatedly (default: 60 iterations at 3s intervals) to ensure reliable actuation.

### 4.2 Learning Mode
- Triggered via BLE or ESPHome interface.
- Captures raw pulse sequences via `remote_receiver`.
- Filters out spurious pulses (shorter than 100µs or longer than 10ms).
- Automatically saves valid signals to NVS and restarts the device to apply changes.

### 4.3 Persistence
- Uses `global_preferences` (NVS) to store a `LearnedCode` structure:
  ```cpp
  struct LearnedCode {
    uint16_t length;
    int32_t data[256];
  };
  ```

## 5. Interface Specifications
### 5.1 Bluetooth Low Energy (BLE)
**Service UUID**: `180F`
| Characteristic UUID | Description | Operations | Data Format |
| :--- | :--- | :--- | :--- |
| `1801` | Open Door | Write | Any write triggers transmission |
| `1802` | Repeat Counter | Read/Notify | 4-byte Little Endian (remaining cycles) |
| `1803` | Start Learning | Write | Any write enables Learning Mode |
| `1804` | End Learning | Write | Any write disables Learning Mode |
| `1805` | Return Code | Read | Partial preview of active RF signal |

### 5.2 User Interface (ESPHome)
- **Buttons**:
  - `Open door`: Manually trigger signal replay.
  - `Learn code start/stop`: Toggle learning state.
  - `Restart Device`: Manual reboot.
- **Sensors**:
  - `Signal length`: Displays number of pulses in the learned code.
  - `Last Received Signal`: Text sensor showing a preview of the pulse sequence.

## 6. Configuration Parameters (Globals)
These variables control the runtime behavior and state of the module.

| ID | Type | Initial Value | Description |
| :--- | :--- | :--- | :--- |
| `default_code` | `std::vector<int32_t>` | *Hardcoded* | A fallback RF pulse sequence (int32 array) used when no code has been learned or when `use_default_code` is true. |
| `learned_code` | `std::vector<int32_t>` | `Empty` | Stores the pulse sequence captured during Learning Mode. Not restored automatically by ESPHome globals; loaded manually from NVS on boot. |
| `use_default_code`| `bool` | `true` | **Source Selector**: If `true`, the device transmits `default_code`. If `false`, it transmits `learned_code`. |
| `learning_mode` | `bool` | `false` | **System State**: When `true`, the `remote_receiver` logic is active, listening for and capturing RF pulses into `learned_code`. |
| `signal_learned` | `bool` | `false` | Internal flag set to `true` once a valid signal is successfully captured in Learning Mode. |
| `reboot_after_signal_learned` | `bool` | `false` | Triggers the `save_and_restart` script once a signal is captured to ensure persistence. |
| `repeat_counter` | `int` | `60` | **Transmission Timer**: When set to a value > 0 (typically by the "Open Door" command), the device transmits the active RF signal every 3 seconds, decrementing this counter until it reaches 0. |

### 6.1 Logic Interaction
- **Triggering**: Setting `repeat_counter` to 60 (via BLE or Button) initiates a 180-second transmission cycle (60 cycles * 3s interval).
- **Persistence Logic**: The `LearnedCode` struct is limited to 256 pulses. If a captured signal exceeds this, it is truncated during the NVS save process.
- **Boot Sequence**: At `priority: -100`, the system attempts to load `learned_code` from NVS. If successful, it populates the `learned_code` vector and updates the `current_signal` text sensor.

## 7. Security & Safety
- CC1101 must be powered strictly at 3.3V.
- BLE access is currently open (no PIN/Encryption configured in YAML); addition of `esp32_ble_beacon` or specific bonding is recommended for production security.
