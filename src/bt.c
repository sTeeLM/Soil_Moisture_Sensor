#include <STC8G.H>

#include "bt.h"
#include "debug.h"

#include "gpio.h"
#include "bt.h"
#include "cext.h"
#include "delay.h"

#include <string.h>
#include <stdio.h>

static bit bt_is_enabled;

#define    OLEN  32                             // size of serial transmission buffer
static unsigned   char  ostart;                 // transmission buffer start index
static unsigned   char  oend;                   // transmission buffer end index
static char idata outbuf[OLEN];                 // storage for transmission buffer

#define    ILEN  32                              // size of serial receiving buffer
static unsigned   char  istart;                 // receiving buffer start index
static unsigned   char  iend;                   // receiving buffer end index
static char idata inbuf[ILEN];                  // storage for receiving buffer

static bit sendfull;                                   // flag: marks transmit buffer full
static bit sendactive;                                 // flag: marks transmitter active

/*--------------------------------------------------------------------------------
 * Serial Interrupt Service Routine
 *------------------------------------------------------------------------------*/
static void bt_isr (void) interrupt 8 using 2
{
char c;

  //----- Received data interrupt. -----
  if (S2CON & 0x1) { //if (RI2) 
    c = S2BUF;                                   // read character
    S2CON &= ~0x1;    // RI2 = 0;                // clear interrupt request flag
    if (istart + ILEN != iend) {
      inbuf[iend++ & (ILEN-1)] = c;             // put character into buffer
    }
  }

  //----- Transmitted data interrupt. -----
  if (S2CON & 0x2) { // if (TI2)
    S2CON &= ~0x2;  // TI2 = 0;                 // clear interrupt request flag
    if (ostart != oend) {                       // if characters in buffer and
      S2BUF = outbuf[ostart++ & (OLEN-1)];       // transmit character
      sendfull = 0;                             // clear 'sendfull' flag
    }
    else {                                      // if all characters transmitted
      sendactive = 0;                           // clear 'sendactive'
    }
  }
}

/*
  const uint8_t * head;
  uint8_t head_len;
  const uint8_t * tail;
  uint8_t tail_len;
  uint8_t * buffer;
  uint8_t buffer_len;
*/

static bool bt_read_res(void)
{
  bt_read_frame_param_t p;
  uint8_t buffer[32] = {0};
  uint8_t ret = 0;
  uint8_t try_cnt = 0;
  do {
    delay_ms(100);
    p.head="OK";
    p.head_len = 2;
    p.tail="\r";
    p.tail_len = 1;
    p.buffer = buffer;
    p.buffer_len = sizeof(buffer) - 1;
    ret = bt_read_frame(&p);
    try_cnt ++;
  } while(ret == 0 && try_cnt < 10);
  
  if(ret == 3 && buffer[0] == 'O' && buffer[1] == 'K')
    return true;
  
  return false;
}

static void bt_clr_buffer(void)
{
  istart = 0;                                  // empty transmit buffers
  iend = 0;
  ostart = 0;                                  // empty transmit buffers
  oend = 0;
  sendactive = 0;                              // transmitter is not active
  sendfull = 0;                                // clear 'sendfull' flag
}


static char bt_bthome_frame[] = 
"AT+TEADV="
//"AT+ADVHEX="
// 标准 BLE 广播 Flags (3 字节)  
//"020106"
// BTHome 长度: LocalName 之前的长度- 核心 Flags占用的3字节 - 长度自身占用的1字节
//"0916"
// 类型: Service Data, BTHome UUID 字节 (0xFCD2), Device Info: V2版本，无加密
//"D2FC"
"40"
// 填充电量：Object ID = 0x01 (1字节无符号)
"01%02bX"
// 填充湿度：Object ID = 0x03 (2字节无符号, 低字节在前，放大100倍)
"03%02bX%02bX"
// Local Name Object ID = 0x09, “SoilMeter”, 总长度=10（字符串9+类型1）
//"0A09536F696F4D65746572"
"\r\n";

