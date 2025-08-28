import errno
import ctypes
import threading
import shlex
import threading
import json

from . import pinggyexception

try:
    from . import core
except pinggyexception.PinggyNativeLoaderError as e:
    class DummyCore:
        def __init__(self, e):
            self.loading_exception = e
        def disable_sdk_log(self):
            pass
        def __getattr__(self, name):
                raise self.loading_exception

        def __call__(self, *args, **kwargs):
            raise self.loading_exception

    core = DummyCore(e)

core.disable_sdk_log()

def set_log_path(path):
    """
    Set path where native library print its log. Use this function only if requires.
    To disable native library logging completly, use `disableLog` function.

    Args:
        path (str): New log path. Path needs to have write permission.
    """
    path = path if isinstance(path, bytes) else path.encode("utf-8")
    core.pinggy_set_log_path(path)

def disable_log():
    """
    Disable logging by the native library.
    """
    core.pinggy_set_log_enable(False)

def enable_log():
    core.pinggy_set_log_enable(True)

setLogPath = set_log_path
disableLog = disable_log

def version():
    """
    Function to know the native library version.

    Returns:
        str: libpinggy version.
    """
    return core._get_string_via_cfunc(core.pinggy_version)

def git_commit():
    """
    Function to get the git commit hash of the source code.

    Returns:
        str: git commit hash.
    """
    return core._get_string_via_cfunc(core.pinggy_git_commit)

def build_timestamp():
    """
    Function to get the build timestamp as per the build-system.

    Returns:
        str: build timestamp.
    """
    return core._get_string_via_cfunc(core.pinggy_build_timestamp)

def libc_version():
    """
    Get the libc version of the native. This information is accurate only for linux operating system.

    Returns:
        str: libc version.
    """
    return core._get_string_via_cfunc(core.pinggy_libc_version)

def build_os():
    """
    Get the detail about the build operating system.

    Returns:
        str: os detail.
    """
    return core._get_string_via_cfunc(core.pinggy_build_os)


class Channel:
    """
    Represents incomming channels from the tunnel.

    **This feature is not finished and not to be used**
    """
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
    """
    Represent basic and default handler for :class:`Tunnel`. It provide default handler
    for various event triggered by the Tunnel. It is expected that all the event handler
    would extend this event handler.
    """
    def __init__(self, tunnel):
        """
        Initializes the basic event handler.
        Args:
            tunnel (Tunnel): The tunnel object
        """
        self.tunnel = tunnel

    def get_tunnel(self):
        """
        Returns the tunnel object
        Returns:
            Tunnel: the tunnel object
        """
        return self.tunnel

    def connected(self):
        """
        Triggers when tunnel successfully connected. it is probably is not required at all.
        """

    def authenticated(self):
        """
        Triggers when tunnel successfully authenticated. Authentication happen even for free tunnels.
        """
        # print(f"Tunnel authenticated")

    def authentication_failed(self, errors):
        """
        Triggers when tunnel could not able to authenticate it self. Reasons are provided in the `errors` argument.
        Any further action on the tunnel object will fail.

        Args:
            errors (list(str)): Authentication failure reasons.
        """
        print(f"Tunnel is failed to authenticate. reasons: {errors}")

    def primary_forwarding_succeeded(self):
        """
        Triggers when primary (or default) forwarding successfully completed.
        Know more about primary (or default) forwarding at
        https://pinggy.io/docs/http_tunnels/multi_port_forwarding/.

        Once this step done, one can fetch the urls from the tunnel.
        """
        # print(f"Forwarding succeeded. urls: {self.tunnel.urls}")

    def primary_forwarding_failed(self, msg):
        """
        Triggers when primary (or default) forwarding fails. The reason is present in the msg.

        Agrs:
            msg (str): the reason why it failes.
        """
        print(f"Forwarding failed with msg {msg}")

    def additional_forwarding_succeeded(self, bindAddr, forwardTo):
        """
        Triggers when additional forwarding completes successfully. Learn more at
        https://pinggy.io/docs/http_tunnels/multi_port_forwarding/.

        **This is experimental and not tested**

        Agrs:
            bindAddr (str): remote address where connection can be sent.
            forwardTo (str): address to which connection would forwarded. It is equivalen to `tcp_forward_to`.
        """
        print(f"Additional forwarding from {bindAddr} to {forwardTo} succeeded")

    def additional_forwarding_failed(self, bindAddr, forwardTo, err):
        """
        Triggers when additional forwarding fails
        """
        print(f"Additional forwarding from {bindAddr} to {forwardTo} failed with error {err}")

    def disconnected(self, msg):
        """
        Triggers when tunnel got disconnected by the server.

        Agrs:
            msg (str): disconnection reason.
        """
        print(f"Tunnel disconnected with msg {msg}")

    def tunnel_error(self, errorNo, msg, recoverable):
        """
        In case some error occures. Errors could be recoverable.

        Args:
            errorNo (int): internal error no. Currently not useful for user.
            msg (str): description
            recoverable (bool): whether a error is recoverable or not. Application should ignore recoverable errors.
        """
        print(f"Tunnel error occured {errorNo}, {msg}, {recoverable}")

    def handle_channel(self):
        """
        **Do not return anything but False**
        """
        return False

    def new_channel(self, channel:Channel):
        """
        **Do not use**
        """
        print(f"New channel received. rejecting it. override `new_channel` method to handle the channel or return `False` from `handle_channel` method")
        channel.reject()

    def will_reconnect(self, messages):
        pass
    def reconnecting(self, retry_cnt):
        pass
    def reconnection_completed(self):
        pass
    def reconnection_failed(self, retry_cnt):
        pass
    def usage_update(self, usages):
        pass

