set(NO_LPTIMTICK true)

add_subdirectory(${SRC_DIR}/lib/FreeRTOS lib/freertos)
add_subdirectory(${SRC_DIR}/lib/printf lib/printf)

set(APP_NAME usb_lora PARENT_SCOPE)

set(APP_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

set(RADIO_FILES
	${SRC_DIR}/lib/radio/src/radio.c
	${SRC_DIR}/lib/radio/src/sx126x.c
	)

set(RADIO_INCLUDES
	${SRC_DIR}/lib/radio/inc
	)

set(LIB_FILES
	${SRC_DIR}/lib/common/info.c
	${SRC_DIR}/lib/common/usb_dfu.c
	${SRC_DIR}/lib/crc/crc.c
	${SRC_DIR}/lib/loraRadio/loraRadio.c
	${SRC_DIR}/lib/packet/packet.c
	${SRC_DIR}/lib/vcp/vcp.c
	)

set(LIB_INCLUDES
	${SRC_DIR}/lib/chaac
	${SRC_DIR}/lib/common/
	${SRC_DIR}/lib/crc
	${SRC_DIR}/lib/debug
	${SRC_DIR}/lib/loraRadio
	${SRC_DIR}/lib/packet
	${SRC_DIR}/lib/vcp
	)

set(APP_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/app_main.c
	${FREERTOS_FILES}
	${PRINTF_FILES}
	${LIB_FILES}
	${RADIO_FILES}
	PARENT_SCOPE)

set(APP_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}
	${FREERTOS_INCLUDES}
	${PRINTF_INCLUDES}
	${LIB_INCLUDES}
	${RADIO_INCLUDES}
	PARENT_SCOPE)
