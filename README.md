# libpinggy - core sdk for Pinggy
A C++ library intended for creating bindings to all major languages.

## Building instructions for windows

Install the dependencies.

### Dependencies

1. `git` - Download and install git
2. Visual Studio - use the script (run as administrator) - `crossbuilding/windows/utilities/install_visual_studio.bat`
3. Pearl - use the script (run as administrator) - `crossbuilding/windows/utilities/install_pearl.bat`
4. NASM - use the script (run as administrator) - `crossbuilding/windows/utilities/install_nasm.bat`

### OpenSSL

Building libpinggy requires OpenSSL to be built first.

**Todo:** Add a script to download prebuild opesnssl files.

Place the openssl built files in `C:\OpenSSL\`

**Building from sorce:**

Run the script `crossbuilding/windows/opensslbuild.bat`

## Building  libpinggy from source


`.\crossbuilding\windows\buildForWindows.bat C:\OpenSSL\ releases build`

`.\crossbuilding\windows\buildForSingleArch.bat x86_64 MT`


# Manual building
Once OpenSSL is installed, manual build can be done using following commands
```
$ mkdir build
$ cmake ..
$ cmake --build . -j --config Release //building the code
$ cmake --build . releaselib //releasing libpinggy and
$ cmake --build . releasessl //releasing ssl
$ cmake --build . distribute //creating libpinggy.tgz/zip
```

# Tunnel life cycle

               +-----------+
               | initiate  |   // 1. initiate
               +-----------+
                    |
                    v
                +--------+
                | config |   // 2. config
                +--------+
                    |  |
                    |  `-------------.
                    |                |
                    v                |
         no    +--------+            |
    ,----------|connect |   // 3. connect
    |          +--------+            |
    |            yes|                |
    |               |----------------|
    |               v                |
    |  no  +-------------------+     |
    |------|request_primary_fwd|   // 4. request_primary_forwarding
    |      +-------------------+     |
    |            yes|                |
    |               |----------------|
    |               v                |
    |      no   +------+             |
    |-----------|resume|<-.  // 5. resume
    |           +------+  |          |
    |            yes|     |          v
    |               |-----'     +--------+
    |               |---------->| start  |   // 6. start
    |               |           +--------+
    |               `------.--------'
    |                      |
    |                      v
    |                  +------+
    `----------------->| end  |   // 7. end
                       +------+

## Edges:
   1. initiate --> 2. config
   2. config --> 3. connect
   2. config --> 6. start
   3. connect --> 6. start
   3. connect --> 4. request_primary_forwarding
   4. request_primary_forwarding --> 6. start
   4. request_primary_forwarding --> 5. resume
   5. resume --> 5. resume
   5. resume --> 6. start
   5. resume --> 7. end
   6. start --> 7. end

## Callback and Allowed Functions

1. Set up the tunnel and config. The tunnel does not remember the config.
2. Configure the tunnel using the config pointer. All `pinggy_config*` functions can be called now.
3. On a successful call to (`pinggy_tunnel_connect_blocking`):
    * The `on_authenticated` callback is called.
    * No `pinggy_config_set*` functions are allowed.
4. On a successful call to `request_primary_forwarding`:
    * The `on_primary_forwarding_succeeded` callback is called.
    * URL, usages, and greeting messages are available now.
    * You can start the web debugger now.
    * You can also add more forwarding.
5. Call `resume` repeatedly or call `start` to start the tunnel.
    * While the tunnel is running (either through `resume` or `start`), it can make the following callbacks in any order:
        1. `on_additional_forwarding_succeeded`
            * If additional forwarding is requested and accepted by the server.
            * At these point, tunnel is ready to use. However, There are few task running in back ground.
        2. `on_additional_forwarding_failed`
        3. `on_disconnected`
            * When the tunnel is formally disconnected by the server, or a connection reset happens and reconnection is not enabled.
        4. `on_will_reconnect`
            * When disconnection happens due to a network issue and the SDK will enter a reconnection loop. This indicates that the old tunnel has disconnected. Will be called only once per connection reset.
        5. `on_reconnecting`
            * The SDK is trying to reconnect. This will be called on every reconnection attempt.
        6. `on_reconnection_completed`
            * Once reconnection is completed. URL and greeting messages are available now.
        7. `on_reconnection_failed`
            * If reconnection fails after multiple attempts.
        8. `on_usage_update`
            * If usage updates are enabled. Every time some change happens at the server, this will be called.
        9. `on_tunnel_error`
