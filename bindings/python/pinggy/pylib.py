
import errno
import ctypes
import os
import threading

from . import core


def setLogPath(path):
    path = path if isinstance(path, bytes) else path.encode("utf-8")
    core.pinggy_set_log_path(path)

def disableLog():
    core.pinggy_set_log_enable(False)


class Channel:
    def __init__(self, channelRef):
        self.__channelRef       = channelRef
        self.__data_received_cb = core.pinggy_channel_data_received_cb_t(self.__func_data_received)
        self.__ready_to_send_cb = core.pinggy_channel_ready_to_send_cb_t(self.__func_ready_to_send)
        self.__error_cb         = core.pinggy_channel_error_cb_t(self.__func_error)
        self.__cleanup_cb       = core.pinggy_channel_cleanup_cb_t(self.__func_cleanup)

        if not core.pinggy_tunnel_channel_set_data_received_callback(self.__channelRef, self.__data_received_cb, None):
            print(f"Could not setup callback `pinggy_channel_data_received_cb_t` for channel {self.__channelRef}")
        if not core.pinggy_tunnel_channel_set_ready_to_send_callback(self.__channelRef, self.__ready_to_send_cb, None):
            print(f"Could not setup callback `pinggy_channel_ready_to_send_cb_t` for channel {self.__channelRef}")
        if not core.pinggy_tunnel_channel_set_error_callback(self.__channelRef, self.__error_cb, None):
            print(f"Could not setup callback `pinggy_channel_error_cb_t` for channel {self.__channelRef}")
        if not core.pinggy_tunnel_channel_set_cleanup_callback(self.__channelRef, self.__cleanup_cb, None):
            print(f"Could not setup callback `pinggy_channel_cleanup_cb_t` for channel {self.__channelRef}")

    def __func_data_received(self, userdata, channelRef):
        assert channelRef == self.__channelRef
    def __func_ready_to_send(self, userdata, channelRef, bufferLen):
        assert channelRef == self.__channelRef
    def __func_error(self, userdata, channelRef, errStr, errLen):
        assert channelRef == self.__channelRef
    def __func_cleanup(self, userdata, channelRef):
        assert channelRef == self.__channelRef

    def accept(self):
        return core.pinggy_tunnel_channel_accept(self.__channelRef)
    def reject(self, val="unknown"):
        val = val if isinstance(val, bytes) else val.encode("utf-8")
        return core.pinggy_tunnel_channel_reject(self.__channelRef, val)
    def close(self):
        return core.pinggy_tunnel_channel_close(self.__channelRef)
    def send(self, data):
        assert isinstance(data, bytes)
        return core.pinggy_tunnel_channel_send(self.__channelRef, data, len(data))
    def recv(self, ln):
        buf = bytes(ln)
        return core.pinggy_tunnel_channel_recv(self.__channelRef, buf, ln)
    def have_data_to_read(self):
        return core.pinggy_tunnel_channel_have_data_to_recv(self.__channelRef)
    def have_buffer_to_write(self):
        return core.pinggy_tunnel_channel_have_buffer_to_send(self.__channelRef)
    def is_connected(self):
        return core.pinggy_tunnel_channel_is_connected(self.__channelRef)
    def get_type(self):
        return core.pinggy_tunnel_channel_get_type(self.__channelRef)
    def get_dest_port(self):
        return core.pinggy_tunnel_channel_get_dest_port(self.__channelRef)
    def get_dest_host(self):
        return core._get_string_via_cfunc(core.pinggy_tunnel_channel_get_dest_host, self.__channelRef)
    def get_src_port(self):
        return core.pinggy_tunnel_channel_get_src_port(self.__channelRef)
    def get_src_host(self):
        return core._get_string_via_cfunc(core.pinggy_tunnel_channel_get_src_host, self.__channelRef)

class BaseTunnelHandler:
    def __init__(self, tunnel):
        self.tunnel = tunnel
    def get_tunnel(self):
        return self.tunnel
    def authenticated(self):
        print(f"Tunnel authenthicated")
    def authentication_failed(self, errors):
        print(f"Tunnel is failed to authenticate. reasons: {errors}")
    def primary_forwarding_succeeded(self):
        print(f"Forwarding succeeded. urls: {self.tunnel.urls}")
    def primary_forwarding_failed(self, msg):
        print(f"Forwarding failed with msg {msg}")
    def additional_forwarding_succeeded(self, bindAddr, forwardTo):
        print(f"Additional forwarding from {bindAddr} to {forwardTo} succeeded")
    def additional_forwarding_failed(self, bindAddr, forwardTo, err):
        print(f"Additional forwarding from {bindAddr} to {forwardTo} failed with error {err}")
    def disconnected(self, msg):
        print(f"Tunnel disconnected with msg {msg}")
    def tunnel_error(self, errorNo, msg, recoverable):
        print(f"Tunnel error occured {errorNo}, {msg}, {recoverable}")
    def handle_channel(self):
        return False
    def new_channel(self, channel:Channel):
        print(f"New channel received. rejecting it. override `new_channel` method to handle the channel or return `False` from `handle_channel` method")
        channel.reject()

