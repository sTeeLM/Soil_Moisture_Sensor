#include "ble_reporter.h"
#include "logger.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_mac.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"

static const char * TAG = "BLE";
static bool ble_reporter_enabled;
static TaskHandle_t ble_reporter_task_handle = NULL;

static char ble_repoter_device_name[32];

static uint16_t ble_reporter_data[4]; // 0: hygrometer_percent, 1: battery_percent, 2: battery_low, 3: battery_charging

void ble_reporter_set_data(uint16_t hygrometer_percent,
                           uint16_t battery_percent,
                           uint8_t battery_low,
                           uint8_t battery_charging) 
{
  ble_reporter_data[0] = hygrometer_percent;
  ble_reporter_data[1] = battery_percent;
  ble_reporter_data[2] = battery_low;
  ble_reporter_data[3] = battery_charging;

}

static void ble_reporter_set_data_internal(uint16_t hygrometer_percent,
                           uint16_t battery_percent,
                           uint16_t battery_low,
                           uint16_t battery_charging) 
{

  uint8_t bthome_adv[31] = {0};
  uint8_t bthome_scan_rsp[31] = {0};
  uint8_t idx_adv = 0;
  uint8_t idx_rsp = 0;
  int rc;
  uint8_t len_idx_adv, len_rsp_idx;
  uint16_t h_val;

  // 标准 BLE 广播 Flags (3 字节)
  bthome_adv[idx_adv++] = 0x02;
  bthome_adv[idx_adv++] = 0x01;
  bthome_adv[idx_adv++] = 0x06;

  // BTHome 报头占位（先跳过长度字节，后面动态计算）
  len_idx_adv = idx_adv;
  idx_adv++;                    // 留给 bthome_adv[len_idx] 填 Service Data 长度
  bthome_adv[idx_adv++] = 0x16; // 类型: Service Data
  bthome_adv[idx_adv++] = 0xD2; // BTHome UUID 低字节 (0xFCD2)
  bthome_adv[idx_adv++] = 0xFC; // BTHome UUID 高字节
  bthome_adv[idx_adv++] = 0x40; // Device Info: V2版本，无加密

  // 回填精确的数据包长度
  // 长度 = 总体写入字节数 - 核心 Flags占用的3字节 - 长度自身占用的1字节
  bthome_adv[len_idx_adv] = idx_adv - 4;

  // Complete Local Name
  bthome_adv[idx_adv++] = 0x10; // 长度：后面 1 字节类型 + 15 字节文本 = 16 字节
  bthome_adv[idx_adv++] = 0x09; // 类型标志：0x09 代表 Complete Local Name (完整本地设备名)
  memcpy(&bthome_adv[idx_adv], ble_repoter_device_name, 15); 
  idx_adv += 15; // 设备名长度为15字节

  len_rsp_idx = idx_rsp;
  idx_rsp++; // 留着回填响应包 BTHome 数据长度
  bthome_scan_rsp[idx_rsp++] = 0x16;
  bthome_scan_rsp[idx_rsp++] = 0xD2;
  bthome_scan_rsp[idx_rsp++] = 0xFC;
  bthome_scan_rsp[idx_rsp++] = 0x40;

  // 填充电量：Object ID = 0x01 (1字节无符号)
  bthome_scan_rsp[idx_rsp++] = 0x01;
  bthome_scan_rsp[idx_rsp++] = (uint8_t)(battery_percent / 10); // 电量百分比放大10倍，1字节即可

  // 填充湿度：Object ID = 0x03 (2字节无符号, 放大100倍)
  h_val = (uint16_t)(hygrometer_percent * 10);
  bthome_scan_rsp[idx_rsp++] = 0x03;
  bthome_scan_rsp[idx_rsp++] = (uint8_t)(h_val & 0xFF);
  bthome_scan_rsp[idx_rsp++] = (uint8_t)((h_val >> 8) & 0xFF);

  // 电池状态：
  bthome_scan_rsp[idx_rsp++] = 0x15; // Object ID for battery state
  bthome_scan_rsp[idx_rsp++] = (uint8_t)(battery_low); // 电池状态值

  // 充电状态：
  bthome_scan_rsp[idx_rsp++] = 0x16; // Object ID for charging state
  bthome_scan_rsp[idx_rsp++] = (uint8_t)(battery_charging); // 充电状态值

  // 回填响应包中 BTHome 数据段的精确字节长度
  bthome_scan_rsp[len_rsp_idx] = idx_rsp - (len_rsp_idx + 1);

  rc = ble_gap_adv_set_data(bthome_adv, idx_adv);
  if (rc != 0) {
      SOL_LOGE(TAG, "Failed to set BLE adv data: %d", rc);
      return;
  } else {
      SOL_LOGI(TAG, "BTHome report adv data: %d bytes", idx_adv);
  }

  rc = ble_gap_adv_rsp_set_data(bthome_scan_rsp, idx_rsp);
  if (rc != 0) {
    SOL_LOGE(TAG, "Failed to set BLE adv rsp data: %d", rc);
    return;
  } else {
    SOL_LOGI(TAG, "BTHome report scan response data: %d bytes", idx_rsp);
  }   
}