/*
static char bt_bthome_frame[] = 
"AT+TEADV="
// 标准 BLE 广播 Flags (3 字节)  
"\0x02\0x01\0x06"
// BTHome 长度: LocalName 之前的长度- 核心 Flags占用的3字节 - 长度自身占用的1字节
"\0x09\0x16"
// 类型: Service Data, BTHome UUID 字节 (0xFCD2), Device Info: V2版本，无加密
"\0xD2\0xFC\0x40"
// 填充电量：Object ID = 0x01 (1字节无符号)
"\0x01\0x00"
// 填充湿度：Object ID = 0x03 (2字节无符号, 低字节在前，放大100倍)
"\0x03\0x00\0x00"
"\r\n";
*/
static char bt_bthome_buffer[64];
void bt_report_data(uint16_t power, uint16_t mol)
{
  uint8_t len;
  CDBG("bt_report_data %u %u", power, mol);
  
  bt_enable(true);
  delay_ms(1000);
  bt_clr_buffer(); // remove "+READY"
  do {
    
    // 关闭广播
    bt_send_buffer("AT+ENADV=0\r\n", 12);
    if(!bt_read_res())
      break;

    bt_send_buffer("AT+SERVICE=FCD2\r\n", 17);
    if(!bt_read_res())
      break;
    
    bt_send_buffer("AT+WRITE=FCD2\r\n", 15);
    if(!bt_read_res())
      break;  
    
    bt_send_buffer("AT+NOTIFY=FCD2\r\n", 16);
    if(!bt_read_res())
      break;  
    
    // 设置广播数据
    mol *= 10;
    memset(bt_bthome_buffer, 0, sizeof(bt_bthome_buffer));
    len = sprintf(bt_bthome_buffer, bt_bthome_frame, (uint8_t)(power/10), (uint8_t)(mol & 0xFF), (uint8_t)((mol >> 8) & 0xFF));
    /*
    len = 24;
    memcpy(bt_bthome_buffer, bt_bthome_frame, len);
    bt_bthome_buffer[18] = (uint8_t)(power/10);
    bt_bthome_buffer[20] = (uint8_t)(mol & 0xFF);
    bt_bthome_buffer[21] = (uint8_t)(uint8_t)((mol >> 8) & 0xFF);
    */
    CDBG("bthome: %s", bt_bthome_buffer);
    bt_send_buffer(bt_bthome_buffer, len);
    if(!bt_read_res())
      break;
    
    // 设置发射功率10dbm
    bt_send_buffer("AT+POWE=10\r\n", 12);
    if(!bt_read_res())
      break;
    
    // 开启广播
    bt_send_buffer("AT+ENADV=1\r\n", 12);
    if(!bt_read_res())
      break;

    
  } while(0);
}

bit bt_enabled(void)
{
  return bt_is_enabled;
}

void bt_enable(bool enable)
{
  CDBG("bt_enable %bd", enable ? 1 : 0);
  if(enable && !bt_is_enabled) {
    bt_clr_buffer();

    /* 波特率 9600 */
    S2CON = 0x50;		//8位数据,可变波特率
    AUXR |= 0x04;		//定时器时钟1T模式
    T2L = 0xA0;			//设置定时初始值
    T2H = 0xFC;			//设置定时初始值
    AUXR |= 0x10;		//定时器2开始计时
           
    
    S2CON |= 0x10;  //S2REN = 1;
    IE2 |= 1;    //ES2 = 1;   
    
    gpio_set_mode(GPIO_BT_RXD_PORT, GPIO_BT_RXD_BIT, GPIO_MODE_BID);
    gpio_set_mode(GPIO_BT_TXD_PORT, GPIO_BT_TXD_BIT, GPIO_MODE_BID);
    
  } else if(!enable && bt_is_enabled) {
    S2CON &= ~0x10;
    IE2 &= ~0x1;
    
      // 设置为高阻输入，避免漏电
    gpio_set_mode(GPIO_BT_RXD_PORT, GPIO_BT_RXD_BIT, GPIO_MODE_IN);
    gpio_set_mode(GPIO_BT_TXD_PORT, GPIO_BT_TXD_BIT, GPIO_MODE_IN);
    
  }

  GPIO_BT_EN = enable;  
  bt_is_enabled = enable;
}

