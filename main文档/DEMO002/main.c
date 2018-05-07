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
//lzw �����������Ϊ�˴����ַ���
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
//��Ϊ1ʱ��ʹ��MLINK��������
#define USER_MLINK 1
#define MAGIC_NUM_SAVE 0xa5a5a5a5
//��Ϊ1ʱ���豸û����д����ʱ��������дģʽ
//�û��������ж��崮�ڸ�ʽ������д�󱣴�
#define PRODUCED	0

//�Զ���ظ���APP����ʽ
#define USER_DEF_MLINK_INFO 1

/*----------------------------------------------------------------*/
/**
 * ������ʹ��û����SSL���ӷ�ʽ,���ڲ鿴����ǰ�����¹���:
 * ��include�ļ����еġ�#define CONFIG_AXTLS 1��ע��
 * make lib�����±���mqtt.a��̬��
 * make mqtt�������̱��룬����mqtt.img�ļ�
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
//lzw ����Ĳ�Ʒ�豸��Ψһid
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
extern struct serial_buffer *ur0_rxbuf;//�˴����ϰ汾������ lzw
//------------------------------------------
//lzw ����ĳ���
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
	���ô��ڲ�����
	����1->�����ʣ�115200
	����2->У��λ����
	����3->ֹͣλ��1λ
	����4->���ںţ�0�Ŵ���
	*/
	serial_conf(baudrate_select(115200), 0, 1, 0,0 );
	serial_init(0);

}
//---------------------begin------------------------------
// lzw ��str�ַ���spl�ָ�,����dst�У����������ַ�������
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
//�������ݻص�����
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
            //���л���Ļ���������ʱ
            sys_msleep(50);
         }


         sprintf(datastr,"datastr.txt=\"%s\"",dst[0]);
         uart0_sendStr(datastr);
         uart0_tx_buffer(ascii,3);

         sprintf(cstr,"cstr.txt=\"%s\"",dst[1]);
         uart0_sendStr(cstr);
         uart0_tx_buffer(ascii,3);

//		uart0_tx_buffer(message->payload,message->payloadlen);
//		uart0_tx_buffer("�Զ����޸�",10);
//		uart0_sendStr(message);
	}
	return;
}

//���ڽ��ջص�����
void test_uart0_rev( void * arg){
	uint8_t temp;

	for(;;){
		//�ȴ������Ƿ������ݴ���
		if(0 != uart_recv_sem_wait(portMAX_DELAY)){
			continue;
		}
//		del_timeout(UartTimeoutSendMqttSendData,0);
		//�жϴ��������Ƿ�Ϊ��
		while(serial_buffer_empty(ur0_rxbuf)){
			//��ȡһ���ֽڵĴ�������
			temp = serial_buffer_getchar(ur0_rxbuf);
//			if((pbuf - mqttbuf) < sizeof(mqttbuf)){
//				*pbuf++ = temp;
//				if((pbuf - mqttbuf) == sizeof(mqttbuf)){
//					xSemaphoreGive(mqtt_rx);
//				}
//			}
		}
		//��������֮����յ���ʱ����Ϊ30msʱ,����ʱ����MQTTЭ�������
//		del_timeout(UartTimeoutSendMqttSendData,0);
//		add_timeout(UartTimeoutSendMqttSendData, 0, 30);
	}
}

//�������ݺ���
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


//Ų������ȥ�ˣ��������޸�
////���ڽ��ջص�����
//void test_uart0_rev( void * arg){
//	uint8_t temp;
//
//	for(;;){
//		//�ȴ������Ƿ������ݴ���
//		if(0 != uart_recv_sem_wait(portMAX_DELAY)){
//			continue;
//		}
////		del_timeout(UartTimeoutSendMqttSendData,0);
//		//�жϴ��������Ƿ�Ϊ��
//		while(serial_buffer_empty(ur0_rxbuf)){
//			//��ȡһ���ֽڵĴ�������
//			temp = serial_buffer_getchar(ur0_rxbuf);
//			if((pbuf - mqttbuf) < sizeof(mqttbuf)){
//				*pbuf++ = temp;
//				if((pbuf - mqttbuf) == sizeof(mqttbuf)){
//					xSemaphoreGive(mqtt_rx);
//				}
//			}
//		}
//		//��������֮����յ���ʱ����Ϊ30msʱ,����ʱ����MQTTЭ�������
//		del_timeout(UartTimeoutSendMqttSendData,0);
//		add_timeout(UartTimeoutSendMqttSendData, 0, 30);
//	}
//}


