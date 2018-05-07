#ifndef __MYLINKS_WIFI_H
#define __MYLINKS_WIFI_H

#include <lwip/ip_addr.h>
#include <wla_api.h>



typedef void (*mylinks_config_callback)(char *ssid, char *key, int mode, int error);
#define CFG_FLASH_USER_START   0x18000

struct station_config {
	unsigned char ssid[32];
	unsigned char password[64];
	unsigned char bssid_set;
	unsigned char bssid[6];
};

struct softap_config {
    unsigned char ssid[32];
    unsigned char password[64];
    unsigned char ssid_len;	// Note: Recommend to set it according to your ssid
    unsigned char channel;	// Note: support 1 ~ 13
    security_types authmode;
    unsigned char ssid_hidden;	// Note: default 0
    unsigned char max_connection;	// Note: default 4, max 4
    unsigned short beacon_interval;	// Note: support 100 ~ 60000 ms, default 100
};




extern char wifi_station_get_config(struct station_config *config);
extern char wifi_station_set_config(struct station_config *config);

extern char wifi_station_connect (void);
//设置模块的工作模式
extern char wifi_set_opmode(uint8_t opmode);

//获取IP地址信息
//if_index:STATION 或者 SOFT_AP
extern char wifi_get_ip_info(uint8_t if_index,struct ip_info *info);
//设置IP地址信息
extern char wifi_set_ip_info(uint8_t if_index,struct ip_info *info);

//串口0输出
extern void uart0_sendStr(char *str);
extern void uart0_tx_buffer(const uint8_t *buf, uint16_t len);

/*
    if_index:STATION 或者 SOFT_AP
    macaddr: 6 bytes
*/
//获取MAC地址
extern char wifi_get_macaddr(uint8_t if_index,uint8_t *macaddr);
//设置MAC地址
extern char wifi_set_macaddr(uint8_t if_index,uint8_t *macaddr);


//打开ap的DHCP服务器功能
extern char wifi_softap_dhcps_start(void);
//关闭ap的DHCP服务器功能
extern char wifi_softap_dhcps_stop(void);

//打开STA的DHCP 自动获取IP功能
extern char wifi_station_dhcpc_start(void);
//关闭STA的DHCP 自动获取IP功能
extern char wifi_station_dhcpc_stop(void);

//装载几号引脚为WIFI状态显示
extern void wifi_status_led_install(uint8_t pin);

//开始airkiss配网
//注意：此函数只能在user_init()中使用。
extern int airkiss_start(void);
//停止airkiss配网
extern int airkiss_finish(void);

#endif

