import voluptuous as vol
from homeassistant import config_entries
from homeassistant.const import CONF_MAC

from .const import DOMAIN

class NovyPurelineProConfigFlow(config_entries.ConfigFlow, domain=DOMAIN):
    """Handle a config flow for Novy Pureline Pro."""

    VERSION = 1
    CONNECTION_CLASS = config_entries.CONN_CLASS_LOCAL_POLL

    async def async_step_user(self, user_input=None):
        """Handle the initial step."""
        errors = {}
        if user_input is not None:
            await self.async_set_unique_id(user_input[CONF_MAC])
            self._abort_if_unique_id_configured()
            return self.async_create_entry(title=user_input[CONF_MAC], data=user_input)

        return self.async_show_form(
            step_id="user",
            data_schema=vol.Schema({vol.Required(CONF_MAC): str}),
            errors=errors,
        )
