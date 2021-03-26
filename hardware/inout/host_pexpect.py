import pexpect
from pexpect import popen_spawn
# import subprocess
import platform
import sys
# print(platform.uname())
# Working example of interacting interactively with a subprocess.
on_windows = "windows" in platform.uname()[0].lower()
on_wsl = ("microsoft" in platform.uname()[2].lower()) or ("microsoft" in platform.uname()[3].lower())

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
print(">> Ready!")

flag = False

while True:
    if flag:
        send_data = input(">> Send to subproc:")
    else:
        flag = True
        send_data = '1'
    if send_data == 'q':
        break
    elif len(send_data) != 1:
        print("Send only one character.")
        continue
    x = c.send(send_data)
    print(">> Sent", send_data, "with", x, "bytes to fgpa.")
    index = c.expect(['{', pexpect.TIMEOUT], timeout=15)
    if index!=0:
        print("Timeout condition reached. Breaking")
        break
    c.expect(" ")
    i = int(c.before, base=16)-4
    c.expect(" ")
    j = int(c.before, base=16)-4
    c.expect("}")   # Now with line endings
    k = int(c.before, base=16)
    print(">> Obtained:", "i:", i, "j:", j, "k:", k)

c.kill(2)
print("Sent SIGINT to child. Exiting")