#include "info.h"
#include "FreeRTOS.h"
#include "stm32l4xx_hal.h"

extern const char *RELEASE_APP_STR;
extern const char *VERSION_STR;
extern const char *GIT_SHA_STR;
const char *hwidStr = HWID_STR;
const char *appName = APP_NAME;

static const char hex[]= "0123456789ABCDEF";

uint32_t getHWID() {
	const uint32_t *HWID = (const uint32_t *)(UID_BASE);
	return HWID[0] ^ HWID[1] ^ HWID[2];
}

bool getHWIDStr(char *buff, size_t len) {
	configASSERT(buff != NULL);
	bool rval = false;
	uint32_t hwid = getHWID();
	uint8_t *hwid_ptr = (uint8_t *)&hwid;
	if(len >= (sizeof(uint32_t) * 2 + 1)) {
		for(uint8_t byte = 0; byte < sizeof(uint32_t); byte++) {
			*buff++ = hex[(hwid_ptr[3 - byte] >> 4) & 0xF];
			*buff++ = hex[hwid_ptr[3 -byte] & 0xF];
		}
		*buff = 0;
		rval = true;
	}

	return rval;
}

const char *getFWVersionStr() {
	return GIT_SHA_STR;
}

const char *getHWVersionStr() {
	return appName;
}

const char *getFWTypeStr() {
	return hwidStr;
}
