import pexpect
from pexpect import popen_spawn
# import subprocess
import platform
import sys
import socket   
import struct      
import numpy as np
import re, uuid
import binascii


# Create a socket object (on local host)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
port = 8080
sock.connect(('52.77.216.211', port))

on_windows = "windows" in platform.uname()[0].lower()
on_wsl = "microsoft" in platform.uname()[3].lower()

if on_windows :
    print(">> On Windows.")
    inputCmd = "C:\\intelFPGA_lite\\20.1\\quartus\\bin64\\nios2-terminal.exe --quiet --no-quit-on-ctrl-d"
elif on_wsl:
    print(">> On WSL.")
    inputCmd = '/bin/bash -c "/mnt/c/intelFPGA_lite/20.1/quartus/bin64/nios2-terminal.exe --quiet --no-quit-on-ctrl-d"'
else:
    print(">> On Linux.")
    inputCmd = '/bin/bash -c "/home/e2/intelFPGA_lite/20.1/quartus/bin/nios2-terminal --quiet --no-quit-on-ctrl-d"'

print(">> Starting subprocess. Enter 'q' on prompt to exit.")
c = pexpect.popen_spawn.PopenSpawn(inputCmd, encoding='utf-8') # with Windows compatibility
print(">> Ready! Waiting for Server to send something.")

# Generate error packet
err_packet = [99,99,99]
err_bytes = bytearray()
for err in err_packet:
    err = bytearray( err.to_bytes(4, "big") )   # Network byte-order is big-endian, so we specify this.
    err_bytes += err

# define return vals
i=0
j=0
k=0

#Comment out this code if using native linux
mac_addr_input = input ("Enter mac address :")
mac_addr = binascii.unhexlify(mac_addr_input.replace(':', ''))
print("mac address in bytes :", mac_addr)
#
#00:0c:29:8e:fd:82



pause = 0

while True:
    pkt_header = np.int8(1)
    mac_int = uuid.getnode()

    #Comment out this code if using WSL 1
    #mac_addr = bytearray(mac_int.to_bytes(6, "big"))
    #print(">> Mac addr: ", mac_addr)
    #
    data_send = [i, j, k]
    data_bytes = bytearray()
    data_bytes += pkt_header.tobytes()
    # data_bytes += mac_addr
    data_bytes += mac_addr
    for data_packet in data_send:
        data_packet = bytearray( data_packet.to_bytes(1, "big") )   # Network byte-order is big-endian, so we specify this.
        data_bytes += data_packet
    
    sock.send(data_bytes)
    print(">> Sent", data_bytes, "to server")
    
    ##
    dataFromServer = sock.recv(1024)    # Receive data from server
    send_data = dataFromServer.decode()
    print(">> Got", send_data, "from server.")
    
    if send_data == 'q':
        break
    elif len(send_data) != 1:
        print("Send only one character.")
        sock.send(err_bytes)   # send [99,99,99] (error code) to server
        continue

    #send to fpga only if not at task station
    if pause==0:
        x = c.send(send_data)
        print(">> Sent", send_data, "with", x, "bytes to fgpa.")

    

    index = c.expect(['{', pexpect.TIMEOUT], timeout=20)
    if index==0:
        #print("Timeout condition reached. Breaking")
    
        c.expect(" ")
        i = int(c.before, base=16)
        c.expect(" ")
        j = int(c.before, base=16)
        c.expect("}")   # Now with line endings
        k = int(c.before, base=16)
        print(">> Obtained:", "i:", i, "j:", j, "k:", k)
        pause = 0
    else:
        i=0
        j=0
        k=0
        print(">> Sending data so server won't fomo")
        pause = 1

    # pkt_header = np.int8(1)
    # mac_int = uuid.getnode()
    # mac_addr = bytearray(mac_int.to_bytes(6, "big"))
    # print(">> Mac addr: ", mac_addr)

    # data_send = [i, j, k]
    # data_bytes = bytearray()
    # data_bytes += pkt_header.tobytes()
    # # data_bytes += mac_addr
    # data_bytes += mac_addr
    # for data_packet in data_send:
    #     data_packet = bytearray( data_packet.to_bytes(1, "big") )   # Network byte-order is big-endian, so we specify this.
    #     data_bytes += data_packet
    

    # sock.send(data_bytes)
    # print(">> Sent", data_bytes, "to server")
     
  
    # printing the value of unique MAC 
    # address using uuid and getnode() function  
    # print (hex(uuid.getnode())) 
    # # joins elements of getnode() after each 2 digits. 
    # # using regex expression 
    print ("The MAC address in formatted and less complex way is : ", end="") 
    print (':'.join(re.findall('..', '%012x' % uuid.getnode()))) 

    # dataFromServer = sock.recv(1024)    # Receive data from server
    # send_data = dataFromServer.decode()
    # print(">> Got", send_data, "from server.")


sock.close()
c.kill(2)
print("This may not kill your nios2 processes. Check Task Manager to make sure they are dead.")