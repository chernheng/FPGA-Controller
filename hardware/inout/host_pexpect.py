import pexpect
from pexpect import popen_spawn
# import subprocess
import os
import sys

# Working example of interacting interactively with a subprocess.
if os.name == "posix":
    print(">> On Linux.")
    inputCmd = '/bin/bash -c "/mnt/c/intelFPGA_lite/20.1/quartus/bin64/nios2-terminal.exe --quiet --no-quit-on-ctrl-d"'
elif os.name == "nt":
    print(">> On Windows.")
    inputCmd = "C:\\intelFPGA_lite\\20.1\\quartus\\bin64\\nios2-terminal.exe --quiet --no-quit-on-ctrl-d"
else:
    print("Please use a linux or windows system.")
    exit(1)

print(">> Starting subprocess. Enter 'q' on prompt to exit.")
c = pexpect.popen_spawn.PopenSpawn(inputCmd, encoding='utf-8') # with Windows compatibility
print(">> Ready!")

while True:
    send_data = input(">> Send to subproc:")
    if send_data == 'q':
        break
    elif len(send_data) != 1:
        print("Send only one character.")
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

c.kill(2)
print("Sent SIGINT to child. Exiting")