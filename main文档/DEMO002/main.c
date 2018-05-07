/*=============================================================================+
|                                                                              |
| Copyright 2015                                                             |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file app_init.c
*   \brief main entry
*   \author Montage
*/

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <c_types.h>
#include <stdint.h>
#include <event.h>
#include <common.h>
#include <os_api.h>
#include <net_api.h>
#include <wla_api.h>
#include <cfg_api.h>
#include <gpio.h>
#include <version.h>
#include <built_info.h>
#include <omniconfig.h>
//---------------begin--------------------
//lzw 引入此两个包为了处理字符串
#include <stdio.h>
#include <string.h>
#include <time.h>
//---------------end---------------------
#if defined(CONFIG_FREERTOS)
#include <FreeRTOS.h>
#include <task.h>
#endif
#if defined(CONFIG_LWIP)
#include <net_api.h>
#endif
#include <user_config.h>
#include <mylinks_wifi.h>
#include <iot_aliyun.h>
#include <smartconfig.h>
#include <cJSON/cJSON.h>
#include <mylinks_sntp.h>
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
//当为1时，使用MLINK进行连接
#define USER_MLINK 1
#define MAGIC_NUM_SAVE 0xa5a5a5a5
//当为1时，设备没有烧写数据时，进入烧写模式
//用户可以自行定义串口格式进行烧写后保存
#define PRODUCED	0

//自定义回复给APP包格式
#define USER_DEF_MLINK_INFO 1

/*----------------------------------------------------------------*/
/**
 * 此例中使用没有无SSL连接方式,请在查看代码前做以下工作:
 * 将include文件夹中的“#define CONFIG_AXTLS 1”注释
 * make lib，重新编译mqtt.a静态库
 * make mqtt进行例程编译，生成mqtt.img文件
 */
/*----------------------------------------------------------------*/

#if PRODUCED
struct _device_info_save{
	uint32_t magic_num;
	char product_key[16 + 1];
	char device_name[32 + 1];
	char device_secret[32 + 1];
};

struct _device_info_save g_device_info_save = {0};
char *topic_get = NULL;
char *topic_upgrade = NULL;
#endif

#define PRODUCT_VERSION	"1.0.1"
#define SSID ""
#define PWD	 ""

#define TESTSTR "Hello,IOT Aliyun"
//The product and device information from IOT console
#define PRODUCT_KEY         "hOrED1w5YeX"
#define DEVICE_NAME         "onlinetest02"
#define DEVICE_SECRET       "UfxkzQMT6tduShNTdXpOLlMWXzvMHOkq"
#define KEEPALIVE	180
//lzw 定义的产品设备的唯一id
#define EQUIPMENTID         "2c91e1c95fb42ec0015fb45058f60002"


//This is the pre-defined topic
#define TOPIC_UPDATE         "/"PRODUCT_KEY"/"DEVICE_NAME"/update"
#define TOPIC_ERROR          "/"PRODUCT_KEY"/"DEVICE_NAME"/update/error"
#define TOPIC_GET            "/"PRODUCT_KEY"/"DEVICE_NAME"/get"

#define IOTALIYUN_ADDR		 ""PRODUCT_KEY".iot-as-mqtt.cn-shanghai.aliyuncs.com"


static uint8_t mqttbuf[1024];
static uint8_t *pbuf = mqttbuf;
static 	int rc = FAILURE;
SemaphoreHandle_t mqtt_rx = NULL;
extern struct serial_buffer *ur0_rxbuf;//此处和老版本比增添 lzw
//------------------------------------------
//lzw 定义的常量
static char* mqttbackbuf;
static int mqttbacklen = 0;
static uint8_t ascii[3] ={0xff,0xff,0xff};
int init_state=0;
//------------------------------------------
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
	return PT_EXITED;
}

