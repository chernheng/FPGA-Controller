import subprocess
from threading import Thread
import time
import collections
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import struct

import socket
import sys

# Basically sends the recieved UART data to another UDP client on localhost.

class jtag_uart:
    def __init__(self, plotLength=100, ip='localhost', port=10000):
        inputCmd = "C:\\intelFPGA_lite\\19.1\\quartus\\bin64\\nios2-terminal.exe --quiet --no-quit-on-ctrl-d"
        self.plotMaxLength = plotLength
        self.previousTimer = 0
        self.data_x = collections.deque([0] * plotLength, maxlen=plotLength)
        self.data_y = collections.deque([0] * plotLength, maxlen=plotLength)
        self.data_z = collections.deque([0] * plotLength, maxlen=plotLength)
        self.thread = None
        
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.server_address = (ip, port)

        self.proc = subprocess.Popen(inputCmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print("Communication started with PID", self.proc.pid)

    def startCommunication(self):
        if self.thread == None:
            self.thread = Thread(target=self.backgroundThread)
            self.thread.start()
            print("Input thread started")

    def backgroundThread(self):
        while( self.proc.poll() == None ):
            # retrieve data from serial
            out = self.proc.stdout.readline()
            if len(out) == 14:
                # out += self.proc.stdout.readline()
                # print("out: \"%s\" length: %d" %(out, len(out)), flush=True)
                self.data_x.append( struct.unpack('<i', out[:4] )[0] )
                self.data_y.append( struct.unpack('<i', out[4:8] )[0] )
                self.data_z.append( struct.unpack('<i', out[8:12] )[0] )
                self.sock.sendto(out, self.server_address)

    def close(self):
        print("Ending communication with Nios-II")
        self.proc.terminate()
        self.sock.close()

    def getSerialData(self, frame, lines, lineValueText, lineLabel, timeText):
        currentTimer = time.perf_counter()      # returns the float value of time in seconds. perf_counter -> performance counter
        self.plotTimer = int((currentTimer - self.previousTimer) * 1000)     # the first reading will be erroneous
        self.previousTimer = currentTimer
        timeText.set_text('Plot Interval = ' + str(self.plotTimer) + 'ms')

        lines[0].set_data(range(self.plotMaxLength), self.data_x)
        lines[1].set_data(range(self.plotMaxLength), self.data_y)
        lines[2].set_data(range(self.plotMaxLength), self.data_z)

        lineValueText[0].set_text('[' + lineLabel[0] + '] = ' + str(self.data_x[-1]))
        lineValueText[1].set_text('[' + lineLabel[1] + '] = ' + str(self.data_y[-1]))
        lineValueText[2].set_text('[' + lineLabel[2] + '] = ' + str(self.data_z[-1]))
    

def main():
    maxPlotLength = 1000
    s = jtag_uart(maxPlotLength)
    s.startCommunication()

    # plotting starts below
    pltInterval = 50    # Period at which the plot animation updates [ms]
    xmin = 0
    xmax = maxPlotLength
    ymin = -500
    ymax = 500
    fig = plt.figure()
    ax = plt.axes(xlim=(xmin, xmax), ylim=(float(ymin - (ymax - ymin) / 10), float(ymax + (ymax - ymin) / 10)))
    ax.set_title('X-axis accelerometer Reading')
    ax.set_xlabel("time")
    ax.set_ylabel("Acceleration value (G)")
    lineLabel = ("X-axis", "Y-axis", "Z-axis")
    timeText = ax.text(0.30, 0.95, '', transform=ax.transAxes)
    lines = (ax.plot([], [], label=lineLabel[0])[0], ax.plot([], [], label=lineLabel[1])[0], ax.plot([], [], label=lineLabel[2])[0])
    lineValueText = (ax.text(0.30, 0.90, '', transform=ax.transAxes), ax.text(0.3, 0.85, '', transform=ax.transAxes), ax.text(0.3, 0.8, '', transform=ax.transAxes))

    # No clue what this animation does but it's some deep matplotlib conspiracy probably
    anim = animation.FuncAnimation(fig, s.getSerialData, fargs=(lines, lineValueText, lineLabel, timeText), interval=pltInterval)    # fargs has to be a tuple

    plt.legend(loc="upper left")
    plt.show() # blocks until the window closes

    s.close()


if __name__ == '__main__':
    main()
