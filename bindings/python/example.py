import pinggy
import time

class handle(pinggy.BaseTunnelHandler):
    def usage_update(self, usages):
        print(usages)
    def will_reconnect(self, messages):
        print(messages)
    def reconnecting(self, retry_cnt):
        print("Reconnecting: ", retry_cnt)
    def reconnection_completed(self):
        print("Reconnection completed: ", self.get_tunnel().urls)
    def reconnection_failed(self, retry_cnt):
        print("Reconnection failed")

pinggy.enable_log()

tun = pinggy.Tunnel(server_address="core2.t.pinggy.io", eventClass=handle)
tun.auto_reconnect = True
tun.start_usage_update()
tun.tcp_forward_to = "127.0.0.1:4000"
# tun.udp_forward_to = "127.0.0.1:4000"
tun.sni_server_name = "t.pinggy.io"
tun.type = "http"

tun.basicauth = {'user1': 'pass1', 'user2': 'pass two'}
print(tun.getProcessedArguments())
tun.basicauth = None
print(tun.getProcessedArguments())
exit(0)

tun.connect()

print(tun.tcp_forward_to)

tun.start(True)
print(tun.greeting_msgs)
print(tun.urls)
time.sleep(2)
print(tun.greeting_msgs)
time.sleep(5)
print(tun.current_usages)
time.sleep(5)
tun.stop()