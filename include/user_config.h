#ifndef __USER_CONFIG_H
#define __USER_CONFIG_H

extern void (*sta_start_func)(char *, char *,unsigned char *);
extern void (*ap_start_func)(char *, char *, int);


#define VER_DEBUG 0 //用于演示

#define S_DEBUG 0

#define M_DEBUG 0

#define DONOT_UPGRADE 0



#define SAVE_ATONCE 0

//#define uart0_sendStr  serial_printf
#define at_backOk        uart0_sendStr("+ok\r\n\r\n")
#define at_backError     uart0_sendStr("+ERR\r\n\r\n")
#define at_backTail      uart0_sendStr("\r\n\r\n")
#define at_backErrHead   uart0_sendStr("+ERR=")
#define at_backOkHead    uart0_sendStr("+ok=")


#define MOM100P0 0
#define M0M100D0 1
#define PLATFORM M0M100D0



struct user_save
{
  unsigned short check_sum;//用于判断配置文件是否正确
  unsigned char echoFlag;//AT指令是否回显，大于0则回显示
  unsigned int magic_flag;//幻码
};


#define CHECKMAC(mac) (mac[0]|mac[1]|mac[2]|mac[3]|mac[4]|mac[5])


#define FS1_FLASH_SIZE      0x40000


#define FS1_FLASH_ADDR      0xA0000//(1024*1024)

#define SECTOR_SIZE         (4*1024) 
#define LOG_BLOCK           (SECTOR_SIZE)
#define LOG_PAGE            (128)

#define FD_BUF_SIZE         32*4
#define CACHE_BUF_SIZE      (LOG_PAGE + 32)*8

#define at_cmdLenMax 128
#define at_dataLenMax 1460

#define MAX_CHN_LEN		2

#define TASK_HEAP_LEN 2048

typedef struct
{
	unsigned char *readCnt;
	unsigned char *writeCnt;
}net_bufferCnt;

#define UART_TIME_SEND 30


#define sw16(x) \
    ((uint16_t)( \
        (((uint16_t)(x) & (uint16_t)0x00ffU) << 8 ) | \
        (((uint16_t)(x) & (uint16_t)0xff00U) >> 8 ) ))

#define sw32(x) \
((uint32_t)( \
	(((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) | \
	(((uint32_t)(x) & (uint32_t)0x0000ff00UL) << 8) | \
	(((uint32_t)(x) & (uint32_t)0x00ff0000UL) >> 8) | \
	(((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24) ))


typedef enum
{
	STA_LINK_DOWN = 0,
	STA_LINK_UP,
	STA_LINK_GET_IP,
	STA_LINK_GODOWN,	
}staStatus;

typedef struct{
	unsigned char led_sts:1;
	unsigned char s_linkup:2;
	unsigned char reset_wifi_flag:2;
	unsigned char try_time:5;
	unsigned char state:3;
	unsigned char wifi_ready:1;
	unsigned char smart_ready:1;
	unsigned char cnt_err_times:3;
}wifi_sys;

typedef enum
{
	UPDATE_SOFT_TYPE = 0,
	UPDATE_WEB_TYPE = 1,
}updateType;

#define AT_CMD_RX_BUFSIZE 256

#define LVERSTRING ".003"


typedef struct
{
	char *at_cmdName;
	signed char at_cmdLen;
	void (*at_setupCmd)(unsigned char id, char *pPara);
	void (*at_exeCmd)(unsigned char id);
}at_funType;


struct airkiss_status
{
	unsigned char isChanLock:1;
	unsigned char isRecvPkt:1;
	unsigned char isGotSSID:1;
	unsigned char airkiss_channel:4;
	unsigned char random;
	unsigned int chn_found_time;
};

#define MOM100P0 0
#define M0M100D0 1
#define PLATFORM M0M100D0


#endif
