# ESPHome Garage Remote Backend

This ESPHome backend has two different modules:

- puertagaraje
- loracoche

## Architecture Diagram

```mermaid
sequenceDiagram
    participant HA as Home Assistant
    participant LC as loracoche
    participant PG as puertagaraje
    participant D as Garage Door

    HA->>LC: LoRa command (cmdopen / pingcasa)
    LC->>PG: BLE command (open_the_door)
    PG->>D: Send 433 MHz RF signal
    PG-->>LC: repeat_counter_read (remaining time)
```	

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
- This would be an example code to send LORA commands to  **loracoche** from a third ESP Home device connected to your Home Assistant

```yaml
button:
  - platform: template
    id: open_door
    name: "Open garage door"
    on_press:
      then:
        - sx127x.send_packet:
            data: !lambda |-
              ESP_LOGI("LoRa", "CMDOPEN packege sent");
              std::string msg = "cmdopen";
              return std::vector<uint8_t>(msg.begin(), msg.end());
			  
packet_transport:
  platform: sx127x
  update_interval: 60s
  sensors:
    - pingcasa
  providers:
    - name: lorasender1
	
sensor:
  - platform: template
    name: "PING casa"
    id: pingcasa
    update_interval: 60s    	
```
			  