void uart_init(void)
{
	/*
	设置串口参数：
	参数1->波特率：115200
	参数2->校验位：无
	参数3->停止位：1位
	参数4->串口号：0号串口
	*/
	serial_conf(baudrate_select(115200), 0, 1, 0,0 );
	serial_init(0);

}
//---------------------begin------------------------------
// lzw 将str字符以spl分割,存于dst中，并返回子字符串数量
int split(char dst[][80], char* str, const char* spl)
{
    int n = 0;
    char *result = NULL;
    result = strtok(str, spl);
    while( result != NULL )
    {
        strcpy(dst[n++], result);
        result = strtok(NULL, spl);
    }
    return n;
}

//---------------------end------------------------------
//接收数据回调函数
void IOTAliyunRecvData(MessageData* md)
{
	MQTTMessage* message = md->message;
	MQTTString * topicName = md->topicName;
	int i;
#if PRODUCED
	if (strncmp(topicName->lenstring.data, topic_get, topicName->lenstring.len) == 0)
#else
	if (strncmp(topicName->lenstring.data, TOPIC_GET, topicName->lenstring.len) == 0)
#endif
	{
        char datastr[30] ;
        char cstr[256];

        char dst[10][80];
        int cnt = split(dst, message->payload, "#");
//        dst[6];
//                int i=0;
//                for (i = 0; i < cnt; i++)
//                    puts(dst[i]);
         char initeq[2]="0";
         char init_state_iot[4];
         sprintf(init_state_iot,"%s",dst[2]);

         if(strcmp(init_state_iot,initeq)==0)
         {
            init_state=1;
            uart0_sendStr("page page0");
            uart0_tx_buffer(ascii,3);
            //先切换屏幕界面给个延时
            sys_msleep(50);
         }


         sprintf(datastr,"datastr.txt=\"%s\"",dst[0]);
         uart0_sendStr(datastr);
         uart0_tx_buffer(ascii,3);

         sprintf(cstr,"cstr.txt=\"%s\"",dst[1]);
         uart0_sendStr(cstr);
         uart0_tx_buffer(ascii,3);

//		uart0_tx_buffer(message->payload,message->payloadlen);
//		uart0_tx_buffer("自定义修改",10);
//		uart0_sendStr(message);
	}
	return;
}

//串口接收回调函数
void test_uart0_rev( void * arg){
	uint8_t temp;

	for(;;){
		//等待串口是否有数据传入
		if(0 != uart_recv_sem_wait(portMAX_DELAY)){
			continue;
		}
//		del_timeout(UartTimeoutSendMqttSendData,0);
		//判断串口数据是否为空
		while(serial_buffer_empty(ur0_rxbuf)){
			//读取一个字节的串口数据
			temp = serial_buffer_getchar(ur0_rxbuf);
//			if((pbuf - mqttbuf) < sizeof(mqttbuf)){
//				*pbuf++ = temp;
//				if((pbuf - mqttbuf) == sizeof(mqttbuf)){
//					xSemaphoreGive(mqtt_rx);
//				}
//			}
		}
		//两个串口之间接收到的时间间隔为30ms时,则再时发送MQTT协议的数据
//		del_timeout(UartTimeoutSendMqttSendData,0);
//		add_timeout(UartTimeoutSendMqttSendData, 0, 30);
	}
}

//发布数据函数
int IOTAliyunSendData(void)
{

	MQTTMessage message;
	/* Send message */

	message.payloadlen = pbuf - mqttbuf;
	if(!message.payloadlen){
		return SUCCESS;
	}
	message.payload = mqttbuf;
	message.dup = 0;
	message.qos = QOS1;
	message.retained = 0;
#if PRODUCED
	return IOTAliyunPublish(topic_upgrade, &message);
#else
	return IOTAliyunPublish(TOPIC_UPDATE, &message);
#endif
}

void UartTimeoutSendMqttSendData(void *arg){
	xSemaphoreGive(mqtt_rx);
	return;
}


