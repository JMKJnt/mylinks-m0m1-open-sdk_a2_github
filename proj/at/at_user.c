/*=============================================================================+
|                                                                              |
| Copyright 2017                                                               |
| Mylinks Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*!
*   \file at_command_demo.c
*   \brief main entry
*   \author Mylinks
*/

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <stdint.h>
#include <common.h>
#include <cfg_api.h>
#include <user_config.h>
#include <otp.h>
#include "../../lib/atcmd/at.h"
#include <mylinks_sntp.h>

extern at_funType at_UserCmd[];

struct rfc_cal_reg{
	unsigned char balancer_nm;				//bb20;
	unsigned char tx_a21; 					//bb23;
	unsigned char tx_a22; 					//bb24;
	unsigned char tx_dc_i; 					//bb25;
	unsigned char rx_a21;  					//bb28;
	unsigned char rx_a22; 					//bb29;
	unsigned char tx_dc_q; 					//bb30;
	unsigned char filter_switch; 			//bb5a;
	unsigned char phaseshifter_rx_alfa;		//bb5c;
	unsigned char phaseshifter_tx_alfa;		//bb5e;
	unsigned char a21a22_ext;

	unsigned char is_failed;
	unsigned char tx_a11;  					//bb21;
	unsigned char tx_a12; 					//bb22;
	unsigned char rx_dc_i; 					//bb2a;
	unsigned char rx_dc_q; 					//bb31;
	unsigned char rolloff_rx_coe; 			//bb5b;
	unsigned char rolloff_tx_coe;			//bb5d;
};


#define RFC2_LEN 11
extern struct rfc_cal_reg rfc_result_lynx[2];

static void StopRFAuto(void){
	int otp_ret;
	unsigned char rfc_otp[RFC2_LEN];
	if(OTP_MEM_SIZE != (otp_ret = otp_load(OTP_MEM_SIZE)))
    {
    	goto end;
    }
    otp_ret = otp_read((u8 *)&rfc_otp, RFC2, RFC2_LEN);
    if(otp_ret != RFC2_LEN)//avoiding re-writing otp data
    {
        if(!(otp_ret = otp_write((u8 *)&rfc_result_lynx[0], RFC2, RFC2_LEN)))
        {
            goto end;
        }
        if(otp_ret == OTP_MEM_SIZE)//OTP write not enough memory
        {
        	goto end;
        }
        if(!otp_submit())
        {
            goto end;
        }
        uart0_sendStr("Stop RF auto\r\n");
        //reboot(0);
        return;

    }
end:
	otp_end();
	return;
}

void at_exeCmdCupdate(uint8_t id){
	char *filename = NULL;
	char type;
	char *buffer = NULL;
	if(STA_LINK_GET_IP != get_slinkup()){
		at_backErrHead;
		uart0_sendStr("-5");
		at_backTail;
		return;		
	}
	buffer = (char *)malloc(128);
	if(NULL == buffer){
		goto UPERR;
	}
	if(!memcmp(at_UserCmd[id].at_cmdName,"UPGRADE",7)){
		filename = "user.img";
		type = UPDATE_SOFT_TYPE;
	}else{
		filename = "minifs_rom.img";
		type = UPDATE_WEB_TYPE;
	}
#if (PLATFORM==M0M100D0)
	sprintf(buffer,"118.178.87.170/products/M0M100x/upgrade/AT/Mylinks/001/%s",filename);
#else
	sprintf(buffer,"118.178.87.170/products/M0M100x/upgrade/AT/Mylinks/000/%s",filename);
#endif
	if(!Firmware_WIFIOTAByUrl(type,buffer,80)){
		free(buffer);
		if(type == UPDATE_SOFT_TYPE){
			StopRFAuto();
			reboot(1);
		}
		at_backOk;
		return;
	}
	free(buffer);
UPERR:
	at_backErrHead;
	uart0_sendStr("-4");
	at_backTail;
	return;	
}

void at_setupCmdCupdate(uint8_t id,char *pPara){
	char type;
	
	if(STA_LINK_GET_IP != get_slinkup()){
		at_backErrHead;
		uart0_sendStr("-5");
		at_backTail;
		return;		
	}
	int i;
	for(i = 0;i < 128;i++)
	{
		if(pPara[i] == '\r' || pPara[i] == '\n')
		{
			pPara[i] = '\0';
			break;
		}
	}
	if(!memcmp(at_UserCmd[id].at_cmdName,"UPGRADE",7)){
		type = UPDATE_SOFT_TYPE;
	}else{
		type = UPDATE_WEB_TYPE;
	}
	if(!Firmware_WIFIOTAByUrl(type,pPara,80)){
		if(type == UPDATE_SOFT_TYPE){
			StopRFAuto();
			reboot(1);
		}
		at_backOk;
		return;
	}
	at_backErrHead;
	uart0_sendStr("-4");
	at_backTail;
	return;	
}

void at_exeCmdSmtlkver(uint8_t id)
{
  display_atcmd_info(smartconfig_get_version());
}


void at_exeCmdntime(uint8_t id){
  extern uint32_t utctime_g;
  char temp[64];
  struct tm *t;
  char wday[7][5] = {"Sun","Mon","Tues","Wed","Thur","Fri","Sat"};
  if(0 == utctime_g && sntp_init("time.pool.aliyun.com") < 0){
    uart0_sendStr("Not Available");
    return;
  }
  t = get_local_time(8);

  sprintf(temp,"%d-%d-%d %d:%d:%d %s",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec,wday[t->tm_wday]);
  uart0_sendStr(temp);

  return;
}

at_funType at_UserCmd[]={
	{"NTPTM", 5, NULL, at_exeCmdntime},//查询时间
	{"UPGRADE",7,at_setupCmdCupdate,at_exeCmdCupdate},
	{"WUPDATE",7,NULL,at_exeCmdCupdate},
	{"SMTLKVER", 8, NULL, at_exeCmdSmtlkver},//设置/查询Smartlink版本
	//自定义的AT指令，必须以以下方式结尾
	{NULL, 0, NULL, NULL},
};



