import socket   
import struct  
import numpy as np    
  
# Create a socket object (on local host)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
port = 8080
sock.connect(('52.77.216.211', port))

# Manipulate the data to be sent back to server here.
# pkt_header = bytes(1, 'utf-8')
pkt_header = np.int8(1)
data_send = [3,2,1]
data_bytes = bytearray()
data_bytes += pkt_header.tobytes()
for data_packet in data_send:
    data_packet = bytearray( data_packet.to_bytes(4, "big") )   # Network byte-order is big-endian, so we specify this.
    data_bytes += data_packet

#data_bytes_send = bytearray()
#data_bytes_send = bytes(pkt_header) + data_bytes
#data_bytes += pkt_header.tobytes()
print("Sent", data_bytes, "to server")
sock.send(data_bytes)

dataFromServer = sock.recv(1024)    # Receive data from server

# Print to the console
print(dataFromServer.decode())

# close the connection  
sock.close()