class Tunnel:
    """
    The primary class which provides the tunnel.

    There are two simple way to start a tunnel. If we want to forward local apache server listening on
    port 80 to the internet we can start tunnel via following:

    Example 1:

        >>> import pinggy
        >>> tunnel = pinggy.Tunnel()
        >>> tunnel.tcp_forward_to = "localhost:80"
        >>> tunnel.start()

    Example 2:

        >>> import pinggy
        >>> tunnel = pinggy.Tunnel()
        >>> tunnel.tcp_forward_to = "localhost:80"
        >>> tunnel.connect()
        >>> tunnel.request_primary_forwarding()
        >>> tunnel.serve_tunnel()

    There several configuration available, that one might need to consider.

    Flow 1:

        > Create Tunnel
        >         |
        >         |-> set attributes
        >         |
        >         |-> connect() -> authentication failed callback
        >         |       |
        >         |       `-> authentication success callback
        >         |
        >         |-> request_primary_forwarding() -> primary forwarding failed callback
        >         |       |
        >         |       `-> primary forwarding succeeded callback
        >         |
        >         |-> request_additional_forwarding(bindaddress, forwardto) -> additional forwarding failed callback
        >         |       |
        >         |       `-> additional forwarding succeeded callback
        >         |
        >         `-> start()

    Flow 2:

        > Create Tunnel
        >         |
        >         |-> set attributes
        >         |
        >         |-> start() -> authentication failed callback
        >                 |
        >                 `-> authentication success callback -> primary forwarding failed callback
        >                             |
        >                             `-> primary forwarding succeeded callback
    """
    def __init__(self, server_address="a.pinggy.io:443", type="", tcp_forward_to=None, udp_forward_to=None, eventClass=BaseTunnelHandler):
        server_address = server_address if isinstance(server_address, bytes) else server_address.encode("utf-8")
        self.__tunnelRef                            = 0
        self.__resumable                            = False
        self.__connected_cb                         = core.pinggy_on_connected_cb_t(self.__func_connected)
        self.__authenticated_cb                     = core.pinggy_on_authenticated_cb_t(self.__func_authenticated)
        self.__authentication_failed_cb             = core.pinggy_on_authentication_failed_cb_t(self.__func_authentication_failed)
        self.__primary_forwarding_succeeded_cb      = core.pinggy_on_primary_forwarding_succeeded_cb_t(self.__func_primary_forwarding_succeeded)
        self.__primary_forwarding_failed_cb         = core.pinggy_on_primary_forwarding_failed_cb_t(self.__func_primary_forwarding_failed)
        self.__additional_forwarding_succeeded_cb   = core.pinggy_on_additional_forwarding_succeeded_cb_t(self.__func_additional_forwarding_succeeded)
        self.__additional_forwarding_failed_cb      = core.pinggy_on_additional_forwarding_failed_cb_t(self.__func_additional_forwarding_failed)
        self.__disconnected_cb                      = core.pinggy_on_disconnected_cb_t(self.__func_disconnected)
        self.__tunnel_error_cb                      = core.pinggy_on_tunnel_error_cb_t(self.__func_tunnel_error)
        self.__new_channel_cb                       = core.pinggy_on_new_channel_cb_t(self.__func_new_channel)
        self.__will_reconnect_cb                    = core.pinggy_on_will_reconnect_cb_t(self.__func_will_reconnect)
        self.__reconnecting_cb                      = core.pinggy_on_reconnecting_cb_t(self.__func_reconnecting)
        self.__reconnection_completed_cb            = core.pinggy_on_reconnection_completed_cb_t(self.__func_reconnection_completed)
        self.__reconnection_failed_cb               = core.pinggy_on_reconnection_failed_cb_t(self.__func_reconnection_failed)
        self.__usage_update_cb                      = core.pinggy_on_usage_update_cb_t(self.__func_usage_update)

        self.__configRef                            = core.pinggy_create_config()
        self.__tunnelRef                            = core.pinggy_tunnel_initiate(self.__configRef)

        self.__connected                            = False
        self.__authenticated                        = False
        self.__tunnel_started                       = False

        self.__continue_polling                     = True
        self.__auto                                 = False

        self.__lock                                 = threading.Lock()
        self.__editableConfig                       = True

        self.__urls                                 = []
        self.authentication_messages                = []
        self.tunnel_statup_messages                 = []
        self.server_address                         = server_address

        self.__cmd                                  = ""
        self.__localservertls                       = None
        self.__ipwhitelist                          = None
        self.__basicauth                            = None
        self.__bearerauth                           = None
        self.__headermodification                   = None
        self.__xff                                  = False
        self.__httpsonly                            = False
        self.__fullrequesturl                       = False
        self.__allowpreflight                       = False
        self.__reverseproxy                         = True

        if tcp_forward_to is not None:
            self.tcp_forward_to                     = tcp_forward_to
        if udp_forward_to is not None:
            self.udp_forward_to                     = udp_forward_to
        if type != "":
            self.type                               = type

        self.__eventHandler                         = eventClass(self)

        self.__thread                               = None

        self.__setup_callbacks()

    def __setup_callbacks(self):
        # print("Setting up callback")
        if not core.pinggy_tunnel_set_on_connected_callback(self.__tunnelRef, self.__connected_cb, None):
            print(f"Could not setup callback for `pinggy_set_connected_callback`")
        if not core.pinggy_tunnel_set_on_authenticated_callback(self.__tunnelRef, self.__authenticated_cb, None):
            print(f"Could not setup callback for `pinggy_set_authenticated_callback`")
        if not core.pinggy_tunnel_set_on_authentication_failed_callback(self.__tunnelRef, self.__authentication_failed_cb, None):
            print(f"Could not setup callback for `pinggy_set_authenticationFailed_callback`")
        if not core.pinggy_tunnel_set_on_primary_forwarding_succeeded_callback(self.__tunnelRef, self.__primary_forwarding_succeeded_cb, None):
            print(f"Could not setup callback for `pinggy_tunnel_set_primary_forwarding_succeeded_callback`")
        if not core.pinggy_tunnel_set_on_primary_forwarding_failed_callback(self.__tunnelRef, self.__primary_forwarding_failed_cb, None):
            print(f"Could not setup callback for `pinggy_tunnel_set_primary_forwarding_failed_callback`")
        if not core.pinggy_tunnel_set_on_additional_forwarding_succeeded_callback(self.__tunnelRef, self.__additional_forwarding_succeeded_cb, None):
            print(f"Could not setup callback for `pinggy_tunnel_set_additional_forwarding_succeeded_callback`")
        if not core.pinggy_tunnel_set_on_additional_forwarding_failed_callback(self.__tunnelRef, self.__additional_forwarding_failed_cb, None):
            print(f"Could not setup callback for `pinggy_tunnel_set_additional_forwarding_failed_callback`")
        if not core.pinggy_tunnel_set_on_disconnected_callback(self.__tunnelRef, self.__disconnected_cb, None):
            print(f"Could not setup callback for `pinggy_set_disconnected_callback`")
        if not core.pinggy_tunnel_set_on_will_reconnect_callback(self.__tunnelRef, self.__will_reconnect_cb, None):
            print(f"Could not setup callback for `pinggy_tunnel_set_on_will_reconnect_callback`")
        if not core.pinggy_tunnel_set_on_reconnecting_callback(self.__tunnelRef, self.__reconnecting_cb, None):
            print(f"Could not setup callback for `pinggy_tunnel_set_on_reconnecting_callback`")
        if not core.pinggy_tunnel_set_on_reconnection_completed_callback(self.__tunnelRef, self.__reconnection_completed_cb, None):
            print(f"Could not setup callback for `pinggy_tunnel_set_on_reconnection_completed_callback`")
        if not core.pinggy_tunnel_set_on_reconnection_failed_callback(self.__tunnelRef, self.__reconnection_failed_cb, None):
            print(f"Could not setup callback for `pinggy_tunnel_set_on_reconnection_failed_callback`")
        if not core.pinggy_tunnel_set_on_usage_update_callback(self.__tunnelRef, self.__usage_update_cb, None):
            print(f"Could not setup callback for `pinggy_tunnel_set_on_usage_update_callback`")
        if not core.pinggy_tunnel_set_on_tunnel_error_callback(self.__tunnelRef, self.__tunnel_error_cb, None):
            print(f"Could not setup callback for `pinggy_set_tunnel_error_callback`")
        if not core.pinggy_tunnel_set_on_new_channel_callback(self.__tunnelRef, self.__new_channel_cb, None):
            print(f"Could not setup callback for `pinggy_tunnel_set_new_channel_callback`")


    def __del__(self): #TODO stop tunnel if it is not already
        if self.__configRef is not None:
            core.pinggy_free_ref(self.__configRef)
        if self.__tunnelRef:
            if core.pinggy_free_ref(self.__tunnelRef) == 0:
                print("Could not free")
            self.__tunnelRef = 0

    def start_with_c(self):
        """
        ** DO NOT USE THIS METHOD **
        """
        self.__editableConfig = False
        print("Kindly don't use this method")
        core.pinggy_tunnel_start(self.__tunnelRef)

    def start(self, thread=False):
        """
        Start the tunnel with the provided configuration. This is a blocking call.
        It does not return unless tunnel stopped externally or some error occures.

        Args:
            thread (bool): Whether to run the start tunnel in a new thread. Default is False
        """
        self.__editableConfig = False
        self.__auto = True
        if not self.__connected:
            self.__connect_tunnel()
        if self.__authenticated and not self.__tunnel_started:
            self.__internal_request_primary_forwarding()
        if self.__tunnel_started:
            if thread:
                t = threading.Thread(target=self.__start_serving)
                self.__thread = t
                t.start()
            else:
                self.__start_serving()

    def connect(self):
        """
        Connect the tunnel with the server and authenticate it self. It returns true on success.

        If this step fails, no futher step steps can be continued.

        Returns:
            bool: whether authentication done sucessfully or not.
        """

        if self.__auto:
            raise Exception("Not permitted as tunnel started with `start` method")

        return self.__connect_tunnel()

    def __connect_tunnel(self):
        if self.__connected:
            raise Exception("You call connect only once")
        locked = False
        if not self.__lock.acquire(False):
            raise Exception("Synchronization error")
        locked = True

        self.__prepare_n_setargument() #set the cmd and seal it

        self.__editableConfig = False
        self.__connected = True
        self.__resumable = core.pinggy_tunnel_connect(self.__tunnelRef)

        if self.__resumable:
            self.__resume()
        if locked:
            self.__lock.release()
        return self.__authenticated

    def stop(self):
        """Stops the running tunnel."""
        core.pinggy_tunnel_stop(self.__tunnelRef)
        if self.__thread is not None: # only if it is different thread than self.__thread
            if threading.current_thread() != self.__thread:
                self.__thread.join()
                self.__thread = None

    def wait(self):
        """Wait for tunnel to stop. It does not stop the tunnel though."""
        if self.__thread is not None and self.__thread != threading.current_thread():
            self.__thread.join()

    def is_active(self):
        """Check if tunnel is active or not."""
        return core.pinggy_tunnel_is_active(self.__tunnelRef)

    def start_web_debugging(self, port=4300):
        """
        Start the web debugger. All the request would be handled internally.

        Call this function after primary forwarding completed successfully.
        """
        return core.pinggy_tunnel_start_web_debugging(self.__tunnelRef, port)

    def request_primary_forwarding(self):
        """
        Request to start the default forwarding. Once suceeded, user can get
        the urls and tunnel starts accepting requests.
        """

        if self.__auto:
            raise Exception("Not permitted as tunnel started with `start` method")

        return self.__internal_request_primary_forwarding()

    def __internal_request_primary_forwarding(self):
        if not self.__authenticated:
            raise Exception("Connect the tunnel first")
        locked = False
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
        """
        Once primary forwarding is done, user can request additional forwarding for other ports.

        More details at: https://pinggy.io/docs/http_tunnels/multi_port_forwarding/.
        """
        bindAddr = bindAddr if isinstance(bindAddr, bytes) else bindAddr.encode('utf-8')
        forwardTo = forwardTo if isinstance(forwardTo, bytes) else forwardTo.encode('utf-8')
        core.pinggy_tunnel_request_additional_forwarding(self.__tunnelRef, bindAddr, forwardTo)

    def start_usage_update(self):
        """
        Start usage update. It would start puching update via the callback
        """
        core.pinggy_tunnel_start_usage_update(self.__tunnelRef)

    def stop_usage_update(self):
        """
        Stop usages update.
        """
        core.pinggy_tunnel_stop_usage_update(self.__tunnelRef)

    def get_current_usages(self):
        """
        Get the usage.
        """
        usages = core.pinggy_tunnel_get_current_usages(self.__tunnelRef)
        usages = usages.decode("utf-8")
        if usages == "" or usages is None:
            return None
        return json.loads(usages)

    def get_greeting_msgs(self):
        msgs = core.pinggy_tunnel_get_greeting_msgs(self.__tunnelRef)
        msgs = msgs.decode("utf-8")
        if msgs == "" or msgs is None:
            return None
        return json.loads(msgs)

    def serve_tunnel(self):
        """
        Final method in the tunnel creation flow. It is again a blocking call.
        **Deprecated**
        """
        self.__start_serving()

    def __start_serving(self):
        if not self.__tunnel_started:
            raise Exception("Tunnel is not running")
        locked = False
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
            if ret:
                continue
            self.__resumable = False
            return

    def __func_connected(self, userdata, ref):
        self.__eventHandler.connected()
        # print(f"AuthenticatedFunc: Reference: {ref}")

    def __func_authenticated(self, userdata, ref):
        self.__authenticated = True
        self.__continue_polling = False
        self.__eventHandler.authenticated()
        # print(f"AuthenticatedFunc: Reference: {ref}")

    def __func_authentication_failed(self, userdata, ref, l, arr):
        self.__continue_polling = False
        self.authentication_messages = core._getStringArray(l, arr)
        self.__eventHandler.authentication_failed(core._getStringArray(l, arr))
        # print(f"AuthenticationFailedFunc: Reference: {ref} {l} {arr} {core._getStringArray(l, arr)}")

    def __func_primary_forwarding_succeeded(self, userdata, ref, l, arr):
        self.tunnel_statup_messages = core._getStringArray(l, arr)
        self.__continue_polling = False
        self.__tunnel_started = True
        self.__urls = core._getStringArray(l, arr)
        self.__eventHandler.primary_forwarding_succeeded()
        # print(f"PrimaryForwardingSucceeded: Reference: {ref} {l} {arr} {core._getStringArray(l, arr)}")

    def __func_primary_forwarding_failed(self, userdata, ref, msg):
        self.tunnel_statup_messages = [msg.decode('utf-8')]
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

    def __func_new_channel(self, userdata, ref, chan_ref):
        if not self.__eventHandler.handle_channel():
            return False
        channel = Channel(chan_ref)
        self.__eventHandler.new_channel(channel)
        return True

    def __func_will_reconnect(self, user_data, ref, error, l, arr):
        msgs = core._getStringArray(l, arr)
        self.__eventHandler.will_reconnect(msgs)

    def __func_reconnecting(self, user_data, ref, retry_cnt):
        self.__eventHandler.reconnecting(retry_cnt)

    def __func_reconnection_completed(self, user_data, ref, l, arr):
        self.__urls = core._getStringArray(l, arr)
        self.__eventHandler.reconnection_completed()

    def __func_reconnection_failed(self, user_data, ref, retry_cnt):
        self.__eventHandler.reconnection_failed(retry_cnt)

    def __func_usage_update(self, user_data, ref, usages):
        usages = usages.decode('utf-8')
        usages = json.loads(usages)
        self.__eventHandler.usage_update(usages)


    #////////////////////
    @property
    def urls(self):
        """list(str): lists of public urls for the running tunnel (read only)"""
        return self.__urls

    @property
    def server_address(self):
        """
        str: pinggy server address. The default server address is `a.pinggy.io`. You can also add the
            port as follows: `a.pinggy.io:443`.
        """
        return core._get_string_via_cfunc(core.pinggy_config_get_server_address, self.__configRef)

    @property
    def token(self):
        """str: Token for the tunnel. One can it from `dashboard.pinggy.io`"""
        return core._get_string_via_cfunc(core.pinggy_config_get_token, self.__configRef)

    @property
    def type(self):
        """
        str: Tunnel type or mode. This is only for TCP type. So, the accepted values are 'http',
            'tcp', 'tls' and 'tlstcp'. Default is 'http'.
        """
        return core._get_string_via_cfunc(core.pinggy_config_get_type, self.__configRef)

    @property
    def udp_type(self):
        """
        str: Tunnel type or mode. This is only for UDP type. currently, only accepted value is 'udp'.
        """
        return core._get_string_via_cfunc(core.pinggy_config_get_udp_type, self.__configRef)

    @property
    def tcp_forward_to(self):
        """
        str: local server address for default or primary forward. It is equivalent to -R option in ssh

        Example:
            If local server is running at port 8080. Forward request to it by setting

            >>> tunnel.tcp_forward_to = "localhost:8080"
        """
        return core._get_string_via_cfunc(core.pinggy_config_get_tcp_forward_to, self.__configRef)

    @property
    def udp_forward_to(self):
        """
        str: Similar to `tcp_forward_to`. However, it is for udp tunnel.
        """
        return core._get_string_via_cfunc(core.pinggy_config_get_udp_forward_to, self.__configRef)

    @property
    def force(self):
        """bool: force flag in tunnel that terminates any existing tunnel with the same token."""
        return core.pinggy_config_get_force(self.__configRef)

    @property
    def argument(self):
        """str: tunnel arguments for header manipulation and others."""
        return self.__cmd

    @property
    def advanced_parsing(self):
        """
        keep it true. Free tunnels won't work without it.
        """
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

    @property
    def auto_reconnect(self):
        return core.pinggy_config_get_auto_reconnect(self.__configRef)

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
        if type(val) == int:
            val = f"localhost:{val}"
        val = val if isinstance(val, bytes) else val.encode("utf-8")
        core.pinggy_config_set_tcp_forward_to(self.__configRef, val)

    @udp_forward_to.setter
    def udp_forward_to(self, val):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        if type(val) == int:
            val = f"localhost:{val}"
        val = val if isinstance(val, bytes) else val.encode("utf-8")
        core.pinggy_config_set_udp_forward_to(self.__configRef, val)

    @force.setter
    def force(self, val):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        core.pinggy_config_set_force(self.__configRef, val)

    @argument.setter
    def argument(self, val: str):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")

        if type(val) != str:
            raise Exception("Only string is allowed")

        self.__cmd = val

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

    @auto_reconnect.setter
    def auto_reconnect(self, val):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        core.pinggy_config_set_auto_reconnect(self.__configRef, val)

    #//////////////////////

    @property
    def ipwhitelist(self):
        """list[str]|None: List of IP/IP ranges that allowed to connect to the tunnel. SDK does not verify the IP"""
        return self.__ipwhitelist

    @ipwhitelist.setter
    def ipwhitelist(self, ipwhitelist: list[str]|str):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        if type(ipwhitelist) == str:
            ipwhitelist = [ipwhitelist]
        self.__ipwhitelist = ipwhitelist


    @property
    def basicauth(self):
        """dict[str, str]|None: List of username and correstponding password."""
        return self.__basicauth

    @basicauth.setter
    def basicauth(self, basicauth:  dict[str,str]):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        self.__basicauth = basicauth


    @property
    def bearerauth(self):
        """list[str]|None: list of key for bearer authentication"""
        return self.__bearerauth

    @bearerauth.setter
    def bearerauth(self, bearerauth:  list[str]|str):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        if type(bearerauth) == str:
            bearerauth = [bearerauth]
        self.__bearerauth = bearerauth


    @property
    def headermodification(self):
        """list[str]|None: list of header modifications. Check https://pinggy.io/docs/advanced/live_header/ for more details"""
        return self.__headermodification

    @headermodification.setter
    def headermodification(self, headermodification: list[str]):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        self.__headermodification = headermodification

    def remove_header(self, header_name):
        self.removeHeader(header_name)
    def removeHeader(self, header_name):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        if self.__headermodification is None:
            self.__headermodification = []
        self.__headermodification.append(f"r:{header_name}")

    def add_header(self, header_name, new_value):
        self.addHeader(header_name, new_value)
    def addHeader(self, header_name, new_value):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        if self.__headermodification is None:
            self.__headermodification = []
        self.__headermodification.append(f"a:{header_name}:{new_value}")

    def update_header(self, header_name, new_value):
        self.updateHeader(header_name, new_value)
    def updateHeader(self, header_name, new_value):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        if self.__headermodification is None:
            self.__headermodification = []
        self.__headermodification.append(f"a:{header_name}:{new_value}")


    @property
    def localservertls(self):
        """str|None: return current localservertls config,"""
        return self.__localservertls

    @localservertls.setter
    def localservertls(self, val: str):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        if val is None or val == "":
            self.__localservertls = None
            return
        if type(val) != str:
            raise Exception("Only string type allowed")
        self.__localservertls = val


    @property
    def xff(self):
        """bool: whethere xff is set or not."""
        return self.__xff

    @xff.setter
    def xff(self, xff: bool):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        self.__xff = xff


    @property
    def httpsonly(self):
        """bool: whether https only is set or not"""
        return self.__httpsonly

    @httpsonly.setter
    def httpsonly(self, httpsonly: bool):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        self.__httpsonly = httpsonly


    @property
    def fullrequesturl(self):
        """bool: request full url. if this flag is set, full original url would be pass through `X-Pinggy-Url` header in the request"""
        return self.__fullrequesturl

    @fullrequesturl.setter
    def fullrequesturl(self, fullrequesturl: bool):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        self.__fullrequesturl = fullrequesturl


    @property
    def allowpreflight(self):
        """bool: allow preflight requests to pass through without processing"""
        return self.__allowpreflight

    @allowpreflight.setter
    def allowpreflight(self, allowpreflight: bool):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        self.__allowpreflight = allowpreflight


    @property
    def reverseproxy(self):
        """"bool: enables reverseproxy mode. default is true."""
        return self.__reverseproxy

    @reverseproxy.setter
    def reverseproxy(self, reverseproxy: bool):
        if not self.__editableConfig:
            raise Exception("Tunnel is already connected, no modification allowed")
        self.__reverseproxy = reverseproxy

    def getProcessedArguments(self):
        if not self.__editableConfig:
            return core.pinggy_config_get_argument(self.__configRef)
        return self.__prepare_argument()

    def __prepare_argument(self):
        val = []

        if self.__ipwhitelist is not None and len(self.__ipwhitelist) > 0:
            whitelist = "w:"
            whitelist += ",".join(self.__ipwhitelist)
            val.append(whitelist)

        if self.__basicauth is not None and len(self.__basicauth) > 0:
            for u,p in self.__basicauth.items():
                val.append(f"b:{u}:{p}")

        if self.__bearerauth is not None and len(self.__bearerauth) > 0:
            for auth in self.__bearerauth:
                val.append(f"k:{auth}")

        if self.__headermodification is not None and len(self.__headermodification) > 0:
            for hm in self.__headermodification:
                val.append(f"{hm}")

        if self.__xff:
            val.append("x:xff")

        if self.__httpsonly:
            val.append("x:https")

        if self.__fullrequesturl:       # bool = False
            val.append("x:fullurl")

        if self.__allowpreflight:       # bool = False
            val.append("x:passpreflight")

        if not self.__reverseproxy:         # bool = False
            val.append("x:noreverseproxy")

        if self.__localservertls is not None and self.__localservertls != "":
            val.append("x:localserverlts:"+self.__localservertls)

        argument = shlex.join(val)

        if self.__cmd != "":
            argument = self.__cmd + " " + argument

        argument = argument if isinstance(argument, bytes) else argument.encode("utf-8")

        return argument

    def __prepare_n_setargument(self):
        argument = self.__prepare_argument()
        core.pinggy_config_set_argument(self.__configRef, argument)


