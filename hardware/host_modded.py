import subprocess
import struct
from time import sleep

def main():
    # the second option is important as we sometimes might send the ASCII code for Ctrl-D (end of stream)
    # through stdout, which would otherwise terminate the subprocess.
    inputCmd = "C:\\intelFPGA_lite\\20.1\\quartus\\bin64\\nios2-terminal.exe --quiet --no-quit-on-ctrl-d"

    print("Starting subprocess")

    # subprocess allows python to run a command.
    # Popen opens something that lets you communicate.
    proc = subprocess.Popen(inputCmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)
    print("Subprocess started with PID ", proc.pid)
    
    # This is how we read from stdout.
    while True:
        in_fpga = int(input("Send to FPGA a number\n")).to_bytes(4, byteorder="little")
        # convert into a bytes object

        proc.stdin.write(in_fpga)

        out = proc.stdout.readline()
        print(out)
        # Barring any errors, we will always recieve 14 bytes (12 as 3 int32_t's (XYZ), and 2 from \r\n line endings.)
        # Sometimes we get less (not sure why) so just reject those as errors.
        if len(out) == 14:
            value_x = struct.unpack('<i', out[:4] )[0]
            value_y = struct.unpack('<i', out[4:8] )[0]
            task_enum = struct.unpack('<i', out[8:12] )[0]
            print("values:", value_x, value_y, task_enum, flush=True)

    proc.terminate()
    return

if __name__ == '__main__':
    main()