#include "usbd_cdc.h"

//
// Static malloc function to get usb cdc to work
// See:
// * https://community.st.com/s/question/0D50X0000BCNOz7/why-does-the-hal-usb-cdc-driver-call-malloc-in-an-isr-context-again
// * https://www.st.com/resource/en/user_manual/dm00108129-stm32cube-usb-device-library-stmicroelectronics.pdf
//
// NOTE: The header usbd_conf.h must be updated to point USBD_malloc/free
// to these static functions. Unfortunately, CubeMX will overwrite these every
// time it re-generates code.
//

void *USDB_static_malloc(uint32_t size) {
  static uint32_t mem[(sizeof(USBD_CDC_HandleTypeDef)/4)+1]; /* on 32-bit boundary*/
  (void)(size);
  return mem;
}

void USDB_static_free(void *p) {
  (void)p;
}
