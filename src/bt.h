#ifndef __SOL_BT_H__
#define __SOL_BT_H__

#include <stdint.h>
#include <stdbool.h>

void bt_initialize(void);
void bt_send_buffer(uint8_t * buffer, uint8_t len);
void bt_enable(bool enable);

typedef struct _bt_read_frame_param_t
{
  const uint8_t * head;
  uint8_t head_len;
  const uint8_t * tail;
  uint8_t tail_len;
  uint8_t * buffer;
  uint8_t buffer_len;
}bt_read_frame_param_t;

uint8_t bt_read_frame(bt_read_frame_param_t * param);
#endif // __SOL_BT_H__