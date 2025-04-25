
### Blocking issue with python
* In windows, poll was not blocking for event at all. It turns out that I was checking notification event wrongly. Fixed it.
* In windows, poll was not interupted by ctrl+c. This is problem. So, I added a 1 sec timeout to the poll.