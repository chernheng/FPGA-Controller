
# Import socket module  
import socket    
import numpy as np    
import struct      
  
# Create a socket object  
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)        
  
# Define the port on which you want to connect  
port = 8080              
  
# connect to the server on local computer  
sock.connect(('13.212.10.207', port))  

# Send data to server
# data = "Hello Server!"

#data = np.array([1, 2, 3], dtype='int32')

# my_bytes = bytearray()
# my_bytes.append(value_x)
# my_bytes.append(value_y)
# my_bytes.append(enum)

# data = my_bytes
#sock.send(data) #encode turns strings into bytes?
# sock.send(b'1234')
data = bytearray(struct.unpack("4b", struct.pack("I", 100)))
print("Sent", data, "to server")

sock.send(data)
# Receive data from server
dataFromServer = sock.recv(1024)

# Print to the console
print(dataFromServer.decode())

# receive data from the server  
#print (s.recv(1024) ) 
# close the connection  
sock.close()   