#if PRODUCED
static void UartTimeoutSaveData(void *arg){
	//���ｫ��������ݣ������������Զ����Device��ȥ
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
		//�ȴ������Ƿ������ݴ���
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

//MQTT��������
static void  iotaliunclient( void *arg )
{
#if PRODUCED
	char *iotaliyun_addr = malloc(64);
	sprintf(iotaliyun_addr,"%s.iot-as-mqtt.cn-shanghai.aliyuncs.com",g_device_info_save.product_key);
#endif
	for(;;){

		//�ȴ�STA��ȡIP��ַ
		if(get_slinkup() != STA_LINK_GET_IP){
			if(rc != FAILURE){
				rc = FAILURE;
				IOTAliyunDeinit();
			}
			goto ALIYUN_ERR;
		}

		if(rc == FAILURE){
			//��ʼ�������������׼�������
#if PRODUCED
			rc = IOTAliyunStart(iotaliyun_addr, 1883);
#else
			rc = IOTAliyunStart(IOTALIYUN_ADDR, 1883);
#endif
			//���rc����ʧ��,���������
			if(rc == FAILURE){
//				uart0_sendStr("IOTAliyun connect fail\r\n");
				IOTAliyunDeinit();
				goto ALIYUN_ERR;
			}else{
				//�����ɹ����򷢲����ĺ�
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
//		//��Aliyun�������׼���������
//		if(xSemaphoreTake(mqtt_rx, 0) == pdTRUE)
//		{
//			//����һ��MQTT������
//			if(IOTAliyunSendData() < 0){
//				//�û������д�������Ϊ�Ͽ����ٴ�����
//				IOTAliyunDeinit();
//				pbuf = mqttbuf;
//				goto ALIYUN_ERR;
//			}
//			pbuf = mqttbuf;
//		}
//#endif


		//Aliyun�������׼� keepactive �����Ӵ���ȴ����û����Բ������˴�������ʱ1000ms
		rc = IOTAliyunYield(10);

		if (rc == FAILURE){
//			uart0_sendStr("IOTAliyun yield fail\r\n");
			//�û������д�������Ϊ�Ͽ����ٴ�����
			IOTAliyunDeinit();
		}
		continue;
ALIYUN_ERR:
		if(rc != FAILURE){
			rc =  FAILURE;
			IOTAliyunDeinit();
		}
		//������ѭ��ʱ��Ϊ500ms
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
		//�������ӵ�·��������
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
	//���ƶ�ά����ʾ��رյı���
	uint32_t m = 200;
    uint32_t n = 0;
    //----------------------
    for(;;){
    //��ά����ʾ20���ر�
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
            //��1�� ���� 5������Ϊ��ʾ��ά��
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
	//�ȴ�WIFIģ������·�����ɹ�
	while(get_slinkup() != STA_LINK_GET_IP){
		sys_msleep(50);
	}
	//�����Ƶ�time.pool.aliyun.com   �й����õ� cn.pool.ntp.org
	while(sntp_init("time.pool.aliyun.com") != 0){
		sys_msleep(100);
	}
//	uart0_sendStr("get sntp\r\n");

	for(;;){
		//��ȡ������
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

//ͨ���ʼ�������Ʒ�������
static void  init_aliyun_message(void *arg)
{
//todo ��ʼ���߼�����д
	//�ȴ�WIFIģ������·�����ɹ�
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
//    	    //todo ���Ƕ����� �Ƿ�ѳ�ʼ��״̬ ���� ȱ�㣺���ú���������û����粻�ȶ���η���ƽ̨����ɴ��������ŵ㣺�û�����ָ��Ϳɴ��»��ʱЧ���ݡ�����Ҫ����˿��ƺò��ԣ���ֹ�������粻�ȶ��Ĵ������ӡ�
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
	//ע��һ������0�Ľ�������������ݽ���

	//����ģ��ΪSTA����ģʽ
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
	//��ȡ��ǰģ���STA������Ϣ
	wifi_station_get_config(&s);
	if(strcmp(s.ssid,SSID) ||
		strcmp(s.password,PWD)){

		memset(&s,0,sizeof(s));
		//�������ӵ�·����ssid
		strcpy(s.ssid,SSID);
		//�������ӵ�·��������
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



