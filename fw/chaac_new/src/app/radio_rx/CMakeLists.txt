add_subdirectory(${SRC_DIR}/lib/FreeRTOS lib/freertos)
add_subdirectory(${SRC_DIR}/lib/printf lib/printf)

set(APP_NAME radio_rx PARENT_SCOPE)

set(APP_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

set(RADIO_FILES
	${SRC_DIR}/lib/LoRaMac-node/src/radio/sx126x/radio.c
	${SRC_DIR}/lib/LoRaMac-node/src/radio/sx126x/sx126x.c
	)

set(RADIO_INCLUDES
	${SRC_DIR}/lib/LoRaMac-node/src/radio/
	${SRC_DIR}/lib/LoRaMac-node/src/radio/sx126x/
	${SRC_DIR}/lib/LoRaMac-node/src/boards/
	)

set(LIB_FILES
	${SRC_DIR}/lib/debug/debug.c
	# ${SRC_DIR}/lib/adc/stm32l4_adc.c
	# ${SRC_DIR}/lib/i2c/stm32l4_i2c.c
	# ${SRC_DIR}/lib/sensor/sht3x/sht3x.c
	# ${SRC_DIR}/lib/sensor/dps368/dps368.c
	# ${SRC_DIR}/lib/sensor/windrain/windrain.c
	)

set(LIB_INCLUDES
	${SRC_DIR}/lib/debug
	# ${SRC_DIR}/lib/adc
	# ${SRC_DIR}/lib/i2c
	# ${SRC_DIR}/lib/sensor/sht3x
	# ${SRC_DIR}/lib/sensor/dps368
	# ${SRC_DIR}/lib/sensor/windrain
	)

set(APP_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/app_main.c
	${FREERTOS_FILES}
	${PRINTF_FILES}
	${LIB_FILES}
	# ${RADIO_FILES}
	PARENT_SCOPE)

set(APP_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}
	${FREERTOS_INCLUDES}
	${PRINTF_INCLUDES}
	${LIB_INCLUDES}
	# ${RADIO_INCLUDES}
	PARENT_SCOPE)

#set(APP_DEFINES
#  PARENT_SCOPE)

