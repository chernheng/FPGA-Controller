import pexpect
import os

# Working example of sending a 

# Get the absolute filepath of the test subprocess (appending properly depending if windows or linux)
dir_path = os.path.dirname(os.path.realpath(__file__))
if os.name=="posix":
    inputCmd = dir_path + "/subproc.py"
elif os.name=="nt":
    inputCmd = dir_path + "\\subproc.py"
else:
    exit(1)

print("Starting subprocess")
c = pexpect.spawnu(inputCmd)

while True:
    send_data = input("Send to subproc:")
    c.expect("Give me Data:")   #expect can be a regex (good for us to filter data!!)
    c.sendline(send_data)
    c.expect("Data:")
    print("Before From host:", c.before) # The before property will contain all text up to the expected string pattern.
    print("After from host:" , c.after) # The after string will contain the text that was matched by the expected pattern.