//挪到上面去了，进行了修改
////串口接收回调函数
//void test_uart0_rev( void * arg){
//	uint8_t temp;
//
//	for(;;){
//		//等待串口是否有数据传入
//		if(0 != uart_recv_sem_wait(portMAX_DELAY)){
//			continue;
//		}
////		del_timeout(UartTimeoutSendMqttSendData,0);
//		//判断串口数据是否为空
//		while(serial_buffer_empty(ur0_rxbuf)){
//			//读取一个字节的串口数据
//			temp = serial_buffer_getchar(ur0_rxbuf);
//			if((pbuf - mqttbuf) < sizeof(mqttbuf)){
//				*pbuf++ = temp;
//				if((pbuf - mqttbuf) == sizeof(mqttbuf)){
//					xSemaphoreGive(mqtt_rx);
//				}
//			}
//		}
//		//两个串口之间接收到的时间间隔为30ms时,则再时发送MQTT协议的数据
//		del_timeout(UartTimeoutSendMqttSendData,0);
//		add_timeout(UartTimeoutSendMqttSendData, 0, 30);
//	}
//}


#if PRODUCED
static void UartTimeoutSaveData(void *arg){
	//这里将处理的数据，拷贝到数据自定义的Device中去
    *pbuf = '\0';
    cJSON *Json,*s;
    Json = cJSON_Parse(mqttbuf);
    if(Json != NULL)
    {
        s = cJSON_GetObjectItem(Json, "key");
        if(s != NULL)
        {
        	strncpy(g_device_info_save.product_key,s->value.valuestring,16 + 1);
        }else{
            goto uart_err;
        }
        s = cJSON_GetObjectItem(Json, "name");

        if(s != NULL)
        {
            strncpy(g_device_info_save.device_name,s->value.valuestring,32 + 1);
        }else{
            goto uart_err;
        }
        s = cJSON_GetObjectItem(Json, "secret");

        if(s != NULL)
        {
            strncpy(g_device_info_save.device_secret,s->value.valuestring,32 + 1);
        }else{
            goto uart_err;
        }
        g_device_info_save.magic_num = MAGIC_NUM_SAVE;
    	flash_erase(CFG_FLASH_USER_START + 2*0x1000, CFG_FLASH_MEM_LENGTH);
    	flash_write(CFG_FLASH_USER_START + 2*0x1000, (unsigned int)&g_device_info_save, sizeof(struct _device_info_save));
//        uart0_sendStr("+ok\r\n");
uart_err:
        cJSON_Delete(Json);
    }
    pbuf = mqttbuf;
}
static void produced_uart0_rev( void * arg){
	uint8_t temp;

	for(;;){
		//等待串口是否有数据传入
		if(0 != uart_recv_sem_wait(portMAX_DELAY)){
			continue;
		}
		while(serial_buffer_empty(ur0_rxbuf)){
			temp = serial_buffer_getchar(ur0_rxbuf);
			if((pbuf - mqttbuf) < (sizeof(mqttbuf) -1)){
				*pbuf++ = temp;
                if((pbuf - mqttbuf) == (sizeof(mqttbuf) -1)){
                    UartTimeoutSaveData(NULL);
                }

			}
		}
        if(pbuf != mqttbuf)
        {
			del_timeout(UartTimeoutSaveData,0);
			add_timeout(UartTimeoutSaveData, 0, 30);
        }


	}
}
#endif

