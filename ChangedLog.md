
### Blocking issue with python
* In windows, poll was not blocking for event at all. It turns out that I was checking notification event wrongly. Fixed it.
* In windows, poll was not interupted by ctrl+c. This is problem. So, I added a 1 sec timeout to the poll.
* Fixed udp connection. It was having problem as I mistakenly collecting IPv6 address for the a domain. Fixed it.
* Added default port and default protocol in UrlPtr. Now, one can set server address as `a.pinggy.io` instead of `a.pinggy.io:443`.
