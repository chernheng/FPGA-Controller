import pexpect
from pexpect import popen_spawn
# import subprocess
import platform
import sys
import socket   
import struct      

# Create a socket object (on local host)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
port = 8080
sock.connect(('127.0.0.1', port))

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

while True:
    dataFromServer = sock.recv(1024)    # Receive data from server
    send_data = dataFromServer.decode()
    print(">> Got", send_data, "from server.")
    
    if send_data == 'q':
        break
    elif len(send_data) != 1:
        print("Send only one character.")
        sock.send(err_bytes)   # send [99,99,99] (error code) to server
        continue

    x = c.send(send_data)
    print(">> Sent", send_data, "with", x, "bytes to fgpa.")
    index = c.expect(['{', pexpect.TIMEOUT], timeout=5)
    if index!=0:
        print("Timeout condition reached. Breaking")
        break
    c.expect(" ")
    i = int(c.before, base=16)
    c.expect(" ")
    j = int(c.before, base=16)
    c.expect("}")   # Now with line endings
    k = int(c.before, base=16)
    print(">> Obtained:", "i:", i, "j:", j, "k:", k)

    data_send = [i, j, k]
    data_bytes = bytearray()
    for data_packet in data_send:
        data_packet = bytearray( data_packet.to_bytes(4, "big") )   # Network byte-order is big-endian, so we specify this.
        data_bytes += data_packet

    sock.send(data_bytes)
    print(">> Sent", data_bytes, "to server")


sock.close()
c.kill(2)
print("This may not kill your nios2 processes. Check Task Manager to make sure they are dead.")