﻿# info-proc-cw

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
- To get the Server up and running, navigate to the tcp_server folder and run `make`. Run `./server` afterwards.

#### Client
- To get the Client up and running, navigate to the tcp_client folder and run `make network`. Run `./network` afterwards.
