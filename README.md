# info-proc-cw

## `hardware`:
1. Unarchive the `DE10_Lite_Nios_II_All_IO.qar` file using Quartus into a directory of your choice. It simply contains a set-up NIOS-II system with all the I/O devices connected to it (LED, HEX0-5, switches, buttons)
2. Open up Eclipse and make a new NIOS-II project. Setup the BSP accordingly and copy and paste in the source code `basic_serial_comms.c`.
    _(as an aside, why is Eclipse such a bitch with version control? If anyone knows how to share the project directory in a more convenient manner it would be great - if not we all just have to copy and paste in the new code everytime someone pushes to git.)_
3. Take a look at the different Python implementations for the host controller. They should be commented liberally.
4. Run `hello_world_small.c` on the Nios-II and `host_threaded.py` on the host computer and hopefully observe XYZ accelerometer being plotted in real time.

### Todo:
- Proper UDP server running on host machine to receive data from JTAG
- TCP communications <----> Serial communications for (among other things) changing FIR filter state, FIR filter coefficients, etc.
- Hardware FIR filtering on FPGA fabric
- Reading and writing to FPGA PIO: eg reading if buttons are pressed, displaying strings over the sev_seg displays.