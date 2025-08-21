"""Binary sensor platform for Novy Pureline Pro."""
from homeassistant.components.binary_sensor import (
    BinarySensorEntity,
    BinarySensorDeviceClass,
)
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity import DeviceInfo
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import DOMAIN
from .pureline import PurelineProDevice


async def async_setup_entry(
    hass: HomeAssistant,
    config_entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up the Novy Pureline Pro binary sensors from a config entry."""
    data = hass.data[DOMAIN][config_entry.entry_id]
    device: PurelineProDevice = data["device"]
    coordinator = data["coordinator"]
    async_add_entities([NovyPurelineProCleanGreaseSensor(device, coordinator, config_entry)])


class NovyPurelineProCleanGreaseSensor(CoordinatorEntity, BinarySensorEntity):
    """Representation of a Novy Pureline Pro clean grease sensor."""

    def __init__(self, device: PurelineProDevice, coordinator, entry: ConfigEntry):
        """Initialize the binary sensor."""
        super().__init__(coordinator)
        self._device = device
        self._entry = entry
        self._attr_device_info = DeviceInfo(
            identifiers={(DOMAIN, self._entry.unique_id)},
            name=self._entry.title,
            manufacturer="Novy",
            model="Pureline Pro",
        )
        self._attr_unique_id = f"{self._entry.unique_id}-cleangrease"
        self._attr_name = "Clean Grease Filter"
        self._attr_device_class = BinarySensorDeviceClass.PROBLEM

    @property
    def is_on(self) -> bool | None:
        """Return true if the binary sensor is on."""
        if self.coordinator.data and "status" in self.coordinator.data:
            return self.coordinator.data["status"].clean_grease_filter
        return None
