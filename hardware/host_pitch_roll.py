import subprocess
from threading import Thread
import time
import collections
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import struct
import numpy as np
import math

# Should plot out XYZ graphs for acceleration in these three axes.

class jtag_uart:
    def __init__(self, plotLength=100):
#       inputCmd = "C:\\intelFPGA_lite\\20.1\\quartus\\bin64\\nios2-terminal.exe --quiet --no-quit-on-ctrl-d"
        inputCmd = "C:\\intelFPGA_lite\\20.1\\quartus\\bin64\\nios2-terminal.exe --quiet --no-quit-on-ctrl-d"
        self.plotMaxLength = plotLength
        self.previousTimer = 0
        self.data_x = collections.deque([0] * plotLength, maxlen=plotLength)
        self.data_y = collections.deque([0] * plotLength, maxlen=plotLength)
        self.data_z = collections.deque([0] * plotLength, maxlen=plotLength)

        self.pitch = collections.deque([0] * plotLength, maxlen=plotLength)
        self.roll = collections.deque([0] * plotLength, maxlen=plotLength)

        self.thread = None
    
        self.proc = subprocess.Popen(inputCmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print("Communication started with PID ", self.proc.pid)

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

                data_x = struct.unpack('<i', out[:4] )[0] 
                data_y = struct.unpack('<i', out[4:8] )[0]
                data_z = struct.unpack('<i', out[8:12] )[0]

                pitch_data = 180 * math.atan2(-data_x, math.sqrt(data_y*data_y + data_z*data_z)) / np.pi
                roll_data = 180 * math.atan2(data_y, data_z) / np.pi

                # data_xy = math.atan2(data_y,data_x) * 180 / np.pi
                # data_yz = math.atan2(data_z,data_y) * 180 / np.pi
                # data_xz = math.atan2(data_z, data_x) * 180 / np.pi

                # xyz values
                self.data_x.append(data_x)
                self.data_y.append(data_y)
                self.data_z.append(data_z)

                # pitch and roll values
                self.pitch.append(pitch_data)
                self.roll.append(roll_data)


    def close(self):
        print("Ending communication with Nios-II")
        self.proc.terminate()

    def getSerialData(self, frame, lines, lineValueText, lineLabel, timeText):
        currentTimer = time.perf_counter()      # returns the float value of time in seconds. perf_counter -> performance counter
        self.plotTimer = int((currentTimer - self.previousTimer) * 1000)     # the first reading will be erroneous
        self.previousTimer = currentTimer
        timeText.set_text('Plot Interval = ' + str(self.plotTimer) + 'ms')



        lines[0].set_data(range(self.plotMaxLength), self.pitch)
        lines[1].set_data(range(self.plotMaxLength), self.roll)
        # lines[2].set_data(range(self.plotMaxLength), self.angle_xz)

        lineValueText[0].set_text('[' + lineLabel[0] + '] = ' + str(self.pitch[-1]))
        lineValueText[1].set_text('[' + lineLabel[1] + '] = ' + str(self.roll[-1]))
        # lineValueText[2].set_text('[' + lineLabel[2] + '] = ' + str(self.angle_xz[-1]))

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
    ax.set_title('Plot pitch and roll of de10-lite')
    ax.set_xlabel("time")
    ax.set_ylabel("Pitch/roll")
    lineLabel = ("Pitch", "Roll")
    timeText = ax.text(0.30, 0.95, '', transform=ax.transAxes)
    lines = (ax.plot([], [], label=lineLabel[0])[0], ax.plot([], [], label=lineLabel[1])[0])
    lineValueText = (ax.text(0.30, 0.90, '', transform=ax.transAxes), ax.text(0.3, 0.85, '', transform=ax.transAxes), ax.text(0.3, 0.8, '', transform=ax.transAxes))

    # No clue what this animation does but it's some deep matplotlib conspiracy probably
    anim = animation.FuncAnimation(fig, s.getSerialData, fargs=(lines, lineValueText, lineLabel, timeText), interval=pltInterval)    # fargs has to be a tuple

    plt.legend(loc="upper left")
    plt.show() # blocks until the window closes

    s.close()


if __name__ == '__main__':
    main()