static void ble_reporter_task(void *param) 
{
  nimble_port_run(); // 阻塞运行，维持蓝牙协议栈

  nimble_port_freertos_deinit();

  vTaskDelete(NULL); 

  ble_reporter_task_handle = NULL;
}

static void ble_reporter_start(void);
static void ble_reporter_stop(void);
static void ble_reporter_app_on_sync(void) 
{
  SOL_LOGD(TAG, "BLE sync callback triggered, ble_reporter_enabled = %d", ble_reporter_enabled);
  ble_reporter_set_data_internal(ble_reporter_data[0], ble_reporter_data[1], ble_reporter_data[2], ble_reporter_data[3]);  
}

void ble_reporter_init(void)
{
  esp_err_t ret = ESP_OK;
  uint8_t bt_mac[6];

  SOL_LOGI(TAG, "init");

  ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    SOL_LOGW(TAG, "nvs flash init failed, try erase and init again");
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ret = esp_read_mac(bt_mac, ESP_MAC_BT);
  if (ret != ESP_OK) {
    SOL_LOGE(TAG, "Failed to read BT MAC address");
    return;
  }

  // 11 + 4 = 15 
  snprintf(ble_repoter_device_name, sizeof(ble_repoter_device_name), 
    "SolMoister-%02X%02X", bt_mac[4], bt_mac[5]);

  ble_repoter_device_name[sizeof(ble_repoter_device_name) - 1] = '\0';

  SOL_LOGI(TAG, "BLE device name: %s", ble_repoter_device_name);

  ble_reporter_enabled = false;
}



static void ble_reporter_start(void) 
{
  // 配置并【单次启动】底层 GAP 广播
  struct ble_gap_adv_params adv_params;
  int rc = 0;

  ESP_ERROR_CHECK(nimble_port_init());
  ble_svc_gap_device_name_set("SolMoister");
  ble_hs_cfg.sync_cb = ble_reporter_app_on_sync;
  // 启动蓝牙宿主线
  xTaskCreate(ble_reporter_task, "ble_host_task", 4096, NULL, 5, &ble_reporter_task_handle);

  memset(&adv_params, 0, sizeof(adv_params));
  adv_params.conn_mode = BLE_GAP_CONN_MODE_NON; 
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; 
  adv_params.itvl_min = BLE_GAP_ADV_ITVL_MS(200); // 正常工作建议调到 200~300ms 发一次，更省电
  adv_params.itvl_max = BLE_GAP_ADV_ITVL_MS(300); 

  rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &adv_params, NULL, NULL);
  if (rc == 0) {
     SOL_LOGI(TAG, "BLE Broadcast started successfully");
  } else {
    SOL_LOGE(TAG, "BLE Broadcast failed: %d", rc);
  }
}

static void ble_reporter_stop(void) 
{
  ble_gap_adv_stop();
  nimble_port_stop();
  nimble_port_deinit();
}

void ble_reporter_enable(bool enable)
{
  SOL_LOGI(TAG, "ble_reporter_enable: %s", enable ? "true" : "false");
  if(enable && !ble_reporter_enabled) {
    // 这里不需要做什么，因为 BLE 广播已经在初始化时启动了，后续通过更新广告数据来上报即可
    SOL_LOGI(TAG, "BLE enabled");
    ble_reporter_start();
    ble_reporter_enabled = true;
  } else if(!enable && ble_reporter_enabled) {
    ble_reporter_stop();
    SOL_LOGI(TAG, "BLE disabled");
    ble_reporter_enabled = false;
  }
}

bool ble_reporter_is_enabled(void)
{
  return ble_reporter_enabled;
}
