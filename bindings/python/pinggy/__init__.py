"""
Pinggy Python SDK.

This sdk provide functionalities to create pinggy tunnel and forward local service to the Internet.

For more details, visit https://pinggy.io.

The python SDK is a wrapper arround the C library `libpinggy`. This module wraps `libpinggy` and
provides easy interfact using Tunnel class.

There are two simple way to start a tunnel. If we want to forward local apache server listening on
port 80 to the internet we can start tunnel via following:

Example 1:

    >>> import pinggy
    >>> tunnel = pinggy.start_tunnel(80)
    >>> print(tunnel.url)

Example 2:

    >>> import pinggy
    >>> tunnel = pinggy.Tunnel()
    >>> tunnel.tcp_forward_to = "localhost:80"
    >>> tunnel.start()

Example 3:

    >>> import pinggy
    >>> tunnel = pinggy.Tunnel()
    >>> tunnel.tcp_forward_to = "localhost:80"
    >>> tunnel.connect()
    >>> tunnel.request_primary_forwarding()
    >>> print(tunnel.url)
    >>> tunnel.start()

"""

from .pylib import Tunnel, Channel, BaseTunnelHandler, \
        setLogPath, disableLog, set_log_path, disable_log, \
        version, git_commit, build_timestamp, libc_version, \
        build_os, start_tunnel, start_udptunnel, enable_log

from .pinggyexception import  PinggyNativeLoaderError

# Specify the public API of the module
__all__ = [
    "Tunnel",
    "Channel",
    "BaseTunnelHandler",
    "start_tunnel",
    "start_udptunnel",
    "setLogPath",
    "disableLog",
    "set_log_path",
    "disable_log",
    "enable_log",
    "version",
    "git_commit",
    "build_timestamp",
    "libc_version",
    "build_os",
    "PinggyNativeLoaderError"
]
