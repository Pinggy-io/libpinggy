import ctypes
import platform
import os
import sys
import urllib
import urllib.request
import tempfile
import ssl
import zipfile
import tarfile

from . import __version__ as version
from . import pinggyexception

PinggyNativeLoaderError = pinggyexception.PinggyNativeLoaderError


lib_version = version.__lib_pinggy_version
base_url = f"{version.__lib_pinggy_url_prefix}{lib_version}"

def get_architecture():
    machine = platform.machine().lower()

    if machine in ('x86_64', 'amd64'):
        return 'x86_64'
    elif machine in ('i386', 'i686', 'x86'):
        return 'i686'
    elif machine in ('aarch64', 'arm64'):
        return 'aarch64'
    elif machine.startswith('armv7'):
        return 'armv7'
    else:
        return machine

def load_native_release():
    # Determine OS and architecture
    system = platform.system().lower()
    machine = get_architecture()
    script_dir = os.path.dirname(os.path.abspath(__file__)) + "/../../.."

    caching_dir_path = {
        "linux":   f"{script_dir}/releases/{system}/{machine}",
        "darwin":  f"{script_dir}/releases/macos/universal",
        "windows": f"{script_dir}/releases/{system}/{machine}",
    }.get(system)

    # caching_dir_path = f"{script_dir}/releases/{system}/{machine}"


    # Mapping system to correct shared library name
    lib_path = {
        "windows": f"{caching_dir_path}/pinggy.dll",
        "linux":   f"{caching_dir_path}/libpinggy.so",
        "darwin":  f"{caching_dir_path}/libpinggy.dylib",
    }.get(system)

    # Ensure the shared library exists
    if not os.path.exists(lib_path):
        sys.exit(f"Shared library missing: `{lib_path}`")

    # Load the shared library
    try:
        cdll = ctypes.CDLL(lib_path)
        return cdll
    except Exception as err:
        raise PinggyNativeLoaderError(f"Failed to load shared library. Ensure dependencies like OpenSSL are installed if required. {err}")


cdll = load_native_release()
