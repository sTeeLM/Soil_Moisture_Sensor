#ifndef SOL_BLE_REPORTER_H
#define SOL_BLE_REPORTER_H

#include <stdint.h>
#include <stdbool.h>

void ble_reporter_init(void);
void ble_reporter_enable(bool enable);
bool ble_reporter_is_enabled(void);
void ble_reporter_set_data(uint16_t hygrometer_percent, uint16_t battery_percent);
#endif //SOL_BLE_REPORTER_H