void bt_initialize(void)
{
  CDBG("bt init");
  bt_is_enabled = true;
  bt_enable(false);
}

static void bt_putchar (char c) {
  if (!sendfull) {                             // transmit only if buffer not full
    if (!sendactive) {                         // if transmitter not active:
      sendactive = 1;                          // transfer first character direct
      S2BUF = c;                                // to S2BUF to start transmission
    }
    else {
      IE2 &= ~0x1; // ES2 = 0;                  // disable serial interrupts during buffer update
      outbuf[oend++ & (OLEN-1)] = c;           // put char to transmission buffer
      if (((oend ^ ostart) & (OLEN-1)) == 0) {
         sendfull = 1;
      }                                        // set flag if buffer is full
      IE2 |= 1;// ES2 = 1;                      // enable serial interrupts again
    }
  } else {
    CDBG("bt_putchar: sendfull");
  }
}

static bool bt_getchar(uint8_t *c) 
{
  if (iend == istart) {
    return 0;                                         // wait until there are characters
  }
  IE2 &= ~0x1; // ES2 = 0;                                      // disable serial interrupts during buffer update
  *c = inbuf[istart++ & (ILEN-1)];
  IE2 |= 1;// ES2 = 1;                                      // enable serial interrupts again
  return 1;
}

void bt_send_buffer(uint8_t * buffer, uint8_t len) 
{
  uint8_t i = 0;
  while(len --) {
    while (sendfull);
    bt_putchar(buffer[i++]);
  }
}

// 状态机状态定义
typedef enum {
    STATE_SEARCH_HEAD,
    STATE_RECEIVE_BODY,
    STATE_SEARCH_TAIL
} frame_state_t;


uint8_t bt_read_frame(bt_read_frame_param_t * p) 
{
  frame_state_t state = STATE_SEARCH_HEAD;
  uint8_t head_match_cnt = 0;
  uint8_t tail_match_cnt = 0;
  uint8_t write_idx = 0;
  uint8_t byte, i;

  // 循环调用低层串口函数
  while (bt_getchar(&byte)) {
    switch (state) {
      case STATE_SEARCH_HEAD:
        // 逐字节匹配帧头
        if (byte == p->head[head_match_cnt]) {
          head_match_cnt++;
          if (head_match_cnt == p->head_len) {
            // 帧头完整匹配成功，将整个帧头一次性写入 buffer
            for (i = 0; i < p->head_len; i++) {
              if (write_idx < p->buffer_len) {
                p->buffer[write_idx++] = p->head[i];
              } else {
                return write_idx; // 溢出截断返回
              }
            }
            state = STATE_RECEIVE_BODY; // 切换到接收主体状态
          }
        } else {
          // 匹配失败，回退重新匹配
          CDBG("bt_read_frame: drop byte %0bx", byte);
          head_match_cnt = (byte == p->head) ? 1 : 0;
        }
        break;

      case STATE_RECEIVE_BODY:
        // 检查缓冲区是否已满
        if (write_idx >= p->buffer_len) {
          CDBG("bt_read_frame: buffer full");
          return write_idx; // 触发截断，直接返回
        }

        // 将当前收到的字节存入缓冲区
        p->buffer[write_idx++] = byte;

        // 判断是否需要匹配帧尾
        if (byte == p->tail[tail_match_cnt]) {
          tail_match_cnt++;
          if (tail_match_cnt == p->tail_len) {
            // 成功匹配完整帧尾，返回包含 head 和 tail 的总长度
            p->buffer[write_idx] = 0;
            CDBG("bt_read_frame: get frame %bd: %s", write_idx, p->buffer);
            return write_idx; 
          }
        } else {
          // 帧尾匹配失败，重置计数器
          tail_match_cnt = (byte == p->tail) ? 1 : 0;
        }
        break;

      default:
        break;
    }
  }  

  // 串口暂时没有更多数据，返回当前已接收的长度（可能为半包）
  CDBG("bt_read_frame: no more data, %bd bytes recved", write_idx);
  
  for(head_match_cnt = 0 ; head_match_cnt < write_idx; head_match_cnt++) {
    CDBG("dump: %02bx ", p->buffer[head_match_cnt]);
  }
  
  return write_idx;
}

