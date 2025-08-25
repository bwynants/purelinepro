"""Light platform for Novy Pureline Pro."""
from typing import Any, TYPE_CHECKING

from homeassistant.components.light import (
    ATTR_BRIGHTNESS,
    ATTR_COLOR_TEMP,
    ColorMode,
    LightEntity,
)
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity import DeviceInfo
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import DOMAIN
if TYPE_CHECKING:
    from .pureline import PurelineProDevice


async def async_setup_entry(
    hass: HomeAssistant,
    config_entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up the Novy Pureline Pro light from a config entry."""
    data = hass.data[DOMAIN][config_entry.entry_id]
    device: PurelineProDevice = data["device"]
    coordinator = data["coordinator"]
    async_add_entities([NovyPurelineProLight(device, coordinator, config_entry)])


class NovyPurelineProLight(CoordinatorEntity, LightEntity):
    """Representation of a Novy Pureline Pro light."""

    _attr_color_mode = ColorMode.COLOR_TEMP
    _attr_supported_color_modes = {ColorMode.COLOR_TEMP}

    def __init__(self, device: PurelineProDevice, coordinator, entry: ConfigEntry):
        """Initialize the light."""
        super().__init__(coordinator)
        self._device = device
        self._entry = entry
        self._attr_device_info = DeviceInfo(
            identifiers={(DOMAIN, self._entry.unique_id)},
            name=self._entry.title,
            manufacturer="Novy",
            model="Pureline Pro",
        )
        self._attr_unique_id = f"{self._entry.unique_id}-light"
        self._attr_name = "Extractor Light"
        self._attr_min_mireds = 153  # 6500K
        self._attr_max_mireds = 370  # 2700K

    @property
    def is_on(self) -> bool:
        """Return true if the light is on."""
        if self.coordinator.data and "status" in self.coordinator.data:
            return self.coordinator.data["status"].light_state
        return False

    @property
    def brightness(self) -> int:
        """Return the brightness of this light between 0..255."""
        if self.coordinator.data and "status" in self.coordinator.data:
            return self.coordinator.data["status"].brightness
        return 0

    @property
    def color_temp(self) -> int:
        """Return the CT color value in mireds."""
        if self.coordinator.data and "status" in self.coordinator.data:
            # Convert device value (0-255) to mireds (153-370)
            device_color_temp = self.coordinator.data["status"].color_temp
            return int(
                self._attr_max_mireds
                - (
                    (device_color_temp / 255)
                    * (self._attr_max_mireds - self._attr_min_mireds)
                )
            )
        return self._attr_min_mireds

    async def async_turn_on(self, **kwargs: Any) -> None:
        """Turn the light on."""
        await self._device.turn_light_on()

        if ATTR_BRIGHTNESS in kwargs:
            await self._device.set_light_brightness(kwargs[ATTR_BRIGHTNESS])

        if ATTR_COLOR_TEMP in kwargs:
            color_temp = kwargs[ATTR_COLOR_TEMP]
            device_color_temp = int(
                255
                - (
                    (color_temp - self._attr_min_mireds)
                    / (self._attr_max_mireds - self._attr_min_mireds)
                )
                * 255
            )
            await self._device.set_light_colortemp(device_color_temp)

        await self.coordinator.async_request_refresh()

    async def async_turn_off(self, **kwargs: Any) -> None:
        """Turn the light off."""
        await self._device.turn_light_off()
        await self.coordinator.async_request_refresh()
