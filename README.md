#ESPHome Garage Remote backend

This ESPHome backend has two different modules

- puertagarage
- loracoche



Home Assistant  loracoche      puertagarage    Door to open
┌─────┐			┌───────┐ 		┌───────┐ 		┌─────┐
│ 	  │ 		│ 		│ ─────>│ 		│ 		│     │
│ 	  │ ──────> │ 		│ 		│ 		│ ─────>│     │
│ 	  │ 		│ 		│ <─────│ 		│ 		│ 	  │
└─────┘ 		└───────┘ 		└───────┘ 		└─────┘


## 🚗 loracoche Features

- Acts as a **Bluetooth client**
- Listens for **LoRa commands**:
  - `pingcasa`
  - `cmdopen`
- Upon receiving valid commands, it sends a BLE command (`open_the_door`) to the **puertagaraje** module

---

## 🚪 puertagaraje Features

- Acts as a **Bluetooth server**
- Listens for BLE commands from the **loracoche** module:
  - `open_the_door`
  - `repeat_counter_read`
- When `open_the_door` is received:
  - Sends a **433 MHz RF signal** for a configured duration (default: 300 seconds)
- `repeat_counter_read`:
  - Returns the remaining time (in seconds) until the RF signal stops

---

## 📡 Project Base

This project is based on the  
[cc1101-transceiver](https://github.com/phdindota/cc1101-transceiver)

---

## Notes

- Communication flow:
  - Home Assistant → LoRa → loracoche → BLE → puertagaraje → RF → Garage Door
- Designed for modularity and remote operation







