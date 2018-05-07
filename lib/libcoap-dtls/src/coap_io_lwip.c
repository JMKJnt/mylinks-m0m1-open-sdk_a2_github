/* coap_io_lwip.h -- Network I/O functions for libcoap on lwIP
 *
 * Copyright (C) 2012,2014 Olaf Bergmann <bergmann@tzi.org>
 *               2014 chrysn <chrysn@fsfe.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use. 
 */

#include "coap_config.h"
#include "mem.h"
#include "coap_io.h"
#include <lwip/sys.h>
#include <lwip/udp.h>

#if defined(CONFIG_FREERTOS)
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#endif

static sys_mbox_t mbox;

void coap_packet_populate_endpoint(coap_packet_t *packet, coap_endpoint_t *target)
{
	printf("FIXME no endpoint populated\n");
}
void coap_packet_copy_source(coap_packet_t *packet, coap_address_t *target)
{
	target->port = packet->srcport;
	memcpy(&target->addr, ip_current_src_addr(), sizeof(ip_addr_t));
}
void coap_packet_get_memmapped(coap_packet_t *packet, unsigned char **address, size_t *length)
{
	LWIP_ASSERT("Can only deal with contiguous PBUFs to read the initial details", packet->pbuf->tot_len == packet->pbuf->len);
	*address = packet->pbuf->payload;
	*length = packet->pbuf->tot_len;
}
void coap_free_packet(coap_packet_t *packet)
{
	if (packet->pbuf)
		pbuf_free(packet->pbuf);
	coap_free_type(COAP_PACKET, packet);
}

struct pbuf *coap_packet_extract_pbuf(coap_packet_t *packet)
{
	struct pbuf *ret = packet->pbuf;
	packet->pbuf = NULL;
	return ret;
}


/** Callback from lwIP when a package was received.
 *
 * The current implemntation deals this to coap_handle_message immedately, but
 * other mechanisms (as storing the package in a queue and later fetching it
 * when coap_read is called) can be envisioned.
 *
 * It handles everything coap_read does on other implementations.
 */
struct coap_pkt {
	void *payload;
	size_t payload_len;
	coap_endpoint_t *ep;
	coap_address_t remote;
}coap_pkt;

static void coap_process(void *data)
{
	struct coap_pkt *coap_pkt;
	
	while(1) {
    	sys_timeouts_mbox_fetch(&mbox, (void **)&coap_pkt);
    	
		if (is_dtls(coap_pkt->ep)) {
				coap_dtls_handle_message(coap_pkt->ep->context, coap_pkt->ep, &coap_pkt->remote, coap_pkt->payload, coap_pkt->payload_len);
      		} else {
				coap_handle_message(coap_pkt->ep->context, coap_pkt->ep, &coap_pkt->remote, coap_pkt->payload, coap_pkt->payload_len);
			}

			free(coap_pkt->payload);
			free(coap_pkt);	
	}
	vTaskDelete(NULL);
}
static void coap_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	struct coap_pkt *coap_pkt = (struct coap_pkt*)malloc(sizeof(struct coap_pkt));
	void *payload_dup = malloc(p->tot_len);

	memcpy(payload_dup, p->payload, p->tot_len);

	coap_pkt->payload = payload_dup;	
	coap_pkt->payload_len = p->tot_len;
	coap_pkt->ep = (coap_endpoint_t*)arg;
	coap_pkt->remote.port = port;
	memcpy(&coap_pkt->remote.addr, ip_current_src_addr(), sizeof(ip_addr_t));
	
	if (!sys_mbox_valid(&mbox)) {
		serial_printf("%s mbox invalid\n",__func__);
  	}

 	sys_mbox_post(&mbox, coap_pkt);

	pbuf_free(p);
}

coap_endpoint_t *coap_new_endpoint(const coap_address_t *addr, int flags) {
	coap_endpoint_t *result;
	err_t err;

	/*LWIP_ASSERT("Flags not supported for LWIP endpoints", flags == COAP_ENDPOINT_NOSEC);*/

	result = coap_malloc_type(COAP_ENDPOINT, sizeof(coap_endpoint_t));
	if (!result) return NULL;
	
	result->context = NULL;
#if defined(MONT_LWIP)
	result->pcb = udp_new();
	result->ifindex = (int)result->pcb;
    if(flags == COAP_ENDPOINT_DTLS)
		result->flags = flags;
#else
	result->pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
#endif
	if (result->pcb == NULL) goto error;

	udp_recv(result->pcb, (udp_recv_fn)coap_recv, (void*)result);
	err = udp_bind(result->pcb, (ip_addr_t *)&addr->addr, addr->port);
	if (err) {
		udp_remove(result->pcb);
		goto error;
	}

    if(sys_mbox_new(&mbox, 4) != 0) {
        serial_printf("failed to create coap thread mbox\n"); 
    }

	xTaskCreate(coap_process, (const signed char*)"coap_process", (unsigned short) 4096, NULL, 5, NULL );


	return result;

error:
	coap_free_type(COAP_ENDPOINT, result);
	return NULL;
}

void coap_free_endpoint(coap_endpoint_t *ep)
{
	udp_remove(ep->pcb);
	coap_free_type(COAP_ENDPOINT, ep);
}
