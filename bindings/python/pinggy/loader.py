import ctypes
import platform
import os

__dllPath = ""
__dllPrefixLocation="releases"

if platform.system() == "Windows":
    # os.add_dll_directory("c:\\Program Files\\OpenSSL-Win64")
    __dllPath = os.path.join(os.path.dirname(__file__), "..\\..\\..\\pinggy.dll")
elif platform.system() == "Linux":
    if platform.machine() in ["x86_64", "i686", "aarch64"]:
        __dllPath = os.path.join(os.path.dirname(__file__), __dllPrefixLocation, "linux/"+platform.machine()+"/libpinggy.so")
    if __dllPath == "" or not os.path.exists(__dllPath):
        __dllPath = os.path.join(os.path.dirname(__file__), "../../../libpinggy.so")
elif platform.system() == "Darwin":
    if platform.machine() in ["x86_64", "arm64"]:
        __dllPath = os.path.join(os.path.dirname(__file__), __dllPrefixLocation, "macos/universal/libpinggy.dylib")
    if __dllPath == "" or not os.path.exists(__dllPath):
        __dllPath = os.path.join(os.path.dirname(__file__), "../../../libpinggy.dylib")
else:
    print("Unsupported platform")
    exit(1)

if not os.path.exists(__dllPath):
    print(f"Required dll does not exists at the path: `{__dllPath}`")
    exit(1)

try:
    cdll = ctypes.CDLL(__dllPath)
except Exception as e:
    print(f"Required dependencies not found. Make sure that the dependencies (OpenSSL) are there. {e}")
    exit(1)
