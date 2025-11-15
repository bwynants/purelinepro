"""Sensor platform for Novy Pureline Pro."""
from homeassistant.components.sensor import SensorEntity, SensorStateClass
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity import DeviceInfo
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.const import TIME_SECONDS, TIME_MINUTES, TIME_HOURS
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import DOMAIN
from .pureline import PurelineProDevice

SENSORS = {
    "off_timer": {"name": "Off Timer", "unit": TIME_SECONDS, "icon": "mdi:timer-outline", "source": "status", "key": "timer"},
    "boost_timer": {"name": "Boost Timer", "unit": TIME_SECONDS, "icon": "mdi:timer-sand", "source": "status", "key": "timer"}, # This is not quite right, boost is a separate flag
    "grease_timer": {"name": "Grease Timer", "unit": TIME_MINUTES, "icon": "mdi:filter-variant", "source": "status402", "key": "grease_timer"},
    "operating_hours_led": {"name": "LED Hours", "unit": TIME_HOURS, "icon": "mdi:lightbulb-on-outline", "source": "status404", "key": "led_timer"},
    "operating_hours_fan": {"name": "Fan Hours", "unit": TIME_HOURS, "icon": "mdi:fan", "source": "status403", "key": "fan_timer"},
}


async def async_setup_entry(
    hass: HomeAssistant,
    config_entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up the Novy Pureline Pro sensors from a config entry."""
    data = hass.data[DOMAIN][config_entry.entry_id]
    device: PurelineProDevice = data["device"]
    coordinator = data["coordinator"]

    sensors = [
        NovyPurelineProSensor(device, coordinator, config_entry, sensor_type, info)
        for sensor_type, info in SENSORS.items()
    ]
    async_add_entities(sensors)


class NovyPurelineProSensor(CoordinatorEntity, SensorEntity):
    """Representation of a Novy Pureline Pro sensor."""

    def __init__(self, device: PurelineProDevice, coordinator, entry: ConfigEntry, sensor_type: str, info: dict):
        """Initialize the sensor."""
        super().__init__(coordinator)
        self._device = device
        self._entry = entry
        self._sensor_type = sensor_type
        self._attr_name = info["name"]
        self._attr_native_unit_of_measurement = info["unit"]
        self._attr_icon = info["icon"]
        self._source = info["source"]
        self._key = info["key"]
        self._attr_device_info = DeviceInfo(
            identifiers={(DOMAIN, self._entry.unique_id)},
            name=self._entry.title,
            manufacturer="Novy",
            model="Pureline Pro",
        )
        self._attr_unique_id = f"{self._entry.unique_id}-{self._sensor_type}"
        self._attr_state_class = SensorStateClass.MEASUREMENT

    @property
    def native_value(self):
        """Return the state of the sensor."""
        if self.coordinator.data and self._source in self.coordinator.data:
            if self._sensor_type == "boost_timer":
                if self.coordinator.data["status"].boost:
                    return getattr(self.coordinator.data[self._source], self._key, None)
                return 0
            return getattr(self.coordinator.data[self._source], self._key, None)
        return None
