#ifndef __DHCPSERVER_H__
#define __DHCPSERVER_H__

#define DHCP_CHADDR_LEN 16U
#define DHCP_SNAME_LEN  64U
#define DHCP_FILE_LEN   128U

#define DHCPS_MAX_LEASE 0x64
#define BOOTP_BROADCAST 0x8000
#define MAX_STATION_NUM  10

enum offer_option{
	OFFER_START = 0x00,
	OFFER_ROUTER = 0x01,
	OFFER_END
};

typedef enum {
	DHCPS_STATE_OFFER = 1,
	DHCPS_STATE_DECLINE,
	DHCPS_STATE_ACK,
	DHCPS_STATE_NAK,
	DHCPS_STATE_IDLE,
	DHCPS_STATE_RELEASE
}dhcps_state;

typedef struct dhcps_msg {
        u8_t op;
		u8_t htype;
		u8_t hlen;
		u8_t hops;
        u32_t xid;
        u16_t secs, flags;
        u8_t ciaddr[4];
        u8_t yiaddr[4];
        u8_t siaddr[4];
        u8_t giaddr[4];
        u8_t chaddr[16];
        u8_t sname[64];
        u8_t file[128];
		u32_t cookie;
        u8_t options[308];
}dhcps_msg;

struct lease_setting {
	struct ip_addr start_ip;
	struct ip_addr end_ip;
};

struct leased_info{
	struct ip_addr ip;
	u8_t mac[6];
	u32_t lease_timer;
};

typedef struct leased_list{
	void *node;
	struct leased_list *next;
}leased_list ;


void dhcpd_start(struct netif *apnetif);
void dhcpd_stop(struct netif *apnetif);

#endif

