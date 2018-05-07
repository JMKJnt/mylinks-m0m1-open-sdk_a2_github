#ifndef __AT_H
#define __AT_H

#define LVERSION "015 (2017-12-02 11:36 03F)"
#define LVER "015"
#define FWSZE   "141440,V15"

typedef enum{
	at_statIdle,
	at_statRecving,
	at_statSetReving,
	at_statProcess,
	at_statIpSended,
	at_statUserSaved,
	at_statModeSaved,
	mq_statIpSending,
	mq_statIpSended,
	at_eLinkProcess,
	at_TranOut,
	at_MQTTTranOut,
	at_udpProcess,
	at_statHttpTraning,
	at_statUrlSending,
	at_statUrlSended,
	at_statIpSending,
	at_statUserSaveing,
	at_statModeSaveing,
	at_statIpTraning,
	at_statIpTranout,
	fw_statIpTraning,
	at_noneTraning,
}at_stateType;


typedef struct
{
	char *at_cmdName;
	signed char at_cmdLen;
	void (*at_setupCmd)(unsigned char id, char *pPara);
	void (*at_exeCmd)(unsigned char id);
}at_funcationType;


#endif
