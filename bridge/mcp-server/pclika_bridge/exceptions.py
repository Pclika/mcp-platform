"""Pclika Bridge exception types."""


class PclikaError(Exception):
    """Base exception for all Pclika bridge errors."""


class DeviceNotConnected(PclikaError):
    """Raised when no device is connected or connection is lost."""


class DeviceTimeout(PclikaError):
    """Raised when a device command times out."""


class DeviceError(PclikaError):
    """Raised when the device returns an error response."""

    def __init__(self, code: str, message: str):
        self.code = code
        self.message = message
        super().__init__(f"[{code}] {message}")


class ToolNotSupported(PclikaError):
    """Raised when the connected device does not support a requested tool."""


class InvalidParameter(PclikaError):
    """Raised when a tool is called with invalid parameters."""


class ProtocolError(PclikaError):
    """Raised when a malformed or unexpected protocol message is received."""