//MQTT处理任务
static void  iotaliunclient( void *arg )
{
#if PRODUCED
	char *iotaliyun_addr = malloc(64);
	sprintf(iotaliyun_addr,"%s.iot-as-mqtt.cn-shanghai.aliyuncs.com",g_device_info_save.product_key);
#endif
	for(;;){

		//等待STA获取IP地址
		if(get_slinkup() != STA_LINK_GET_IP){
			if(rc != FAILURE){
				rc = FAILURE;
				IOTAliyunDeinit();
			}
			goto ALIYUN_ERR;
		}

		if(rc == FAILURE){
			//开始阿里云物联网套件的连接
#if PRODUCED
			rc = IOTAliyunStart(iotaliyun_addr, 1883);
#else
			rc = IOTAliyunStart(IOTALIYUN_ADDR, 1883);
#endif
			//如果rc创建失败,则继续创建
			if(rc == FAILURE){
//				uart0_sendStr("IOTAliyun connect fail\r\n");
				IOTAliyunDeinit();
				goto ALIYUN_ERR;
			}else{
				//创建成功，则发布订阅号
#if PRODUCED
				rc = IOTAliyunSubscribe(topic_get,QOS1,IOTAliyunRecvData);
#else
				rc = IOTAliyunSubscribe(TOPIC_GET, QOS1, IOTAliyunRecvData);
#endif
				if(rc == FAILURE){
//					uart0_sendStr("IOTAliyun subscribe fail\r\n");
					IOTAliyunDeinit();
					goto ALIYUN_ERR;
				}else{
//					uart0_sendStr("IOTAliyun subscribe ok\r\n");
				}
			}
		}
//#if 1
//		//向Aliyun物联网套件发送数据
//		if(xSemaphoreTake(mqtt_rx, 0) == pdTRUE)
//		{
//			//发送一个MQTT的数据
//			if(IOTAliyunSendData() < 0){
//				//用户可自行处理，这里为断开后再次连接
//				IOTAliyunDeinit();
//				pbuf = mqttbuf;
//				goto ALIYUN_ERR;
//			}
//			pbuf = mqttbuf;
//		}
//#endif


		//Aliyun物联网套件 keepactive 重连接处理等处理，用户可以不用理会此处设置延时1000ms
		rc = IOTAliyunYield(10);

		if (rc == FAILURE){
//			uart0_sendStr("IOTAliyun yield fail\r\n");
			//用户可自行处理，这里为断开后再次连接
			IOTAliyunDeinit();
		}
		continue;
ALIYUN_ERR:
		if(rc != FAILURE){
			rc =  FAILURE;
			IOTAliyunDeinit();
		}
		//此任务循环时间为500ms
		sys_msleep(500);
	}
exit:
    vTaskDelete(NULL);
	return;
}



#if USER_MLINK
struct _ssid_save{
	uint32_t magic_num;
	char ssid[32 + 1];
	char pwd[64 + 1];
};


struct _ssid_save g_ssid_save;


#if USER_DEF_MLINK_INFO
static const uint8_t user_bssid[6] = {0x1,0x2,0x3,0x4,0x5,0x6};
static void user_pkt_init(void)
{
	uint8_t user_buffer[11];
	mlink_broadcast_init(user_buffer);
	memcpy(user_buffer + 1,user_bssid,sizeof(user_bssid));
	mlink_broadcast_copy(user_buffer,sizeof(user_buffer));
	return;
}

#endif

void mlink_callback_t(sc_status status, void *pdata)
{
	struct _ssid_pwd *s = (struct _ssid_pwd *)pdata;
	switch(status) {
		case SC_STATUS_WAIT:
//		uart0_sendStr("SC_STATUS_WAIT\r\n");
		break;
		case SC_STATUS_FIND_CHANNEL:
//		uart0_sendStr("SC_STATUS_FIND_CHANNEL\r\n");
		break;
		case SC_STATUS_GETTING_SSID_PSWD:
//		uart0_sendStr("SC_STATUS_GETTING_SSID_PSWD\r\n");
		break;
		case SC_STATUS_LINK:
//		uart0_sendStr("SC_STATUS_LINK\r\n");

//		uart0_sendStr("ssid:");
//		uart0_sendStr(s->ssid);
//		uart0_sendStr(",pwd:");
//		uart0_sendStr(s->pwd);
//		uart0_sendStr("\r\n");
		g_ssid_save.magic_num = MAGIC_NUM_SAVE;
		memcpy(g_ssid_save.ssid,s->ssid,strlen(s->ssid)+1);
		memcpy(g_ssid_save.pwd,s->pwd,strlen(s->pwd)+1);
		struct station_config new;
		wifi_station_get_config(&new);
		strcpy(new.ssid,s->ssid);
		//设置连接的路由器密码
		strcpy(new.password,s->pwd);
		wifi_station_set_config(&new);
		flash_erase(CFG_FLASH_USER_START + 1*0x1000, CFG_FLASH_MEM_LENGTH);
		flash_write(CFG_FLASH_USER_START + 1*0x1000, (unsigned int)&g_ssid_save, sizeof(struct _ssid_save));
#if USER_DEF_MLINK_INFO
		user_pkt_init();
#endif
		smartconfig_connect();
		smartconfig_stop();
		break;
	}
}

