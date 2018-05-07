/*=============================================================================+
|                                                                              |
| Copyright 2015                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*!
*   \file at_command_demo.c
*   \brief main entry
*   \author Montage
*/

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <stdint.h>
#include <common.h>
#include <flash_api.h>
#include "lwip/sockets.h"
#include <lynx_debug.h>
#include <gpio.h>
#include <serial.h>
#include <event.h>
#include <net_api.h>
#include <wla_api.h>
#include <gpio.h>
#include <cfg_api.h>
#include <user_config.h>
#include <FreeRTOS.h>
#include <task.h>
#include <net_api.h>
#include <minifs/mfs.h>
#include <webserver/webserver.h>

#define TEST_UART   0//Lynx UART1
#define uart_log(...)                       serial_printf(__VA_ARGS__)
#define UART_RECV_TIMEOUT                   0
#define UART_ONE_PACKAGE_LENGTH             1024
#define UART_BAUD_RATE                      115200

extern sdk_param g_atParam;
extern const php_funcationType php_fun[];
extern const php_funPostType post_php_fun[];
extern at_funType at_UserCmd[];


/*----------------------------------------------------------------*/
/**
 * The function is called once application start-up. Users can initial
 * structures or global parameters here.
 *
 * @param None.
 * @return int Protothread state.
 */
/*----------------------------------------------------------------*/
int app_main(void)
{
#ifdef CONFIG_WLA_LED
	pin_mode(WIFI_LED_PIN, 1);
	digital_write(WIFI_LED_PIN, 0);
#endif
	/* Do not add any process here, user should add process in user_thread */
	hw_sys_init();
	//serial_conf(8, 0, 1, 1,0 );
	return PT_EXITED;
}




void uart_init(void)
{
	int threshold = 0;	
	//g_atParam.ur_param.baudrate

//	serial_printf("baud:%d,par:%d,stop:%d\n",baudrate_select(g_atParam.ur_param.baudrate),g_atParam.ur_param.parity,g_atParam.ur_param.stopbits);
//	serial_conf(7, 0, 1, TEST_UART,threshold );
	serial_conf(baudrate_select(g_atParam.ur_param.baudrate), g_atParam.ur_param.parity, g_atParam.ur_param.stopbits, TEST_UART,threshold );
	serial_init((int)TEST_UART);

#ifdef CONFIG_UR_FLOW_CONTROL
	pin_mode(CONFIG_UR_CTS, 0);
	digital_write(CONFIG_UR_RTS, 0);//RTS active
	pin_en_intr(CONFIG_UR_CTS, 0, (gpio_func_cb)uart_flow_cts_stat, NULL);//0: rising
	pin_en_intr(CONFIG_UR_CTS, 1, (gpio_func_cb)uart_flow_cts_stat, NULL);//1: falling
	pin_mode(CONFIG_UR_RTS, 1);
#endif	
}



void user_init(void){
	uart_init();
	//at_test:用户也自定义at指令
	at_cmd_init(at_UserCmd);


	//mount mini fs
	if(mfmount(FS1_FLASH_ADDR) == RES_OK){
		//当连接AP时，可以通过下面的网页来打开内置网页
		dnsserver_init("set.mqlinks.com");
		webserver_init(php_fun,post_php_fun);
	}

	return;
}

