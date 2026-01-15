"""Switch platform for Novy Pureline Pro."""
from typing import Any

from homeassistant.components.switch import SwitchEntity
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
    """Set up the Novy Pureline Pro switches from a config entry."""
    data = hass.data[DOMAIN][config_entry.entry_id]
    device: PurelineProDevice = data["device"]
    coordinator = data["coordinator"]
    async_add_entities([NovyPurelineProRecirculateSwitch(device, coordinator, config_entry)])


class NovyPurelineProRecirculateSwitch(CoordinatorEntity, SwitchEntity):
    """Representation of a Novy Pureline Pro recirculate switch."""

    def __init__(self, device: PurelineProDevice, coordinator, entry: ConfigEntry):
        """Initialize the switch."""
        super().__init__(coordinator)
        self._device = device
        self._entry = entry
        self._attr_device_info = DeviceInfo(
            identifiers={(DOMAIN, self._entry.unique_id)},
            name=self._entry.title,
            manufacturer="Novy",
            model="Pureline Pro",
        )
        self._attr_unique_id = f"{self._entry.unique_id}-recirculate"
        self._attr_name = "Recirculation"
        self._attr_icon = "mdi:air-filter"

    @property
    def is_on(self) -> bool | None:
        """Return true if the switch is on."""
        if self.coordinator.data and "status402" in self.coordinator.data:
            return self.coordinator.data["status402"].recirculate
        return None

    async def async_turn_on(self, **kwargs: Any) -> None:
        """Turn the switch on."""
        await self._device.set_recirculate(True)
        await self.coordinator.async_request_refresh()

    async def async_turn_off(self, **kwargs: Any) -> None:
        """Turn the switch off."""
        await self._device.set_recirculate(False)
        await self.coordinator.async_request_refresh()
