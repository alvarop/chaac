## Toolchain
- Arm GNU Toolchain 13.2.rel1 (x86_64, arm-none-eabi) unpacked in `~/toolchains/`
  (https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads). Override the path with `ARM_TOOLCHAIN`.
- CMake and Ninja come from a local venv (`requirements.txt`), created by `env.sh` on first use.

## Build
```
source env.sh
cmake -B build/usb_lora_v7 -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/cross/STM32L412KBUx.cmake \
      -DAPP=usb_lora -DBSP=chaac_v7p0 -DCMAKE_BUILD_TYPE=Release
ninja -C build/usb_lora_v7
```

## Flash
Install the udev rules once (DFU + CDC access, ModemManager ignore): see `tools/70-chaac.rules`.

- `ninja -C <build> dfu_flash` — dfu-util over USB (STM32 ROM bootloader, `0483:df11`).
  A running usb_lora board can be put in DFU with `sw/chaac_logger/chaac_util.py <dev> dfu`.
- `ninja -C <build> flash` — Black Magic Probe (needs `arm-none-eabi-gdb`, which needs `libncursesw.so.5`),
  or configure with `-DDEBUGGER=stlink` for OpenOCD.
