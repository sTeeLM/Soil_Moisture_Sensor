#ifndef SOL_LOGGER_H
#define SOL_LOGGER_H

#include "esp_log.h"

void logger_init(void);

#define SOL_LOGI(TAG, fmt, ...) ESP_LOGI(TAG, fmt, ##__VA_ARGS__)
#define SOL_LOGE(TAG, fmt, ...) ESP_LOGE(TAG, fmt, ##__VA_ARGS__)
#define SOL_LOGW(TAG, fmt, ...) ESP_LOGW(TAG, fmt, ##__VA_ARGS__)
#define SOL_LOGD(TAG, fmt, ...) ESP_LOGD(TAG, fmt, ##__VA_ARGS__)
#define SOL_LOGV(TAG, fmt, ...) ESP_LOGV(TAG, fmt, ##__VA_ARGS__)

#define SOL_EARLY_LOGI(TAG, fmt, ...) ESP_EARLY_LOGI(TAG, fmt, ##__VA_ARGS__)
#define SOL_EARLY_LOGE(TAG, fmt, ...) ESP_EARLY_LOGE(TAG, fmt, ##__VA_ARGS__)
#define SOL_EARLY_LOGW(TAG, fmt, ...) ESP_EARLY_LOGW(TAG, fmt, ##__VA_ARGS__)
#define SOL_EARLY_LOGD(TAG, fmt, ...) ESP_EARLY_LOGD(TAG, fmt, ##__VA_ARGS__)
#define SOL_EARLY_LOGV(TAG, fmt, ...) ESP_EARLY_LOGV(TAG, fmt, ##__VA_ARGS__)

#define SOL_LOGI_HEX(TAG, buffer, size) ESP_LOG_BUFFER_HEXDUMP(TAG, buffer, size, ESP_LOG_INFO)
#define SOL_LOGE_HEX(TAG, buffer, size) ESP_LOG_BUFFER_HEXDUMP(TAG, buffer, size, ESP_LOG_ERROR)
#define SOL_LOGW_HEX(TAG, buffer, size) ESP_LOG_BUFFER_HEXDUMP(TAG, buffer, size, ESP_LOG_WARN)
#define SOL_LOGD_HEX(TAG, buffer, size) ESP_LOG_BUFFER_HEXDUMP(TAG, buffer, size, ESP_LOG_DEBUG)
#define SOL_LOGV_HEX(TAG, buffer, size) ESP_LOG_BUFFER_HEXDUMP(TAG, buffer, size, ESP_LOG_VERBOSE)

#endif // SOL_LOGGER_H
