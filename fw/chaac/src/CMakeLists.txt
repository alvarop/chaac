set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if (NOT DEFINED BSP)
	message(SEND_ERROR "BSP is not defined")
	return()
endif()

if (NOT DEFINED APP)
	message(SEND_ERROR "APP is not defined")
	return()
endif()

if (NOT DEFINED HWID)
	message("HWID is not defined, using BSP")
	set(HWID ${BSP})
endif()

set(DEBUGGER "bmp" CACHE STRING "Options: bmp, stlink")

add_subdirectory(bsp/${BSP})
add_subdirectory(app/${APP})

set(EXECUTABLE ${PROJECT_NAME}-${APP_NAME}.elf)

add_compile_definitions(
	${BSP_DEFINES}
	${APP_DEFINES}
	HWID_STR="${HWID}"
	APP_NAME="${APP}"
	$<$<CONFIG:Debug>:BUILD_DEBUG>
	$<$<CONFIG:Release>:BUILD_RELEASE>
	)

add_compile_options(
	-g
	-ggdb

	$<$<CONFIG:Debug>:-Og>
	$<$<CONFIG:Release>:-O2>

	# TODO
	#	-ffile-prefix-map=${CMAKE_SOURCE_DIR}=.

	$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
	$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
	)

set(MEMFAULT_PORT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/memfault)
set(MEMFAULT_SDK_ROOT ${MEMFAULT_PORT_ROOT}/memfault-firmware-sdk)
set(MEMFAULT_FREERTOS_PORT_ROOT ${MEMFAULT_SDK_ROOT}/ports/freertos)

list(APPEND MEMFAULT_COMPONENTS core util panics metrics)
include(${MEMFAULT_SDK_ROOT}/cmake/Memfault.cmake)
memfault_library(${MEMFAULT_SDK_ROOT} MEMFAULT_COMPONENTS
 MEMFAULT_COMPONENTS_SRCS MEMFAULT_COMPONENTS_INC_FOLDERS)

# TODO get bsp from variable
add_executable(${EXECUTABLE})

target_sources(${EXECUTABLE} PRIVATE
	${BSP_FILES}
	${APP_FILES}
	${CMAKE_CURRENT_BINARY_DIR}/version.c
	${MEMFAULT_COMPONENTS_SRCS}
	${MEMFAULT_PORT_ROOT}/memfault_platform_port.c
	${MEMFAULT_PORT_ROOT}/memfault_rcc_reboot_tracking.c
	${MEMFAULT_PORT_ROOT}/memfault_flash_coredump_storage.c
	${MEMFAULT_FREERTOS_PORT_ROOT}/src/memfault_core_freertos.c
	${MEMFAULT_FREERTOS_PORT_ROOT}/src/memfault_freertos_ram_regions.c
	${MEMFAULT_FREERTOS_PORT_ROOT}/src/memfault_metrics_freertos.c
	${MEMFAULT_FREERTOS_PORT_ROOT}/src/memfault_panics_freertos.c)

target_include_directories(${EXECUTABLE} PRIVATE
	${BSP_INCLUDES}
	${APP_INCLUDES}
	${MEMFAULT_COMPONENTS_INC_FOLDERS}
	${MEMFAULT_SDK_ROOT}/ports/include
	${MEMFAULT_PORT_ROOT})

target_linker_map(${EXECUTABLE})

# Print executable size
add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
        COMMAND ${SIZE} ${EXECUTABLE})

# Generate hex file
add_custom_command(
	TARGET ${EXECUTABLE} POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE} ${EXECUTABLE}.hex
	BYPRODUCTS ${EXECUTABLE}.hex
)

# Generate bin file
add_custom_command(
	TARGET ${EXECUTABLE} POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE} ${EXECUTABLE}.bin
	BYPRODUCTS ${EXECUTABLE}.bin
)

# Generate lst file
add_custom_command(
	TARGET ${EXECUTABLE} POST_BUILD
	COMMAND ${CMAKE_OBJDUMP} -S -d -w -l ${EXECUTABLE} > ${EXECUTABLE}.lst
	BYPRODUCTS ${EXECUTABLE}.lst
)

if(${DEBUGGER} STREQUAL "bmp")
	add_custom_target(flash
		COMMAND bash ${CMAKE_SOURCE_DIR}/tools/flash_bmp.sh ${EXECUTABLE}
		DEPENDS ${EXECUTABLE}
	)

	add_custom_target(debug
		COMMAND bash ${CMAKE_SOURCE_DIR}/tools/debug_bmp.sh ${EXECUTABLE}
	)
elseif(${DEBUGGER} STREQUAL "stlink")
	add_custom_target(flash
		COMMAND bash ${CMAKE_SOURCE_DIR}/tools/flash_stlink.sh ${EXECUTABLE}
		DEPENDS ${EXECUTABLE}
	)

	add_custom_target(debug
		COMMAND bash ${CMAKE_SOURCE_DIR}/tools/debug_stlink.sh ${EXECUTABLE}
	)

	add_custom_target(debug_reset
		COMMAND bash ${CMAKE_SOURCE_DIR}/tools/debug_stlink_reset.sh ${EXECUTABLE}
	)
else()
	message("No debugger selected!!!")
endif()


add_custom_target(dfu_flash
	COMMAND dfu-util -d 0483:df11 -c 1 -i 0 -a 0 -s 0x8000000:leave -D ${EXECUTABLE}.bin
	DEPENDS ${EXECUTABLE}
)

message(STATUS "Saving to ${CMAKE_CURRENT_BINARY_DIR}/version.c")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version.c
           ${CMAKE_CURRENT_BINARY_DIR}/_version.c
    COMMAND ${CMAKE_COMMAND} -P
            ${CMAKE_SOURCE_DIR}/cmake/git_version.cmake)
