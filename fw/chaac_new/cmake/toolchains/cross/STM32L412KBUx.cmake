##############################
# STM32L412KBUx (Cortex-M4F) #
##############################

# CMake includes the toolchain file multiple times when configuring the build,
# which causes errors for some flags (e.g., --specs=nano.specs).
# We prevent this with an include guard.
if(STM32L412KBUx_TOOLCHAIN_INCLUDED)
	return()
endif()

set(STM32L412KBUx_TOOLCHAIN_INCLUDED true)

set(CPU_NAME STM32L412KBUx)

set(LINKER_SCRIPT_DIR ${CMAKE_SOURCE_DIR}/cmake/linker-scripts/stm)
set(LINKER_SCRIPT_PRIMARY_FILE STM32L412KBUx_FLASH.ld)

# These dependencies are applied to add_executable targets by the
# AddExecutableWithLinkerScriptDep module
list(APPEND LINKER_SCRIPT_DEPENDENCIES "${LINKER_SCRIPT_DIR}/${LINKER_SCRIPT_PRIMARY_FILE}")

set(LD_FLAGS "-T${LINKER_SCRIPT_PRIMARY_FILE} -L${LINKER_SCRIPT_DIR}")

include(${CMAKE_CURRENT_LIST_DIR}/cortex-m4_hardfloat.cmake)

