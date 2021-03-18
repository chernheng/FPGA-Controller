import pexpect
import subprocess
import os
import sys

# Working example of interacting interactively with a subprocess.
# DOES NOT WORK ON WINDOWS.
if os.name != "posix":
    print("Please use a linux system.")
    exit(1)

# Run nios2-command shell first.
# subprocess.run("/mnt/c/intelFPGA_lite/20.1/nios2eds/nios2_command_shell.sh")
# Actual 
inputCmd = "/mnt/c/intelFPGA_lite/20.1/quartus/bin64/nios2-terminal.exe --quiet --no-quit-on-ctrl-d"

print(">> Starting subprocess (Ctrl-D to exit)")
c = pexpect.spawnu(inputCmd)
print(str(c))   # Debugging information
# c.logfile = sys.stdout.buffer

while True:
    c.expect('.')
    send_data = input(">> Send to subproc:")
    c.send(send_data)
    print(">> Sent", send_data, "to fgpa.")
    c.expect(".")   #
    print(">> Before:", c.before)
    print(">> After:", c.after)
    # c.expect("Give me Data:")   #expect can be a regex (good for us to filter data!!)
    # c.expect("Data:")
    # print("Before From host:", c.before) # The before property will contain all text up to the expected string pattern.
    # print("After from host:" , c.after) # The after string will contain the text that was matched by the expected pattern.