class Tunnel:
    def __init__(self, server_address="a.pinggy.io:443", eventClass=BaseTunnelHandler):
        server_address = server_address if isinstance(server_address, bytes) else server_address.encode("utf-8")
        self.__tunnelRef                            = 0
        self.__resumable                            = False
        self.__authenticated_cb                     = core.pinggy_authenticated_cb_t(self.__func_authenticated)
        self.__authentication_failed_cb             = core.pinggy_authentication_failed_cb_t(self.__func_authentication_failed)
        self.__primary_forwarding_succeeded_cb      = core.pinggy_primary_forwarding_succeeded_cb_t(self.__func_primary_forwarding_succeeded)
        self.__primary_forwarding_failed_cb         = core.pinggy_primary_forwarding_failed_cb_t(self.__func_primary_forwarding_failed)
        self.__additional_forwarding_succeeded_cb   = core.pinggy_additional_forwarding_succeeded_cb_t(self.__func_additional_forwarding_succeeded)
        self.__additional_forwarding_failed_cb      = core.pinggy_additional_forwarding_failed_cb_t(self.__func_additional_forwarding_failed)
        self.__disconnected_cb                      = core.pinggy_disconnected_cb_t(self.__func_disconnected)
        self.__tunnel_error_cb                      = core.pinggy_tunnel_error_cb_t(self.__func_tunnel_error)
        self.__new_channel_cb                       = core.pinggy_new_channel_cb_t(self.__func_new_channel)

        self.__configRef                            = core.pinggy_create_config()
        self.__tunnelRef                            = core.pinggy_tunnel_initiate(self.__configRef)
        self.__auto                                 = False

        self.__connected                            = False
        self.__authenticated                        = False
        self.__tunnel_started                       = False

        self.__continue_polling                     = True

        self.__lock                                 = threading.Lock()
        self.__editableConfig                       = True

        self.__urls                                 = []
        self.authentication_messages                = []
        self.tunnel_statup_messages                 = []
        self.server_address                         = server_address

        self.__eventHandler                         = eventClass(self)

        self.__setup_callbacks()

    def __setup_callbacks(self):
        print("Setting up callback")
        if not core.pinggy_tunnel_set_authenticated_callback(self.__tunnelRef, self.__authenticated_cb, None):
            print(f"Could not setup callback for `pinggy_set_authenticated_callback`")
        if not core.pinggy_tunnel_set_authentication_failed_callback(self.__tunnelRef, self.__authentication_failed_cb, None):
            print(f"Could not setup callback for `pinggy_set_authenticationFailed_callback`")
        if not core.pinggy_tunnel_set_primary_forwarding_succeeded_callback(self.__tunnelRef, self.__primary_forwarding_succeeded_cb, None):
            print(f"Could not setup callback for `pinggy_tunnel_set_primary_forwarding_succeeded_callback`")
        if not core.pinggy_tunnel_set_primary_forwarding_failed_callback(self.__tunnelRef, self.__primary_forwarding_failed_cb, None):
            print(f"Could not setup callback for `pinggy_tunnel_set_primary_forwarding_failed_callback`")
        if not core.pinggy_tunnel_set_additional_forwarding_succeeded_callback(self.__tunnelRef, self.__additional_forwarding_succeeded_cb, None):
            print(f"Could not setup callback for `pinggy_tunnel_set_additional_forwarding_succeeded_callback`")
        if not core.pinggy_tunnel_set_additional_forwarding_failed_callback(self.__tunnelRef, self.__additional_forwarding_failed_cb, None):
            print(f"Could not setup callback for `pinggy_tunnel_set_additional_forwarding_failed_callback`")
        if not core.pinggy_tunnel_set_disconnected_callback(self.__tunnelRef, self.__disconnected_cb, None):
            print(f"Could not setup callback for `pinggy_set_disconnected_callback`")
        if not core.pinggy_tunnel_set_tunnel_error_callback(self.__tunnelRef, self.__tunnel_error_cb, None):
            print(f"Could not setup callback for `pinggy_set_tunnel_error_callback`")
        if not core.pinggy_tunnel_set_new_channel_callback(self.__tunnelRef, self.__new_channel_cb, None):
            print(f"Could not setup callback for `pinggy_tunnel_set_new_channel_callback`")


    def __del__(self): #TODO stop tunnel if it is not already
        if self.__configRef is not None:
            core.pinggy_free_ref(self.__configRef)
        if self.__tunnelRef:
            if core.pinggy_free_ref(self.__tunnelRef) == 0:
                print("Could not free")
            self.__tunnelRef = 0

    def start_with_c(self):
        self.__editableConfig = False
        print("Kindly don't use this method")
        core.pinggy_tunnel_start(self.__tunnelRef)

    def start(self):
        self.__editableConfig = False
        self.__auto = True
        self.connect()

    def connect(self):
        if self.__connected:
            raise Exception("You call connec only once")
        locked = False
        if not self.__auto:
            if not self.__lock.acquire(False):
                raise Exception("Synchronization error")
            locked = True
        self.__editableConfig = False
        self.__connected = True
        self.__resumable = core.pinggy_tunnel_connect(self.__tunnelRef)
        if self.__resumable:
            self.__resume()
        if locked:
            self.__lock.release()
        return self.__authenticated

    def stop(self):
        core.pinggy_tunnel_stop(self.__tunnelRef)

    def start_web_debugging(self, port=4300):
        return core.pinggy_tunnel_start_web_debugging(self.__tunnelRef, port)

    def request_primary_forwarding(self):
        if self.__auto:
            raise Exception("Not permitted as tunnel started with `start` method")
        if not self.__authenticated:
            raise Exception("Connect the tunnel first")
        locked = False
        if not self.__auto:
            if not self.__lock.acquire(False):
                raise Exception("Synchronization error")
            locked = True
        self.__continue_polling = True
        core.pinggy_tunnel_request_primary_forwarding(self.__tunnelRef)
        self.__resume()
        if locked:
            self.__lock.release()
        return self.__tunnel_started

    def request_additional_forwarding(self, bindAddr, forwardTo):
        bindAddr = bindAddr if isinstance(bindAddr, bytes) else bindAddr.encode('utf-8')
        forwardTo = forwardTo if isinstance(forwardTo, bytes) else forwardTo.encode('utf-8')
        core.pinggy_tunnel_request_additional_forwarding(self.__tunnelRef, bindAddr, forwardTo)

    def serve_tunnel(self):
        if not self.__tunnel_started:
            raise Exception("Tunnel is not running")
        locked = False
        if not self.__auto:
            if not self.__lock.acquire(False):
                raise Exception("Synchronization error")
            locked = True
        self.__continue_polling = True
        self.__resume()
        if locked:
            self.__lock.release()

    def __resume(self):
        if not self.__resumable:
            raise Exception("Tunnel is not resumable")
        while self.__continue_polling:
            ret = core.pinggy_tunnel_resume(self.__tunnelRef)
            if ret == 0:
                continue
            error = ctypes.get_errno()
            if error != errno.EINTR:
                self.__resumable = False
                return

    def __func_authenticated(self, userdata, ref):
        self.__authenticated = True
        if self.__auto:
            core.pinggy_tunnel_request_primary_forwarding(self.__tunnelRef)
        else:
            self.__continue_polling = False
        self.__eventHandler.authenticated()
        # print(f"AuthenticatedFunc: Reference: {ref}")

    def __func_authentication_failed(self, userdata, ref, l, arr):
        if not self.__auto:
            self.__continue_polling = False
        self.authentication_messages = core._getStringArray(l, arr)
        self.__eventHandler.authentication_failed(core._getStringArray(l, arr))
        # print(f"AuthenticationFailedFunc: Reference: {ref} {l} {arr} {core._getStringArray(l, arr)}")

    def __func_primary_forwarding_succeeded(self, userdata, ref, l, arr):
        self.tunnel_statup_messages = core._getStringArray(l, arr)
        if not self.__auto:
            self.__continue_polling = False
        self.__tunnel_started = True
        self.__urls = core._getStringArray(l, arr)
        self.__eventHandler.primary_forwarding_succeeded()
        # print(f"PrimaryForwardingSucceeded: Reference: {ref} {l} {arr} {core._getStringArray(l, arr)}")

    def __func_primary_forwarding_failed(self, userdata, ref, msg):
        self.tunnel_statup_messages = [msg.decode('utf-8')]
        if not self.__auto:
            self.__continue_polling = False
        self.__eventHandler.primary_forwarding_failed(msg)
        # print(f"PrimaryForwardingFailed: Reference: {ref} {msg}")

    def __func_additional_forwarding_succeeded(self, userdata, ref, bindAddr, forwardTo):
        bindAddr = bindAddr.decode('utf-8')
        forwardTo = forwardTo.decode('utf-8')
        self.__eventHandler.additional_forwarding_succeeded(bindAddr, forwardTo)
        # print(f"RemoteFowardingSucceeded: Reference: {ref} `{bindAddr}` `{forwardTo}`")

    def __func_additional_forwarding_failed(self, userdata, ref, bindAddr, forwardTo, err):
        bindAddr = bindAddr.decode('utf-8')
        forwardTo = forwardTo.decode('utf-8')
        err = err.decode('utf-8')
        self.__eventHandler.additional_forwarding_failed(bindAddr, forwardTo, err)
        # print(f"RemoteFowardingSucceeded: Reference: {ref} `{bindAddr}` `{forwardTo}` `{err}`")

    def __func_disconnected(self, userdata, ref, msg, l, arr):
        self.__continue_polling = False
        self.__resumable = False
        self.__eventHandler.disconnected(msg.decode('utf-8'))
        # print(f"DisconnectedFunc: Reference: {ref} {msg} {l} {arr} {core._getStringArray(l, arr)}")

    def __func_tunnel_error(self, userdata, ref, errorNo, msg, recoverable):
        # print(f"DisconnectedFunc: Reference: {ref} {msg} {l} {arr} {core._getStringArray(l, arr)}")
        self.__eventHandler.tunnel_error(errorNo, msg, recoverable)

    def __func_new_channel(self, userdata, ref, chanRef):
        print(f"Channel received") # todo
        if not self.__eventHandler.handle_channel():
            return False
        channel = Channel(chanRef)
        self.__eventHandler.new_channel(channel)
        return True


    #////////////////////
    @property
    def urls(self):
        return self.__urls

    @property
    def server_address(self):
        return core._get_string_via_cfunc(core.pinggy_config_get_server_address, self.__configRef)

    @property
    def token(self):
        return core._get_string_via_cfunc(core.pinggy_config_get_token, self.__configRef)

    @property
    def type(self):
        return core._get_string_via_cfunc(core.pinggy_config_get_type, self.__configRef)

    @property
    def udp_type(self):
        return core._get_string_via_cfunc(core.pinggy_config_get_udp_type, self.__configRef)

    @property
    def tcp_forward_to(self):
        return core._get_string_via_cfunc(core.pinggy_config_get_tcp_forward_to, self.__configRef)

    @property
    def udp_forward_to(self):
        return core._get_string_via_cfunc(core.pinggy_config_get_udp_forward_to, self.__configRef)

    @property
    def force(self):
        return core.pinggy_config_get_force(self.__configRef)

    @property
    def argument(self):
        return core._get_string_via_cfunc(core.pinggy_config_get_argument, self.__configRef)

    @property
    def advanced_parsing(self):
        return core.pinggy_config_get_advanced_parsing(self.__configRef)

    @property
    def ssl(self):
        return core.pinggy_config_get_ssl(self.__configRef)

    @property
    def sni_server_name(self):
        return core._get_string_via_cfunc(core.pinggy_config_get_sni_server_name, self.__configRef)

    @property
    def insecure(self):
        return core.pinggy_config_get_insecure(self.__configRef)

    #////////////////////////////////

    @server_address.setter
    def server_address(self, val):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        val = val if isinstance(val, bytes) else val.encode("utf-8")
        core.pinggy_config_set_server_address(self.__configRef, val)

    @token.setter
    def token(self, val):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        val = val if isinstance(val, bytes) else val.encode("utf-8")
        core.pinggy_config_set_token(self.__configRef, val)

    @type.setter
    def type(self, val):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        val = val if isinstance(val, bytes) else val.encode("utf-8")
        core.pinggy_config_set_type(self.__configRef, val)

    @udp_type.setter
    def udp_type(self, val):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        val = val if isinstance(val, bytes) else val.encode("utf-8")
        core.pinggy_config_set_udp_type(self.__configRef, val)

    @tcp_forward_to.setter
    def tcp_forward_to(self, val):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        val = val if isinstance(val, bytes) else val.encode("utf-8")
        core.pinggy_config_set_tcp_forward_to(self.__configRef, val)

    @udp_forward_to.setter
    def udp_forward_to(self, val):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        val = val if isinstance(val, bytes) else val.encode("utf-8")
        core.pinggy_config_set_udp_forward_to(self.__configRef, val)

    @force.setter
    def force(self, val):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        core.pinggy_config_set_force(self.__configRef, val)

    @argument.setter
    def argument(self, val):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        val = val if isinstance(val, bytes) else val.encode("utf-8")
        core.pinggy_config_set_argument(self.__configRef, val)

    @advanced_parsing.setter
    def advanced_parsing(self, val):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        core.pinggy_config_set_advanced_parsing(self.__configRef, val)

    @ssl.setter
    def ssl(self, val):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        core.pinggy_config_set_ssl(self.__configRef, val)

    @sni_server_name.setter
    def sni_server_name(self, val):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        val = val if isinstance(val, bytes) else val.encode("utf-8")
        core.pinggy_config_set_sni_server_name(self.__configRef, val)

    @insecure.setter
    def insecure(self, val):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        core.pinggy_config_set_insecure(self.__configRef, val)

# core.pinggy_set_log_path(b"/tmp/asd")

