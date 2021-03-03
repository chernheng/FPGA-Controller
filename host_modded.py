import subprocess
import struct
from time import sleep


def main():
    inputCmd = "C:\\intelFPGA_lite\\20.1\\quartus\\bin64\\nios2-terminal.exe --quiet --no-quit-on-ctrl-d"

    log = open("log.txt", 'a')
    log_errors = 0

    print("Starting subprocess")

    # subprocess allows python to run a command.
    # Popen opens something that lets you communicate.
    proc = subprocess.Popen(inputCmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    print("Subprocess started with PID ", proc.pid)
    
    # This is how we read from stdout.
    while log_errors < 10:
        out = proc.stdout.readline()
        # raw = proc.stdout.readline()
        # out = raw[:-2]
        # out += bytes(raw[-1]) # Get rid of the trailing \r
        # if len(out) < 12:
        #     out += proc.stdout.readline()
            # raw = proc.stdout.readline()
            # out = raw[:-1] 
            # out += bytes(raw[-1]) # Get rid of the trailing \r
            # log.write(str(out))
            # log.write('\n')
            # log_errors += 1
            # If we get a newline character, continue to read the buffer

        print(out)
        if len(out) >= 14:
            value_x = struct.unpack('<i', out[:4] )[0]
            value_y = struct.unpack('<i', out[4:8] )[0]
            value_z = struct.unpack('<i', out[8:12] )[0]
            print("values:", value_x, value_y, value_z, flush=True)
    
    proc.terminate()
    return

    
if __name__ == '__main__':
    main()
