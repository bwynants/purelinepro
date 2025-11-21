"""Fan platform for Novy Pureline Pro."""
from homeassistant.components.fan import (
    FanEntity,
    SUPPORT_SET_SPEED,
)
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant, callback
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
    """Set up the Novy Pureline Pro fan from a config entry."""
    data = hass.data[DOMAIN][config_entry.entry_id]
    device: PurelineProDevice = data["device"]
    coordinator = data["coordinator"]
    async_add_entities([NovyPurelineProFan(device, coordinator, config_entry)])


class NovyPurelineProFan(CoordinatorEntity, FanEntity):
    """Representation of a Novy Pureline Pro fan."""

    def __init__(self, device: PurelineProDevice, coordinator, entry: ConfigEntry):
        """Initialize the fan."""
        super().__init__(coordinator)
        self._device = device
        self._entry = entry
        self._attr_device_info = DeviceInfo(
            identifiers={(DOMAIN, self._entry.unique_id)},
            name=self._entry.title,
            manufacturer="Novy",
            model="Pureline Pro",
        )
        self._attr_unique_id = f"{self._entry.unique_id}-fan"
        self._attr_name = "Extractor Fan"

    @property
    def is_on(self) -> bool:
        """Return true if the fan is on."""
        if self.coordinator.data and "status" in self.coordinator.data:
            return self.coordinator.data["status"].fan_state
        return False

    @property
    def percentage(self) -> int:
        """Return the current speed."""
        if self.coordinator.data and "status" in self.coordinator.data:
            return self.coordinator.data["status"].fan_speed
        return 0

    @property
    def supported_features(self) -> int:
        """Flag supported features."""
        return SUPPORT_SET_SPEED

    async def async_turn_on(self, percentage: int = None, **kwargs) -> None:
        """Turn the fan on."""
        if percentage is not None:
            await self.async_set_percentage(percentage)
        else:
            await self._device.turn_fan_on()
        await self.coordinator.async_request_refresh()

    async def async_turn_off(self, **kwargs) -> None:
        """Turn the fan off."""
        await self._device.turn_fan_off()
        await self.coordinator.async_request_refresh()

    async def async_set_percentage(self, percentage: int) -> None:
        """Set the speed of the fan."""
        await self._device.set_fan_speed(percentage)
        await self.coordinator.async_request_refresh()
