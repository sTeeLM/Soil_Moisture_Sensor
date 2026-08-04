#include "ble_reporter.h"
#include "logger.h"

static const char * TAG = "BLE";
static bool ble_reporter_enabled;

void ble_reporter_init(void)
{
  SOL_LOGI(TAG, "init");
  ble_reporter_enabled = false;
}

void ble_reporter_enable(bool enable)
{
  SOL_LOGI(TAG, "ble_reporter_enable: %s", enable ? "true" : "false");
  ble_reporter_enabled = enable;
}

bool ble_reporter_is_enabled(void)
{
  return ble_reporter_enabled;
}

void ble_reporter_set_data(uint16_t hygrometer_percent, uint16_t battery_percent)
{
  SOL_LOGI(TAG, "ble_reporter_set_data: hygrometer %u, battery %u", hygrometer_percent, battery_percent);
}