#define GIPO_ZCFG_RESET 6


static void buttons_monitor(void* data)
{
	char db[32];
	//----lzw-------------
	//控制二维码显示与关闭的变量
	uint32_t m = 200;
    uint32_t n = 0;
    //----------------------
    for(;;){
    //二维码显示20秒后关闭
        if(m<(10*20))
        {
            ++m;
            if(m==197)
            {
                init_state=0;
            }
        }
        if(digital_read(GIPO_ZCFG_RESET) == LOW)
        {
            ++n;
            if(n == 10*5){
            	wlan_led_uninstall();
            	pin_mode(WIFI_LED_PIN, 1);
				digital_write(WIFI_LED_PIN, 1);
            }
        }else{
            if(n > (10*5))
            {
                flash_erase(CFG_FLASH_USER_START + 1*0x1000, CFG_FLASH_MEM_LENGTH);
                reboot(0);
            }
            //按1秒 以上 5秒以下为显示二维码
            if(n > (10*1))
            {
                uart0_sendStr("page page1");
                uart0_tx_buffer(ascii,3);
                m=0;
            }
            n = 0;
        }
        sys_msleep(100);
    }
    vTaskDelete(NULL);
}

void key_init(void)
{
    pin_mode(GIPO_ZCFG_RESET,GPIO_IN);
    xTaskCreate(buttons_monitor, (const signed char*)"bts", (unsigned short) 512, NULL, 5, NULL );
    return;
}
#endif



uint8_t tbuf[1024];
static void  sntp_demo( void *arg )
{
	struct tm * t;
    char t7[20];
	//等待WIFI模块连接路由器成功
	while(get_slinkup() != STA_LINK_GET_IP){
		sys_msleep(50);
	}
	//阿里云的time.pool.aliyun.com   中国共用的 cn.pool.ntp.org
	while(sntp_init("time.pool.aliyun.com") != 0){
		sys_msleep(100);
	}
//	uart0_sendStr("get sntp\r\n");

	for(;;){
		//获取东八区
		t = get_local_time(8);
//		sprintf(tbuf,"%d-%d-%d %d:%d:%d\r\n",t->tm_year + 1900,t->tm_mon + 1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
//        sprintf(tbuf,"t7.txt=\"%d-%d-%d %d:%d:%d\"",t->tm_year + 1900,t->tm_mon + 1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
        sprintf(tbuf,"times.txt=\"%d:%d:%d\"",t->tm_hour,t->tm_min,t->tm_sec);
		uart0_sendStr(tbuf);
		uart0_tx_buffer(ascii,3);
		sys_msleep(1000);
	}
exit:
    vTaskDelete(NULL);
	return;
}

