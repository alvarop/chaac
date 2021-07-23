//! @file
//!
//! Copyright (c) Memfault, Inc.
//! See License.txt for details
//!
//! Glue layer between the Memfault SDK and the underlying platform
//!
//! TODO: Fill in FIXMEs below for your platform

#include "memfault/components.h"
#include "memfault/ports/reboot_reason.h"
#include "memfault/ports/freertos.h"
#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "info.h"

#include <stdbool.h>

static char serialStr[sizeof(uint32_t)*2 + 1];

void memfault_platform_get_device_info(sMemfaultDeviceInfo *info) {
  // !FIXME: Populate with platform device information

  // IMPORTANT: All strings returned in info must be constant
  // or static as they will be used _after_ the function returns

  // See https://mflt.io/version-nomenclature for more context
  *info = (sMemfaultDeviceInfo) {
    // An ID that uniquely identifies the device in your fleet
    // (i.e serial number, mac addr, chip id, etc)
    // Regular expression defining valid device serials: ^[-a-zA-Z0-9_]+$
    .device_serial = serialStr,
     // A name to represent the firmware running on the MCU.
    // (i.e "ble-fw", "main-fw", or a codename for your project)
    .software_type = getFWTypeStr(),
    // The version of the "software_type" currently running.
    // "software_type" + "software_version" must uniquely represent
    // a single binary
    .software_version = getFWVersionStr(),
    // The revision of hardware for the device. This value must remain
    // the same for a unique device.
    // (i.e evt, dvt, pvt, or rev1, rev2, etc)
    // Regular expression defining valid hardware versions: ^[-a-zA-Z0-9_\.\+]+$
    .hardware_version = getHWVersionStr(),
  };
}

//! Last function called after a coredump is saved. Should perform
//! any final cleanup and then reset the device
void memfault_platform_reboot(void) {
  // !FIXME: Perform any final system cleanup here

  NVIC_SystemReset();
  while (1) { } // unreachable
}

bool memfault_platform_time_get_current(sMemfaultCurrentTime *time) {
  (void)time;
  return false;
}

size_t memfault_platform_sanitize_address_range(void *start_addr, size_t desired_size) {
  static const struct {
    uint32_t start_addr;
    size_t length;
  } s_mcu_mem_regions[] = {
    // !FIXME: Update with list of valid memory banks to collect in a coredump
    {.start_addr = 0x00000000, .length = 0xFFFFFFFF},
  };

  for (size_t i = 0; i < MEMFAULT_ARRAY_SIZE(s_mcu_mem_regions); i++) {
    const uint32_t lower_addr = s_mcu_mem_regions[i].start_addr;
    const uint32_t upper_addr = lower_addr + s_mcu_mem_regions[i].length;
    if ((uint32_t)start_addr >= lower_addr && ((uint32_t)start_addr < upper_addr)) {
      return MEMFAULT_MIN(desired_size, upper_addr - (uint32_t)start_addr);
    }
  }

  return 0;
}

//! !FIXME: This function _must_ be called by your main() routine prior
//! to starting an RTOS or baremetal loop.
int memfault_platform_boot(void) {
  // !FIXME: Add init to any platform specific ports here.
  // (This will be done in later steps in the getting started Guide)

  configASSERT(getHWIDStr(serialStr, sizeof(serialStr)));

  memfault_freertos_port_boot();
  memfault_build_info_dump();
  memfault_device_info_dump();
  memfault_platform_reboot_tracking_boot();

  static uint8_t s_event_storage[1024];
  const sMemfaultEventStorageImpl *evt_storage =
      memfault_events_storage_boot(s_event_storage, sizeof(s_event_storage));
  memfault_trace_event_boot(evt_storage);

  memfault_reboot_tracking_collect_reset_info(evt_storage);

  sMemfaultMetricBootInfo boot_info = {
    .unexpected_reboot_count = memfault_reboot_tracking_get_crash_count(),
  };
  memfault_metrics_boot(evt_storage, &boot_info);

  MEMFAULT_LOG_INFO("Memfault Initialized!");

  return 0;
}


MEMFAULT_PUT_IN_SECTION(".noinit.mflt_reboot_tracking")
static uint8_t s_reboot_tracking[MEMFAULT_REBOOT_TRACKING_REGION_SIZE];

void memfault_platform_reboot_tracking_boot(void) {
  sResetBootupInfo reset_info = { 0 };
  memfault_reboot_reason_get(&reset_info);
  memfault_reboot_tracking_boot(s_reboot_tracking, &reset_info);
}

void memfault_platform_log(eMemfaultPlatformLogLevel level, const char *fmt, ...) {
  (void)level;
  (void)fmt;
}

const sMfltCoredumpRegion *memfault_platform_coredump_get_regions(const sCoredumpCrashInfo *crash_info,
                                                                  size_t *num_regions) {
  (void)crash_info;
  static sMfltCoredumpRegion s_coredump_regions[1];
  // NOTE: This is just an example of regions which could be collected
  // Typically sizes are derived from variables added to the .ld script of the port

  s_coredump_regions[0] = MEMFAULT_COREDUMP_MEMORY_REGION_INIT((void *)0x20000000,
      1024);

  *num_regions = MEMFAULT_ARRAY_SIZE(s_coredump_regions);
  return s_coredump_regions;
}

// Disable coredumps for now
bool memfault_coredump_read(uint32_t offset, void *buf, size_t buf_len) {
  (void)offset;
  (void)buf;
  (void)buf_len;
  return false;
}

// memfault_platform_get_time_since_boot_ms()