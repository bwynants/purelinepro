import asyncio
import logging
from typing import Optional
from dataclasses import dataclass
import struct

from bleak import BleakClient, BleakError

_LOGGER = logging.getLogger(__name__)


@dataclass
class HoodStatus:
    """Data class for hood status."""
    fan_state: bool
    fan_speed: int
    light_state: bool
    brightness: int
    color_temp: int
    timer: int
    clean_grease_filter: bool
    boost: bool

@dataclass
class HoodStatus402:
    """Data class for hood status 402."""
    grease_timer: int
    recirculate: bool
    version: str

@dataclass
class HoodStatus403:
    """Data class for hood status 403."""
    fan_timer: int
    recirculate_timer: int
    last_fan_speed: int

@dataclass
class HoodStatus404:
    """Data class for hood status 404."""
    led_timer: int


# From the C++ code
UART_SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
TX_CHAR_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"  # Notifications
RX_CHAR_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"  # Write

CMD_POWER = 10
CMD_LIGHT_ON_AMBI = 15
CMD_LIGHT_ON_WHITE = 16
CMD_LIGHT_BRIGHTNESS = 21
CMD_LIGHT_COLORTEMP = 22
CMD_RESET_GREASE = 23
CMD_FAN_RECIRCULATE = 25
CMD_FAN_SPEED = 28
CMD_FAN_STATE = 29
CMD_LIGHT_OFF = 36
CMD_FAN_DEFAULT = 41
CMD_LIGHT_DEFAULT = 42
CMD_HOOD_STATUS = 400
CMD_HOOD_STATUS_402 = 402
CMD_HOOD_STATUS_403 = 403
CMD_HOOD_STATUS_404 = 404