def __start_tunnel(tun, webdebuggerport):

    success = tun.connect()
    if not success:
        msg = tun.authentication_messages
        if type(msg) == list:
            msg = "\n".join(msg)
        raise Exception("Connection Failed:\n" + msg)

    success = tun.request_primary_forwarding()
    if not success:
        msg = tun.tunnel_statup_messages
        if type(msg) == list:
            msg = "\n".join(msg)
        raise Exception("Connection Failed:\n" + msg)

    tun.start(True)

    if webdebuggerport > 0:
        tun.start_web_debugging(webdebuggerport)


def start_tunnel(
        forwardto: int|str = 80,
        type: str = "http",
        token: str = "",
        force: bool = False,
        ipwhitelist: list[str]|str|None = None,
        basicauth:  dict[str,str]|None = None,
        bearerauth:  list[str]|str|None = None,
        headermodification: list[str]|None = None,
        webdebuggerport: int = 0,
        localservertls: str|bool = False,
        xff: bool = False,
        httpsonly: bool = False,
        fullrequesturl: bool = False,
        allowpreflight: bool = False,
        reverseproxy: bool = True,
        serveraddress: str = "a.pinggy.io:443",
        udpforwardto: int | str = 0,
        autoreconnect: bool = False,
        eventclass = BaseTunnelHandler
):
    """
    Start a tunnel inside a new thread and get reference to the tunnel.

    Args:
        forwardto: address of local server. Only port can be provided incase of local server. Example: 80, "localhost:80".

        type: Type of the tunnel. values can be one of `http`, `tcp`, `tls`, `tlstcp`. `http` is the default value.

        token: User token. Get it from https://dashboard.pinggy.io

        force: enable of disable force flag. Enabling it would cause to stop any existing tunnel with same token.

        ipwhitelist: list of ipaddresses that are allowed to connect to the tunnel. Example: ["2301::c4f:45c2:57e6:e637:7f1a/128","23.15.30.223/32"].
                    Be carefull about the ipv6 syntax

        basicauth: dictionary of username:password. This dictionary be used for basic authentication. Example: {"hello": "world"}

        bearerauth: list of keys that would be used for bearer key authentication. Both basicauth and bearerauth can be used together.
                    Example: ["1234"]

        headermodification: list of header modification that would be added. More detail at https://pinggy.io/docs/advanced/live_header/
                    Example: ["r:Accept", "u:UserAgent:PinggyTestServer 1.2.3"]

        webdebuggerport: Webdebugging port. Webdebugging would start only if valid port is provided. Example: 4300

        localservertls: This flag enables TLS for the local server. If it is a string, it would be used as the server name for SNI. If it is True, it would be set to "localhost" by default.
                    If it is False, it would be set to None. Default: False

        xff: With this flag, pinggy adds `X-Forwarded-For` with the request header.

        httpsonly: This flag make sure that the visitor uses only the https. Any request to http would the redirected to https url.

        fullrequesturl: Pinggy server adds the original url that is requested in a header `X-Pinggy-Url ` with the request.

        allowpreflight: With this flag, pinggy detects and allow preflight request without processing so that the server can handle it.

        reverseproxy: Pinggy by default runs in reverse proxy mode. However, it can be turned off by setting this flag `False`

        serveraddress: User can set the server address to which pinggy would connect. Default: `a.pinggy.io:443`.

        udpforwardto: same as tcp forward to, however, it allows users to forward udp along with tcp. If user wants to forward only udp, use `start_udptunnel`.

        autoreconnect: automatically reconnects when tunnel failes. It happens silently. So, to detect reconnection, one need to override the event handler.

        eventclass: event handler class. Object would be created for the tunnel.
    """

    if eventclass is None:
        eventclass = BaseTunnelHandler
    tun = Tunnel(server_address=serveraddress, eventClass=eventclass)

    tun.tcp_forward_to          = forwardto
    tun.type                    = type
    tun.token                   = token
    tun.force                   = force
    try:
        tun.auto_reconnect      = autoreconnect
        #auto reconnection may not present in the library
    except:
        pass

    if bool(udpforwardto):
        tun.udp_forward_to = udpforwardto

    if ipwhitelist is not None:
        tun.ipwhitelist = ipwhitelist
    if basicauth is not None:
        tun.basicauth = basicauth
    if bearerauth is not None:
        tun.bearerauth = bearerauth
    if headermodification is not None:
        tun.headermodification = headermodification

    if isinstance(localservertls, str):
        tun.localservertls = localservertls
    elif localservertls:
        tun.localservertls = "localhost"

    tun.xff                     = xff
    tun.httpsonly               = httpsonly
    tun.fullrequesturl          = fullrequesturl
    tun.allowpreflight          = allowpreflight
    tun.reverseproxy            = reverseproxy

    __start_tunnel(tun, webdebuggerport)

    return tun