//通电初始化向阿里云发送请求
static void  init_aliyun_message(void *arg)
{
//todo 初始化逻辑在这写
	//等待WIFI模块连接路由器成功
	while(get_slinkup() != STA_LINK_GET_IP){
		sys_msleep(50);
	}
	uint32_t k = 0;
	for(;;)
	{
	++k;
        if(rc != FAILURE)
        {
           if(init_state == 0)
           {
                if(k>10*3)
                {
                    mqttbackbuf =EQUIPMENTID;
                    mqttbacklen = strlen(mqttbackbuf);

                    MQTTMessage message;
                    message.payloadlen = mqttbacklen;
                    message.payload = mqttbackbuf;
                    message.dup = 0;
                    message.qos = QOS1;
                    message.retained = 0;
                    IOTAliyunPublish(TOPIC_UPDATE, &message);
                    k=0;
                }
           }else{
                if(k>10*120)
                {
                    mqttbackbuf =EQUIPMENTID;
                    mqttbacklen = strlen(mqttbackbuf);

                    MQTTMessage message;
                    message.payloadlen = mqttbacklen;
                    message.payload = mqttbackbuf;
                    message.dup = 0;
                    message.qos = QOS1;
                    message.retained = 0;
                    IOTAliyunPublish(TOPIC_UPDATE, &message);
                    k=0;
                }
           }
    	}
//    	if(rc==FAILURE){
//    	    //todo 考虑断网后 是否把初始化状态 重置 缺点：重置后可能由于用户网络不稳定多次访问平台，造成大量请求。优点：用户网络恢复就可从新获得时效数据。但需要服务端控制好策略，防止由于网络不稳定的大量连接。
//    	sys_msleep(2000);
//    	}
         sys_msleep(100);
    }
exit:
    vTaskDelete(NULL);
	return;
}


void user_init(void){
	struct station_config s;
	uart_init();
	//注册一个串口0的接收任务进行数据接收

	//设置模块为STA工作模式
	wifi_set_opmode(OPMODE_STA);
//	uart0_sendStr("start system\r\n");
#if PRODUCED
	flash_read(CFG_FLASH_USER_START + 2*0x1000, (unsigned int)&g_device_info_save, sizeof(struct _device_info_save));
	if(g_device_info_save.magic_num != MAGIC_NUM_SAVE)
	{
		uart0_rev_register(produced_uart0_rev);
		return;
	}
#endif
	uart0_rev_register(test_uart0_rev);
#if USER_MLINK
	key_init();
	flash_read(CFG_FLASH_USER_START + 1*0x1000, (unsigned int)&g_ssid_save, sizeof(struct _ssid_save));
	if(g_ssid_save.magic_num != MAGIC_NUM_SAVE)
	{
//		uart0_sendStr("start mlink\r\n");
		mlink_start(mlink_callback_t);
	}
#else
	//读取当前模块的STA配置信息
	wifi_station_get_config(&s);
	if(strcmp(s.ssid,SSID) ||
		strcmp(s.password,PWD)){

		memset(&s,0,sizeof(s));
		//设置连接的路由器ssid
		strcpy(s.ssid,SSID);
		//设置连接的路由器密码
		strcpy(s.password,PWD);
		wifi_station_set_config(&s);
	}
#endif
	mqtt_rx = xSemaphoreCreateBinary();
	xSemaphoreTake(mqtt_rx, (TickType_t)10);
	xTaskCreate(sntp_demo, "sntp_demo", TASK_HEAP_LEN, 0, 5, NULL);
	xTaskCreate(init_aliyun_message, "init_aliyun_message", TASK_HEAP_LEN, 0, 5, NULL);
#if PRODUCED
	topic_get = malloc(128);
	topic_upgrade = malloc(128);
	sprintf(topic_get,"%s/%s/get",g_device_info_save.product_key,g_device_info_save.device_name);
	sprintf(topic_upgrade,"%s/%s/update",g_device_info_save.product_key,g_device_info_save.device_name);	
	IOTAliyunDataInit(g_device_info_save.product_key,g_device_info_save.device_name,g_device_info_save.device_secret,KEEPALIVE);
	xTaskCreate(iotaliunclient, "iotaliyun", TASK_HEAP_LEN * 2, 0, 5, NULL);
#else
	IOTAliyunSetVersion(PRODUCT_VERSION);
	IOTAliyunDataInit(PRODUCT_KEY,DEVICE_NAME,DEVICE_SECRET,KEEPALIVE);
	xTaskCreate(iotaliunclient, "iotaliyun", TASK_HEAP_LEN * 2, 0, 5, NULL);
#endif
	return;
}



