set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(bsp/chaac_v6p0)
add_subdirectory(app/test)

set(EXECUTABLE ${PROJECT_NAME}-${APP_NAME}.elf)

add_compile_definitions(
	${BSP_DEFINES}
	${APP_DEFINES}
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

# TODO get bsp from variable
add_executable(${EXECUTABLE})

target_sources(${EXECUTABLE} PRIVATE 
	${BSP_FILES}
	${APP_FILES})

target_include_directories(${EXECUTABLE} PRIVATE 
	${BSP_INCLUDES}
	${APP_INCLUDES})

target_linker_map(${EXECUTABLE})

# TODO - Figure out how to do this from bsp dir
# Add file-specific flags for lib compiler warnings
set_source_files_properties(
	${BSP_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_exti.c
	PROPERTIES
	COMPILE_FLAGS -Wno-unused-parameter)

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


add_custom_target(flash
	COMMAND bash ${CMAKE_SOURCE_DIR}/tools/flash_bmp.sh ${EXECUTABLE}
	DEPENDS ${EXECUTABLE}
)

add_custom_target(dfu_flash
	COMMAND bash dfu-util -d 0483:df11 -c 1 -i 0 -a 0 -s 0x8000000:leave -D ${PROJECT_NAME}.bin
	DEPENDS ${EXECUTABLE}
)

add_custom_target(debug
	COMMAND bash ${CMAKE_SOURCE_DIR}/tools/debug.sh ${EXECUTABLE}
)

