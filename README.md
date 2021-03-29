# info-proc-cw

## Instructions
### Hardware
- To deploy the hardware, unarchive `260321_updated_fir_hardware.qar` in Quartus. Compile the design and upload it to the FPGA.
- Then, open up a new Eclipse SBT project from template and ensure that the required BSP settings have been configured properly:
    - `sys_clk_timer`: `sys_timer`
    - `timestamp_timer` : `timer`
    - `stdin` : `jtag_uart`
    - `stdout` : `jtag_uart`
    - `stderr` : `jtag_uart`

- Copy and paste the code in `hardware/FPGA_CODE.c` into the project source code. Compile and upload to the FPGA. Ensure that the window in Eclipse that automatically begins monitoring Serial output is closed.


### Software
#### Server
- To get the Server up and running, navigate to the tcp_server folder. 
    1. Run `make`. 
    2. Run `./server`.

#### Client
- To get the Client up and running, navigate to the tcp_client folder.
    1. Run `make network`. 
    2. Run `./network`.
    3. Enter IP address of Server and player name when prompted.

### FPGA Host
- To get the FPGA Host up and running:
    1. Run `./hardware/inout/host_pexpect_server.py` in base directory.
