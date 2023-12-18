from socket import *

# Change to the IP address of the board
address = ("192.168.36.121", 1234)

s = socket(AF_INET, SOCK_DGRAM)

s.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)

message = b"This is broadcase message !"

s.sendto(message, address)

print(" send ok !")

s.close()
