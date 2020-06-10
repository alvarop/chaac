# Chaac Firmware

The latest Chaac hardware uses the STM32L432KC microcontroller and the [Apache Mynewt](https://mynewt.apache.org/) OS for the firmware.

## Building the Firmware
**NOTE: This project currently uses mynewt's top-of-tree code, so the newt-tool will have to be compiled from source. Sorry :(**
1. Install the newt tool. [Instructions here](https://mynewt.apache.org/latest/get_started/index.html).
2. Install the [GNU Arm Embedded Toolchain](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)
3. Clone this repo `git clone https://github.com/alvarop/chaac.git`
4. Cd into the fw directory (this one) `cd fw/chaac/`
5. Run `newt install` to get the Mynewt libraries
6. Run `newt build stm32_boot` to build the bootloader
7. Run `newt build chaac_v1p0`

## Loading the Firmware (using a [SEGGER Jlink](https://www.segger.com/products/debug-probes/j-link/) as a programmer)
1. Make sure the [JLink tools](https://www.segger.com/downloads/jlink/) are installed
2. Connect the JLink to the board's [Tag Connect](http://www.tag-connect.com/TC2030-CTX-NL) programming connector
3. Run `newt load stm32_boot` to load the bootloader
4. Run `newt run chaac_v1p0 0` to load the main firmware and enter a gdb debugging session. Press `c + enter` to start running the code.

## Loading the Firmware using the Bootloader
1. Make sure [mcumgr](https://github.com/apache/mynewt-mcumgr) is installed
2. Connect usb-serial adapter to debug serial port (J3)
3. Enter the bootloader by holding the SCL line low (with a wire) and press the reset button.
4. Run `mcumgr --conntype serial --connstring "dev=/dev/ttyUSB0,mtu=256" image upload -e bin/targets/chaac_v1p0/app/apps/controller/controller.img`, replacing ttyUSB0 with the appropriate serial port
5. When finished, push the reset button again

## Loading the Firmware remotely
1. Make sure [mcumgr](https://github.com/apache/mynewt-mcumgr) is installed
2. Take a look at the [/sw/scripts/update.py](/sw/scripts/update.py) script.
