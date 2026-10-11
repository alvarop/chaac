# Source this file to set up the firmware build environment:
#   source fw/chaac/env.sh
#
# Expects the Arm GNU toolchain unpacked under ~/toolchains (override with
# ARM_TOOLCHAIN) and a venv with cmake/ninja (created on first use).

FW_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ARM_TOOLCHAIN="${ARM_TOOLCHAIN:-$HOME/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi}"

if [ ! -x "$ARM_TOOLCHAIN/bin/arm-none-eabi-gcc" ]; then
	echo "arm-none-eabi-gcc not found in $ARM_TOOLCHAIN/bin" >&2
	return 1
fi

if [ ! -d "$FW_DIR/venv" ]; then
	python3 -m venv "$FW_DIR/venv"
	"$FW_DIR/venv/bin/pip" install -q -r "$FW_DIR/requirements.txt"
fi

source "$FW_DIR/venv/bin/activate"
export PATH="$ARM_TOOLCHAIN/bin:$PATH"
