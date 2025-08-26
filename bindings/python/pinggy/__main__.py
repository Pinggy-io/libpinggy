from .pylib import *
import argparse
from pinggy import Tunnel


# main function that can read command line arguments and use the same to call start_tunnel followed by printing the URLs
# The options are:
# The command format is:
# pinggy [options] [[token+][type+][force+]@server_address [arguments]]
# -R, --tcp-forward-to: The TCP address to forward to (default: "localhost:80"). It supports formats like [[[bindname:]bindport:]]localaddress:]localport
# -U, --udp-forward-to: The UDP address to forward to (default: "localhost:53"). It supports formats like [[[bindname:]bindport:]]localaddress:]localport
# -l, --token: The token to use (default: None)
# -p, --port: The port to connect to (default: 443)
# Arguments are:
# a:HeaderName:HeaderValue  Add a header to the request
# r:HeaderName Remove a header from the request
# u:HeaderName:HeaderValue Update a header in the request. It is equivalent to r:HeaderName followed by a:HeaderName:HeaderValue
# b:username:password Set the basic authentication credentials
# k:key Set the key for bearer key authentication
# w:[IP1[,IP2[,IP3..]]] Set the allowed IPs for the tunnel
# x:https Force the tunnel to use HTTPS
# x:xff Force pinggy to use add X-Forwarded-For header
# x:fullurl Pinggy will put the full URL in the  X-Pinggy-Url header
# x:localServerTls[:serverName] Assume the local server is using TLS, and optionally set the server name for SNI
# x:passpreflight Allow preflight requests to pass through without any auhentication
# x:noreverseproxy Do not use reverse proxy for the tunnel

def parse_server_address_and_type(server_address):
    force = False
    token = None
    tunnel_type = None
    udp_type = None
    address = None

    parts = server_address.split("@")
    if len(parts) == 2:
        type_and_token, address = parts
        type_and_token_parts = type_and_token.split("+")
        for p in type_and_token_parts:
            orig = p
            p = p.lower()
            if p == "force":
                force = True
            elif p == "udp":
                udp_type = p
            elif p == "http" or p == "tcp" or p == "tls" or p == "tlstcp":
                tunnel_type = p
            elif p != "qr" and p != "aqr" and p != "auth":
                if token is None:
                    token = orig
    else:
        address = parts[0]

    return address, tunnel_type, udp_type, token, force

def parse_forward_to(arg_forward_to):
    if arg_forward_to is not None:
        parts = arg_forward_to.split(":")
        if len(parts) == 1:
            arg_forward_to = "localhost:" + parts[0]
        elif len(parts) == 2:
            arg_forward_to = ":".join(parts)
        elif len(parts) > 2:
            arg_forward_to = ":".join(parts[-2:])
    return arg_forward_to

def parse_local_forward(forward):
    if forward is None:
        return 0
    parts = forward.split(":")
    if len(parts) >= 3:
        return int(parts[-3])
    return 0

def main():

    parser = argparse.ArgumentParser(description="Start a Pinggy tunnel with specified options.")
    # parser.add_argument("-s", "--server-address", default="a.pinggy.io", help="Server address to connect to")
    parser.add_argument("-R", "--forward-to", default=None, help="TCP address to forward to")
    parser.add_argument("-U", "--udp-forward-to", default=None, help="UDP address to forward to")
    parser.add_argument("-S", "--sni-server-name", default="a.pinggy.io", help=argparse.SUPPRESS)
    parser.add_argument("-l", "--token", default=None, help="Token to use for the tunnel")
    parser.add_argument("-p", "--port", type=int, default=443, help=argparse.SUPPRESS)
    parser.add_argument("-t", "--ignore1", help=argparse.SUPPRESS)
    parser.add_argument("-T", "--ignore2", help=argparse.SUPPRESS)
    parser.add_argument("-n", "--ignore3", help=argparse.SUPPRESS)
    parser.add_argument("-N", "--ignore4", help=argparse.SUPPRESS)
    parser.add_argument("-L", "--web-debug", default=None, help="enable webdebugging")
    parser.add_argument("server_info", nargs=argparse.REMAINDER, help="[username]@servername and any extra arguments")

    args = parser.parse_args()

    server_address = args.server_info[0]
    unknown = args.server_info[1:]

    address, tunnel_type, udp_type, token, force = parse_server_address_and_type(server_address)

    tun = Tunnel(server_address=address)
    # tun.tcp_forward_to = args.tcp_forward_to
    # tun.udp_forward_to = args.udp_forward_to
    tun.sni_server_name = args.sni_server_name
    # tun.token = args.token

    tcp_forward_to = parse_forward_to(args.forward_to)
    udp_forward_to = parse_forward_to(args.udp_forward_to)

    web_debug_port = parse_local_forward(args.web_debug)

    if address is not None:
        tun.server_address = address

    if args.token is not None:
        tun.token = token
    elif token is not None:
        tun.token = token

    if tunnel_type is None and udp_type is None and tcp_forward_to is None and udp_forward_to is None:
        tcp_forward_to = "localhost:80"
        tunnel_type = "http"
    else:
        if tunnel_type is not None or tcp_forward_to is not None:
            if tunnel_type is None:
                tunnel_type = "http"
            if tcp_forward_to is None:
                tcp_forward_to = "localhost:80"
        if udp_type is not None or udp_forward_to is not None:
            if udp_type is None:
                udp_type = "udp"
            if udp_forward_to is None:
                udp_forward_to = "localhost:53"

    if force:
        tun.force = True

    if udp_type is not None:
        tun.udp_type = udp_type
        tun.udp_forward_to = udp_forward_to
    if tunnel_type is not None:
        tun.type = tunnel_type
        tun.tcp_forward_to = tcp_forward_to

    # Process additional arguments
    for arg in unknown:
        if arg.startswith("a:"):
            header = arg[2:].split(":")
            tun.add_header(header[0], header[1] if len(header) > 1 else "")
        elif arg.startswith("r:"):
            tun.remove_header(arg[2:])
        elif arg.startswith("u:"):
            header = arg[2:].split(":")
            tun.update_header(header[0], header[1] if len(header) > 1 else "")
        elif arg.startswith("b:"):
            credentials = arg[2:].split(":")
            if len(credentials) > 1:
                tun.basicauth = {credentials[0]: credentials[1]}
        elif arg.startswith("k:"):
            tun.bearerauth = arg[2:]
        elif arg.startswith("w:"):
            ips = arg[2:].split(",")
            tun.ipwhitelist = ips
        elif arg.startswith("x:"):
            option = arg[2:]
            if option.lower() == "https":
                tun.httpsonly = True
            elif option.lower() == "xff":
                tun.xff = True
            elif option.lower() == "fullurl":
                tun.fullrequesturl = True
            elif option.lower().startswith("localservertls"):
                parts = option.split(":")
                tun.localservertls = "localhost"
                if len(parts) > 1 and parts[1] != "":
                    tun.localservertls = parts[1]
            elif option.lower() == "passpreflight":
                tun.allowpreflight = True
            elif option.lower() == "noreverseproxy":
                tun.reverseproxy = False

    if not tun.connect():
        print("Failed to connect to the server.")
        return
    if not tun.request_primary_forwarding():
        print("Failed to request primary forwarding.")
        return
    if web_debug_port > 0:
        tun.start_web_debugging(web_debug_port)
    print("Tunnel URLs:", tun.urls)
    tun.start()

if __name__ == "__main__":
    main()