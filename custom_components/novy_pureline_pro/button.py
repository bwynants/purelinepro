"""Button platform for Novy Pureline Pro."""
from homeassistant.components.button import ButtonEntity
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity import DeviceInfo
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import DOMAIN
from .pureline import PurelineProDevice

BUTTONS = {
    "power": {"name": "Power Toggle", "icon": "mdi:power"},
    "delayed_off": {"name": "Delayed Off", "icon": "mdi:timer-off-outline"},
    "reset_grease": {"name": "Reset Grease Filter", "icon": "mdi:filter-remove-outline"},
    "set_default_light": {"name": "Set Default Light", "icon": "mdi:lightbulb-on-outline"},
    "set_default_speed": {"name": "Set Default Speed", "icon": "mdi:fan-plus"},
}


async def async_setup_entry(
    hass: HomeAssistant,
    config_entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up the Novy Pureline Pro buttons from a config entry."""
    data = hass.data[DOMAIN][config_entry.entry_id]
    device: PurelineProDevice = data["device"]
    coordinator = data["coordinator"]

    buttons = [
        NovyPurelineProButton(device, coordinator, config_entry, button_type, info)
        for button_type, info in BUTTONS.items()
    ]
    async_add_entities(buttons)


class NovyPurelineProButton(CoordinatorEntity, ButtonEntity):
    """Representation of a Novy Pureline Pro button."""

    def __init__(self, device: PurelineProDevice, coordinator, entry: ConfigEntry, button_type: str, info: dict):
        """Initialize the button."""
        super().__init__(coordinator)
        self._device = device
        self._entry = entry
        self._button_type = button_type
        self._attr_name = info["name"]
        self._attr_icon = info["icon"]
        self._attr_device_info = DeviceInfo(
            identifiers={(DOMAIN, self._entry.unique_id)},
            name=self._entry.title,
            manufacturer="Novy",
            model="Pureline Pro",
        )
        self._attr_unique_id = f"{self._entry.unique_id}-{self._button_type}"

    async def async_press(self) -> None:
        """Handle the button press."""
        if self._button_type == "power":
            await self._device.power_toggle()
        elif self._button_type == "delayed_off":
            # In the ESPHome component, this sets the fan to 25% and then turns it off after 5 minutes.
            # We can only set the speed here. A timer would need to be implemented in HA.
            await self._device.set_fan_speed(25)
        elif self._button_type == "reset_grease":
            await self._device.reset_grease_filter()
        elif self._button_type == "set_default_light":
            await self._device.set_default_light()
        elif self._button_type == "set_default_speed":
            await self._device.set_default_speed()

        await self.coordinator.async_request_refresh()
