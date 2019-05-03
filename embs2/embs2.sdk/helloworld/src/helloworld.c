#include <stdio.h>
#include "xparameters.h"
#include "platform.h"
#include "xil_printf.h"
#include "xil_cache.h"
#include <stdlib.h>
#include "xtoplevel.h"

#define MAX_SOLUTION 10

// Control varaibles to get out of handling loop
int received_puzzle = 0;

char input[10*10*4] = {11};
char fake_input[10*10*4] = {1,7,7,3,   1,2,3,4,   3,3,3,3,    1,1,1,2};
char three[10*10*4] = {1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4};

//char fake_input[10*10*4] = {1,7,7,3,   1,2,3,4,   3,3,3,3,    1,1,1,2,   3,3,1,1,   3,3,1,7,  3,3,3,3,   7,3,3,3,  1,3,3,3};
char output[10*10*4*MAX_SOLUTION] = {50};

char* run_solver(int size){
	XToplevel hls;
	Xil_DCacheDisable(); // dumb way, you should manually invalidate and flush
	XToplevel_Initialize(&hls, XPAR_TOPLEVEL_0_DEVICE_ID);
	XToplevel_Set_input_r(&hls, (int) input);
	XToplevel_Set_output_r(&hls, (int) output);
	XToplevel_Set_size(&hls, (int) size);

	printf("Solver started!\n");
	XToplevel_Start(&hls);

	while(!XToplevel_IsDone(&hls));

	char *hope = XToplevel_Get_output_r(&hls);

	cleanup_platform();
	return hope;
}

void udp_get_handler(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
	// Gets puzzle and puts it into input
    if(p) {
        char msg[p->len + 1];
        memcpy(msg, p->payload, p->len);
        msg[p->len] = '\0';

        int size = msg[1];
        for(int i = 0; i < size*size*4; i++){
        	input[i] = msg[i+6];
        }
        printf("I got a puzzle of size: %d\n", size);

        received_puzzle = 1;

        pbuf_free(p);
    }
}

int main() {
	unsigned char mac_ethernet_address[] = {0x00, 0x11, 0x22, 0x33, 0x00, 0x51}; // Put your own MAC address here!
    init_platform(mac_ethernet_address, NULL, NULL);


    struct udp_pcb *recv_pcb = udp_new();
    if(!recv_pcb) {
        printf("Error creating PCB\n");
    }

    udp_bind(recv_pcb, IP_ADDR_ANY, 51050);
    udp_recv(recv_pcb, udp_get_handler, NULL);

    struct udp_pcb *send_pcb = udp_new();
    unsigned char mex[6] = {'\x01', '\x03', '\x00', '\x00', '\x00', '\x00'};
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
        if(received_puzzle){
        	break;
        }
    }

    char *hope = run_solver(3);
    printf("Solver has finished");
    print_puzzle(hope, 3);

    return 0;
}
