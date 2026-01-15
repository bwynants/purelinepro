"""The Novy Pureline Pro integration."""
import asyncio
import logging
from datetime import timedelta

from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.const import CONF_MAC
from homeassistant.helpers.update_coordinator import DataUpdateCoordinator, UpdateFailed

from .const import DOMAIN
from .pureline import PurelineProDevice

_LOGGER = logging.getLogger(__name__)

PLATFORMS = ["fan", "light", "sensor", "binary_sensor", "switch", "button"]


async def async_setup_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    """Set up Novy Pureline Pro from a config entry."""
    hass.data.setdefault(DOMAIN, {})
    mac = entry.data[CONF_MAC]
    device = PurelineProDevice(mac)

    async def _async_update_data():
        """Fetch data from API endpoint."""
        try:
            return await device.async_get_status()
        except Exception as err:
            raise UpdateFailed(f"Error communicating with API: {err}")

    coordinator = DataUpdateCoordinator(
        hass,
        _LOGGER,
        name="Novy Pureline Pro",
        update_method=_async_update_data,
        update_interval=timedelta(seconds=30),
    )

    await coordinator.async_config_entry_first_refresh()

    hass.data[DOMAIN][entry.entry_id] = {
        "device": device,
        "coordinator": coordinator,
    }

    for component in PLATFORMS:
        hass.async_create_task(
            hass.config_entries.async_forward_entry_setup(entry, component)
        )

    return True


async def async_unload_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    """Unload a config entry."""
    unload_ok = all(
        await asyncio.gather(
            *[
                hass.config_entries.async_forward_entry_unload(entry, component)
                for component in PLATFORMS
            ]
        )
    )
    if unload_ok:
        data = hass.data[DOMAIN].pop(entry.entry_id)
        await data["device"].disconnect()

    return unload_ok
