import subprocess
import struct
from time import sleep
import os

'''
Intentionally broken code.
This illustrates that subprocess is not what we are looking for in terms of
implementing the functionality we desire.
'''

# Get the absolute filepath of the test subprocess (appending properly depending if windows or linux)
dir_path = os.path.dirname(os.path.realpath(__file__))
if os.name=="posix":
    inputCmd = dir_path + "/subproc.py"
elif os.name=="nt":
    inputCmd = dir_path + "\\subproc.py"
else:
    exit(1)

print("Starting subprocess")

proc = subprocess.Popen(inputCmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)
print("Subprocess started with PID ", proc.pid)

# grep_stdout = proc.communicate(input=b'123')[0]
# print("Got", grep_stdout.decode(), "from subprocess")

while True:
    in_data = int( input("Send input to subproc:") )
    print("Sent", in_data, "to subprocess.")

    out = proc.communicate(in_data.to_bytes(4, "big"))[0]
    # multiple writes to communicate fail
        
    # proc.stdin.write(in_data.to_bytes(4, "big"))
    # out = proc.stdout.readline()
    print(out.decode())

proc.terminate()