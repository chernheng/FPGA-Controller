# info-proc-cw : Team 1

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

#### Dependencies
- Make sure ncurses is installed. If not, run `sudo apt install libncurses5-dev`
- Running the client and server requires either native Linux or WSL 2. Since Nios II for eclipse requires WSL 1 to run, you can either:
    - Program the FPGA using Nios II and Eclipse with WSL 1, then change to WSL 2
    - Modify one of the GNU coreutils in WSL 2 to get Nios II to work with WSL 2 (instructions are shown below)

<details> <summary> Modify WSL 2 to allow Nios II to work </summary>
    
In WSL 2 run the command `which uname`. It should return `/bin/uname`.

Rename the original `uname` command:
```
sudo mv /bin/uname /bin/uname_original
```
Create a bash script `/bin/uname` in its place and insert the following contents:

```bash
#!/bin/bash
PARENT_COMMAND=$(ps -o comm=$PPID)
# echo $PARENT_COMMAND >> ~/log.txt
TXT=$(uname_original $1)
if [[ $PARENT_COMMAND =~ "nios" ]] || [[ $PARENT_COMMAND =~ "create-this" ]] || [[ $PARENT_COMMAND =~ "make" ]]; then
        echo $TXT"-Microsoft"
else
        echo $TXT
fi
```

Change the permissions of the new file: `sudo chmod 755 /bin/uname`

</details>

Else, to swap from WSL1 to WSL2 and vice-versa, simply run `wsl.exe --set-version $DISTRO_NAME $DISTRO_VER` in PowerShell, where `DISTRO_NAME` is the name of your Linux distribution on your computer (eg. `Ubuntu-20.04`) and `DISTRO_VER` is the version of WSL you wish to change to.
To get `DISTRO_NAME`, you can run `wsl.exe --list`.

#### Server
- To get the Server up and running, navigate to the `tcp_server` folder. 
    1. Run `make`. 
    2. Run `./server`.

#### Client and FPGA Host
- To get the Client up and running, navigate to the `tcp_client` folder.
    1. Run `make network`. 
    2. Run `./network`.
    3. Enter IP address of Server and player name when prompted.
- To get the FPGA Host up and running:
    1. Run `./hardware/inout/host_pexpect_server.py` in base directory.
    2. To ensure the FPGA is in a known state, reset it by pressing `KEY0` immediately after running the Python script.

- Subsequently, look at the Client screen. Your FPGA should display a `0` on `HEX5`. When this happens, press `KEY1`.
- Enjoy the game!
