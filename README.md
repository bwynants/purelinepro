# purelinepro
ESP32 Pureline Pro BLE esphome project

# Example configuration entry for finding MAC addresses, Service UUIDs, iBeacon UUIDs, and identifiers
esp32_ble_tracker:
  on_ble_advertise:
    - then:

logger:
  level: VERY_VERBOSE

### how to setup

Add a reference to the code on github

    external_components:
      - source:
          type: git
          url: https://github.com/bwynants/purelinepro

          ref: main
        components: [ purelinepro ]

or local on your esphome directory

    external_components:
      - source: 
          type: local
          path: components

add esphome ble tracker component

    esp32_ble_tracker:
      scan_parameters:
        interval: 1100ms
        window: 1100ms
        active: false

add esphome ble client component and set the correct MAC address

    ble_client:
      - mac_address: "00:00:00:00:00:00"
        id: purelinepro_ble_id

configure the components

 the main device
    purelinepro:
      - id: novydampkap

 the recirculate switch toggles normal or recirculate mode 
 the enabled switch toggles connected on or off (put off to connect bluetooth of another device) 

    switch:
      - platform: purelinepro
        purelinepro_id: novydampkap
        recirculate:
          name: Recirculate
        enabled:
          name: Enabled

 set up the light

    light:
      - platform: purelinepro
        purelinepro_id: novydampkap
        name: "Extractor Light"
        id: extractor_light
        restore_mode: RESTORE_DEFAULT_OFF

 set up some toggle buttons

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

 set up the fan
    fan:
      - platform: purelinepro
        purelinepro_id: novydampkap
        name: "Extractor Fan"
        id: extractor_fan
        restore_mode: RESTORE_DEFAULT_OFF

 setup the sensors
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
        name: Led Hours
        operating_hours_fan:
        name: Fan Hours

 setup the sbinary_sensorensors

    binary_sensor:
      - platform: purelinepro
        purelinepro_id: novydampkap
        cleangrease:
        disabled_by_default: false
        name: Clean Grease

