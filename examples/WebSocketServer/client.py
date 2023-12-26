#!/usr/bin/env python
from websocket import create_connection

# Replace the IP address with the IP address output by the board’s serial port
IP_ADDRESS = '192.168.36.121'
url = "ws://{0}/ws".format(IP_ADDRESS)
while True:
    ws = create_connection(url)
    data = input("Please enter message:")
    ws.send(data)
    print("Message received:", ws.recv())
    if data == "q":
        ws.close()
        break