class PurelineProDevice:
    """A class to interact with a Novy Pureline Pro cooker hood."""

    def __init__(self, address: str):
        """Initialize the device."""
        self._address = address
        self._client: Optional[BleakClient] = None
        self.is_connected = False

    async def connect(self) -> bool:
        """Connect to the device."""
        _LOGGER.debug("Connecting to %s", self._address)
        try:
            self._client = BleakClient(self._address)
            await self._client.connect()
            self.is_connected = self._client.is_connected
            _LOGGER.debug("Connected to %s", self._address)
            return self.is_connected
        except BleakError as e:
            _LOGGER.error("Failed to connect to %s: %s", self._address, e)
            return False

    async def disconnect(self):
        """Disconnect from the device."""
        if self._client and self.is_connected:
            await self._client.disconnect()
            self.is_connected = False
            _LOGGER.debug("Disconnected from %s", self._address)

    async def _send_command(self, command_id: int, args: list[int] = None):
        """Send a command to the device."""
        if not self.is_connected or not self._client:
            _LOGGER.error("Not connected to device.")
            return

        payload_str = f"[{command_id}"
        if args:
            for arg in args:
                payload_str += f";{arg}"
        payload_str += "]"

        payload = payload_str.encode("utf-8")
        _LOGGER.debug("Sending command: %s", payload_str)
        try:
            await self._client.write_gatt_char(RX_CHAR_UUID, payload)
        except BleakError as e:
            _LOGGER.error("Failed to send command: %s", e)

    async def set_fan_speed(self, speed: int):
        """Set the fan speed."""
        await self._send_command(CMD_FAN_SPEED, [1, speed])

    async def turn_fan_on(self):
        """Turn the fan on."""
        await self._send_command(CMD_FAN_STATE, [1, 1])

    async def turn_fan_off(self):
        """Turn the fan off."""
        await self._send_command(CMD_FAN_STATE, [1, 0])

    async def set_light_brightness(self, brightness: int):
        """Set the light brightness."""
        await self._send_command(CMD_LIGHT_BRIGHTNESS, [1, brightness])

    async def set_light_colortemp(self, colortemp: int):
        """Set the light color temperature."""
        await self._send_command(CMD_LIGHT_COLORTEMP, [1, colortemp])

    async def turn_light_on(self):
        """Turn the light on."""
        await self._send_command(CMD_LIGHT_ON_WHITE, [0])

    async def turn_light_off(self):
        """Turn the light off."""
        await self._send_command(CMD_LIGHT_OFF, [0])

    async def power_toggle(self):
        """Toggle the power."""
        await self._send_command(CMD_POWER, [0])

    async def reset_grease_filter(self):
        """Reset the grease filter timer."""
        await self._send_command(CMD_RESET_GREASE, [0])

    async def set_recirculate(self, recirculate: bool):
        """Set the recirculate mode."""
        await self._send_command(CMD_FAN_RECIRCULATE, [1, 1 if recirculate else 0])

    async def set_default_light(self):
        """Set the current light settings as default."""
        await self._send_command(CMD_LIGHT_DEFAULT, [1, 1])

    async def set_default_speed(self):
        """Set the current fan speed as default."""
        await self._send_command(CMD_FAN_DEFAULT, [0])

    async def start_notifications(self, callback):
        """Start receiving notifications from the device."""
        if self._client and self.is_connected:
            await self._client.start_notify(TX_CHAR_UUID, callback)

    async def stop_notifications(self):
        """Stop receiving notifications from the device."""
        if self._client and self.is_connected:
            await self._client.stop_notify(TX_CHAR_UUID)

    def parse_packet(self, data: bytearray) -> Optional[HoodStatus]:
        """Parse the main status packet."""
        if len(data) != 20:
            return None
        flags1, fanspeed, flags2, _, _, lightmode, brightness, colortemp, countdown, _, _, _ = struct.unpack('<B B B B B B B B H H H H', data)

        return HoodStatus(
            fan_state=fanspeed > 0,
            fan_speed=fanspeed,
            light_state=lightmode > 0,
            brightness=brightness,
            color_temp=colortemp,
            timer=countdown,
            clean_grease_filter=(flags2 & 0x01) != 0,
            boost=(flags1 & 0x02) != 0,
        )

    def parse_packet402(self, data: bytearray) -> Optional[HoodStatus402]:
        """Parse the 402 status packet."""
        if len(data) != 23:
            return None
        _, flags, _, greasetime, major, minor, patch, _, _, _ = struct.unpack('<H B B I B B B B I I', data)
        return HoodStatus402(
            grease_timer=greasetime,
            recirculate=(flags & 0x01) != 0,
            version=f"{major}.{minor}.{patch}",
        )

    def parse_packet403(self, data: bytearray) -> Optional[HoodStatus403]:
        """Parse the 403 status packet."""
        if len(data) != 23:
            return None
        _, _, recirculateTimer, fantimer, lastfanspeed, _, _ = struct.unpack('<H I I I B B I', data)
        return HoodStatus403(
            fan_timer=fantimer,
            recirculate_timer=recirculateTimer,
            last_fan_speed=lastfanspeed,
        )

    def parse_packet404(self, data: bytearray) -> Optional[HoodStatus404]:
        """Parse the 404 status packet."""
        if len(data) != 23:
            return None
        _, _, _, _, ledtimer, _, _ = struct.unpack('<I I I B I B H', data)
        return HoodStatus404(
            led_timer=ledtimer
        )

    async def async_get_status(self):
        """Get the status of the device."""
        if not self._client or not self._client.is_connected:
            _LOGGER.debug("Client not connected, trying to connect")
            if not await self.connect():
                _LOGGER.error("Failed to connect to device")
                return None

        status = {}
        notification_queue = asyncio.Queue()
        request_402 = asyncio.Event()
        request_403 = asyncio.Event()
        request_404 = asyncio.Event()

        async def notification_handler(sender, data):
            if self.parse_packet(data):
                status['status'] = self.parse_packet(data)
                request_402.set()
            elif self.parse_packet402(data):
                status['status402'] = self.parse_packet402(data)
                request_403.set()
            elif self.parse_packet403(data):
                status['status403'] = self.parse_packet403(data)
                request_404.set()
            elif self.parse_packet404(data):
                status['status404'] = self.parse_packet404(data)

        await self.start_notifications(notification_handler)

        try:
            await self._send_command(CMD_HOOD_STATUS, [0])
            await asyncio.wait_for(request_402.wait(), timeout=5)

            await self._send_command(CMD_HOOD_STATUS_402, [0])
            await asyncio.wait_for(request_403.wait(), timeout=5)

            await self._send_command(CMD_HOOD_STATUS_403, [0])
            await asyncio.wait_for(request_404.wait(), timeout=5)

            await self._send_command(CMD_HOOD_STATUS_404, [0])
            # Wait a bit for the last notification to come in
            await asyncio.sleep(1)

        except asyncio.TimeoutError:
            _LOGGER.error("Timeout waiting for notification")
            return None
        finally:
            await self.stop_notifications()

        return status