def start_udptunnel(
        forwardto: int|str,
        token: str = "",
        force: bool = False,
        ipwhitelist: list[str]|str|None = None,
        webdebuggerport: int = 4300,
        serveraddress: str = "a.pinggy.io:443",
        autoreconnect: bool = False,
        eventclass = BaseTunnelHandler
):
    """
    Start an udp tunnel inside a new thread and get reference to the tunnel.

    Args:
        forwardto: address of local server. Only port can be provided incase of local server. Example: 53, "localhost:53".

        token: User token. Get it from https://dashboard.pinggy.io

        force: enable of disable force flag. Enabling it would cause to stop any existing tunnel with same token.

        ipwhitelist: list of ipaddresses that are allowed to connect to the tunnel. Example: ["2301::c4f:45c2:57e6:e637:7f1a/128","23.15.30.223/32"].

        webdebuggerport: Webdebugging port. Webdebugging would start only if valid port is provided. Example: 4300

        serveraddress: User can set the server address to which pinggy would connect. Default: `a.pinggy.io:443`.

        autoreconnect: automatically reconnects when tunnel failes. It happens silently. So, to detect reconnection, one need to override the event handler.

        eventclass: event handler class. Object would be created for the tunnel.
    """

    if eventclass is None:
        eventclass = BaseTunnelHandler

    tun = Tunnel(server_address=serveraddress)

    tun.udp_forward_to          = forwardto
    tun.type                    = "udp"
    tun.token                   = token
    tun.force                   = force
    try:
        tun.auto_reconnect      = autoreconnect
        #auto reconnection may not present in the library
    except:
        pass

    if ipwhitelist is not None:
        tun.ipwhitelist = ipwhitelist

    __start_tunnel(tun, webdebuggerport)

    return tun


