# Novy Pureline Pro ESPHome Integration

An ESPHome custom component for controlling a **Novy Pureline Pro extractor hood** over **Bluetooth Low Energy (BLE)** via an ESP32.

This integration is based on reverse-engineering the communication between the official mobile app and the extractor hood.

---

## 🔍 Discover Devices

To discover the MAC address, service UUIDs, and iBeacon UUIDs, use the following ESPHome configuration:

```yaml
esp32_ble_tracker:
  on_ble_advertise:
    - then:

logger:
  level: VERY_VERBOSE
```

Check the ESPHome logs to identify the MAC address associated with your Pureline Pro extractor hood.

---

## 🛠️ Installation

### 1. Add External Component

#### Option A: From GitHub

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/bwynants/purelinepro
      ref: main
    components: [purelinepro]
```

#### Option B: From Local Directory

Place the component in your ESPHome `components/` directory:

```yaml
external_components:
  - source:
      type: local
      path: components
```

---

### 2. Enable BLE Tracker

```yaml
esp32_ble_tracker:
  scan_parameters:
    interval: 1100ms
    window: 1100ms
    active: false
```

---

### 3. Set Up BLE Client

Replace the MAC address with your device's address:

```yaml
ble_client:
  - mac_address: "00:00:00:00:00:00"
    id: purelinepro_ble_id
```

---

### 4. Configure the Extractor Hood

#### Core Device

```yaml
purelinepro:
  - id: novydampkap
```

---

## 📦 Component Configuration

### Switches

- `Recirculate`: Toggles between normal and recirculate mode.
- `Enabled`: Enables/disables BLE connection (disable to pair with another device).

```yaml
switch:
  - platform: purelinepro
    purelinepro_id: novydampkap
    recirculate:
      name: Recirculate
    enabled:
      name: Enabled
```

---

### Light

Single light (combines white/ambient), supports color and brightness control.

```yaml
light:
  - platform: purelinepro
    purelinepro_id: novydampkap
    name: "Extractor Light"
    id: extractor_light
    restore_mode: RESTORE_DEFAULT_OFF
```

---

### Buttons

Useful remote-like actions:
  
- `power`: simulates power of remote
- `delayed_off`: if on it sets the fan to 25% and puts it off after 5 minutes
- `reset_grease`: resets grease filter timer
- `set_default_light`: sets default light
- `set_default_speed`: sets default speed


```yaml
button:
  - platform: purelinepro
    purelinepro_id: novydampkap
    power:
      name: "Power Toggle"
    delayed_off:
      name: "Delayed Off"
    reset_grease:
      name: "Reset Grease"
    set_default_light:
      name: "Set Default Light"
    set_default_speed:
      name: "Set Default Speed"
```

---

### Fan

```yaml
fan:
  - platform: purelinepro
    purelinepro_id: novydampkap
    name: "Extractor Fan"
    id: extractor_fan
    restore_mode: RESTORE_DEFAULT_OFF
```

---

### Sensors

Monitor extractor hood status and usage:

- `off_timer`: when in cooldown mode, this gives how long remaining before off
- `boost_timer`: when in boost mode (fan +75%) this is the remaing time before the fan speed gets lowered
- `grease_timer`: remainig time before grease filter needs cleaning
- `operating_hours_led`: total hours the leds where on
- `operating_hours_fan`: total hours the fan was on


```yaml
sensor:
  - platform: purelinepro
    purelinepro_id: novydampkap
    off_timer:
      name: Off Timer
    boost_timer:
      name: Boost Timer
    grease_timer:
      name: Grease Timer
    operating_hours_led:
      name: LED Hours
    operating_hours_fan:
      name: Fan Hours
```

---

### Binary Sensors

- `cleangrease`: Indicates when the grease filter needs cleaning.

```yaml
binary_sensor:
  - platform: purelinepro
    purelinepro_id: novydampkap
    cleangrease:
      name: Clean Grease
      disabled_by_default: false
```

---

## 📎 License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

---

## 🙏 Credits

Thanks to the open-source community for tools and documentation that made this reverse engineering effort possible.
