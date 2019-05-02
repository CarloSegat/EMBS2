#include <stdio.h>
#include "xparameters.h"
#include "platform.h"
#include "xil_printf.h"
#include "xil_cache.h"
#include <stdlib.h>

int is_solved(char puzzle[], int size) {
	return puzzle[size - 1] != 0;
}

int can_fit(char piece[], char puzzle[], int solved_index, int size){
	// On the 1st row
	if (solved_index < size*4){
		int adjacent = puzzle[solved_index - 4 + 1] == piece[3];
		return adjacent;
	}
	// first element of a row
	if (solved_index % size == 0){
		int above = puzzle[solved_index - (size*4) + 2] == piece[0];
		return above;
	}

	int adjacent = puzzle[solved_index - 4 + 1] == piece[3];
	int above = puzzle[solved_index - (size*4) + 2] == piece[0];
	return adjacent && above;

}

void solve_puzzle(int size, char msg[]){

	// First tile of a piece
	int original_index = rand() % (size*size);
	int solved_index = 0;
	char solved_puzzle[size*size*4];
	memset(solved_puzzle, -10, size*size*4*sizeof(char));
	char original_puzzle[size*size*4];

	for(int i = 0;   i<size*size*4;   i++ ){
		original_puzzle[i] = msg[i+6];
	}

	while(!is_solved(solved_puzzle, size)){

		if(solved_puzzle[0] == -10){
			for (int i = 0; i < 4; i++){
				solved_puzzle[solved_index+i] = original_puzzle[original_index+i];
				original_puzzle[original_index+i] = -10;
			}

			solved_index += 4;

		} else {
			char piece[4] = {original_puzzle[original_index], original_puzzle[original_index+1],
								original_puzzle[original_index+2], original_puzzle[original_index+3]};

			if(can_fit(piece, solved_puzzle, solved_index, size)) {
				for(int i = 0; i < 4; i++){
					solved_puzzle[solved_index+i] = original_puzzle[original_index+i];
					original_puzzle[original_index+i] = -10;
				}

				solved_index += 4;

			} else {

			}
		}

		original_index += 4;
		original_index %= size*size*4;

	}


}

void udp_get_handler(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    if(p) { //Must check that a valid protocol control block was received
        //The message may not be zero terminated, so to ensure that we only
        //print what was sent, we can create a zero-terminated copy and print that
        char msg[p->len + 1];
        memcpy(msg, p->payload, p->len);
        msg[p->len] = '\0';

        printf("Piece %d\n", msg[6]);
        printf("Piece be 2: %d\n", msg[7]);
        solve_puzzle(5, msg);

        //Send a reply to the address which messaged us on port 7000
        //udp_sendto(pcb, p, addr, 7000);

        //Don't forget to free the protocol control block
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
