#include <stdio.h>
#include "xparameters.h"
#include "platform.h"
#include "xil_printf.h"
#include "xil_cache.h"

void udp_get_handler(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    if(p) { //Must check that a valid protocol control block was received
        //The message may not be zero terminated, so to ensure that we only
        //print what was sent, we can create a zero-terminated copy and print that
        char msg[p->len + 1];
        memcpy(msg, p->payload, p->len);
        msg[p->len] = '\0';

        printf("Sent Length %d\n", msg[1]);
        printf("Should be 2: %d\n", msg[0]);

        //Send a reply to the address which messaged us on port 7000
        //udp_sendto(pcb, p, addr, 7000);

        //Don't forget to free the protocol control block
        pbuf_free(p);
    }
}

int main_old() {
    unsigned char mac_ethernet_address[] = {0x00, 0x11, 0x22, 0x33, 0x00, 0x51}; // Put your own MAC address here!
    init_platform(mac_ethernet_address, NULL, NULL);



    struct udp_pcb *recv_pcb = udp_new();
    if(!recv_pcb) {
        printf("Error creating PCB\n");
    }

    udp_bind(recv_pcb, IP_ADDR_ANY, 51050);
    udp_recv(recv_pcb, udp_get_handler, NULL);

    struct udp_pcb *send_pcb = udp_new();
    unsigned char mex[6] = {'\x01', '\x05', '\x01', '\x01', '\x01', '\x01'};
    struct pbuf * reply = pbuf_alloc(PBUF_TRANSPORT, strlen(mex), PBUF_REF);
    reply->payload = mex;
    reply->len = strlen(mex);

    ip_addr_t ip;
    IP4_ADDR(&ip, 192, 168, 10, 1);
    udp_sendto(send_pcb, reply, &ip, 51050);
    pbuf_free(reply);
    udp_remove(send_pcb);

    while(1) {
        handle_ethernet();
    }
    return 0;
}
