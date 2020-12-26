add_subdirectory(${SRC_DIR}/lib/FreeRTOS lib/freertos)
add_subdirectory(${SRC_DIR}/lib/debug lib/debug)
add_subdirectory(${SRC_DIR}/lib/printf lib/printf)

set(APP_NAME test PARENT_SCOPE)

set(APP_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

set(LIB_FILES
	${SRC_DIR}/lib/i2c/stm32l4_i2c.c
	${SRC_DIR}/lib/sensor/sht3x/sht3x.c
	)

set(LIB_INCLUDES
	${SRC_DIR}/lib/i2c
	${SRC_DIR}/lib/sensor/sht3x
	)

set(APP_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/app_main.c
	${FREERTOS_FILES}
	${PRINTF_FILES}
	${DEBUG_FILES}
	${LIB_FILES}
	PARENT_SCOPE)

set(APP_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}
	${FREERTOS_INCLUDES}
	${PRINTF_INCLUDES}
	${DEBUG_INCLUDES}
	${LIB_INCLUDES}
	PARENT_SCOPE)

#set(APP_DEFINES
#  PARENT_SCOPE)

