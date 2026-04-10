# Novy Pureline Pro Integration for Home Assistant

This is a custom integration for Home Assistant to control Novy Pureline Pro cooker hoods over Bluetooth Low Energy (BLE).

## Features

* Control the fan (on/off, speed).
* Control the light (on/off, brightness, color temperature).
* View sensor data (timers, operating hours).
* Control the recirculation mode.
* Buttons for common actions (power toggle, delayed off, etc.).

## Installation

1.  **Copy the integration files:**
    Copy the `custom_components/novy_pureline_pro` directory to the `custom_components` directory of your Home Assistant configuration.

2.  **Restart Home Assistant:**
    Restart your Home Assistant instance to allow it to find the new integration.

## Configuration

1.  Go to **Settings > Devices & Services**.
2.  Click the **+ Add Integration** button.
3.  Search for "Novy Pureline Pro" and select it.
4.  Enter the Bluetooth MAC address of your cooker hood. You can find this using a Bluetooth scanner app on your phone, or by checking the ESPHome logs if you have previously used the ESPHome component.
5.  Click **Submit**.

The integration will be set up, and you will see all the related entities for your cooker hood.

## Entities

This integration will create the following entities:

*   **Fan:** `fan.extractor_fan`
*   **Light:** `light.extractor_light`
*   **Sensors:**
    *   `sensor.off_timer`
    *   `sensor.boost_timer`
    *   `sensor.grease_timer`
    *   `sensor.led_hours`
    *   `sensor.fan_hours`
*   **Binary Sensor:**
    *   `binary_sensor.clean_grease_filter`
*   **Switch:**
    *   `switch.recirculation`
*   **Buttons:**
    *   `button.power_toggle`
    *   `button.delayed_off`
    *   `button.reset_grease_filter`
    *   `button.set_default_light`
    *   `button.set_default_speed`

## Update Mechanism

This integration polls the cooker hood every 30 seconds to get its current state. This means that if you control the device with its physical remote, the state in Home Assistant will be updated on the next poll. When you perform an action in Home Assistant (e.g., turn on the light), the integration will immediately request an update to reflect the new state.
