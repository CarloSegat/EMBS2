/**
 * Example of using the Digilent display drivers for Zybo Z7 HDMI output, with animation
 * Russell Joyce, 11/03/2019
 */
#include <math.h>
#include <stdio.h>
#include "xil_types.h"
#include "xil_cache.h"
#include "xparameters.h"
#include "zybo_z7_hdmi/display_ctrl.h"

// Frame size (based on 1440x900 resolution, 32 bits per pixel)
#define MAX_FRAME (1440*900)
#define FRAME_STRIDE (1440*4)

DisplayCtrl dispCtrl; // Display driver struct
u32 frameBuf[DISPLAY_NUM_FRAMES][MAX_FRAME]; // Frame buffers for video data
void *pFrames[DISPLAY_NUM_FRAMES]; // Array of pointers to the frame buffers

int get_colour(int tile) {
	switch(tile) {

		case 0:
			return 16765951;
			break;

		case 1:
			return 16776960;
			break;

		case 2:
			return 65280;
			break;

		case 3:
			return 16711680;
			break;

		case 4:
			return 16711935;
			break;


		case 5:
			return 10083745;
			break;

		case 6:
			return 10079231;
			break;

		case 7:
			return 1931520;
			break;

		case 8:
			return 9127796;
			break;

		case 9:
			return 16753920;
			break;
	}
}

int print_puzzle(char *puzzle, int size) {
	// Initialise an array of pointers to the 2 frame buffers
	int i;
	for (i = 0; i < DISPLAY_NUM_FRAMES; i++)
		pFrames[i] = frameBuf[i];
	// Initialise the display controller
	DisplayInitialize(&dispCtrl, XPAR_AXIVDMA_0_DEVICE_ID, XPAR_VTC_0_DEVICE_ID, XPAR_HDMI_AXI_DYNCLK_0_BASEADDR, pFrames, FRAME_STRIDE);
	// Start with the first frame buffer (of two)
	DisplayChangeFrame(&dispCtrl, 0);
	// Set the display resolution
	DisplaySetMode(&dispCtrl, &VMODE_1440x900);
	// Enable video output
	DisplayStart(&dispCtrl);

	printf("\n\r");
	printf("HDMI output enabled\n\r");
	printf("Current Resolution: %s\n\r", dispCtrl.vMode.label);
	printf("Pixel Clock Frequency: %.3fMHz\n\r", dispCtrl.pxlFreq);
	printf("Starting animation loop...\n\r");


	u32 *frame;
	u32 buff = dispCtrl.curFrame;

	while (1) {
			// Switch the frame we're modifying to be the back buffer (1 to 0, or 0 to 1)
			buff = !buff;
			frame = (u32 *)dispCtrl.framePtr[buff];

			// Clear the entire frame to white (inefficient, but it works)
			memset(frame, 0xFF, MAX_FRAME*4);

			for(int tile = 0;   tile<size*size;   tile++)
			{
				for(int section=0;   section<4;   section++)
				{

					int colour_to_use = get_colour(puzzle[tile*4 + section]);

					int x_start;
					int x_width;
					int y_max;
					int y_start;

					switch(section){
						case 0:
							x_start = 1;
							x_width = 43;
							y_max = 22;
							y_start = 0;
							break;
						case 1:
							x_start = 24;
							x_width = 22;
							y_max = 43;
							y_start = 0;
							break;
						case 2:
							x_start = 1;
							x_width = 43;
							y_max = 22;
							y_start = 0;
							break;
						case 3:
							x_start = 0;
							x_width = 22;
							y_max = 43;
							y_start = 43;
							break;
					}

					int row = floor(tile / size);
					int row_stride = (row * 45) * 1440;

					int column = tile % size;
					int column_stride = column * 45;

					int increasing = 1;
					int next_y = y_start;
					int x_iteration = 0;

					for(int x = x_start;   x < x_start + x_width;   x++){
						x_iteration += 1;

						switch(section){
							case 0:
								for(int y = y_start; y < next_y; y++){
									frame[y*1440 + row_stride + (x+ column_stride)] = colour_to_use;
								}
								if(increasing){
									next_y += 1;
								} else {
									next_y -= 1;
								}

								if(next_y == y_max){
									increasing = 0;
								}
								break;
							case 1:
								for(int y = y_start; y < next_y; y++){
									frame[y*1440 + row_stride + (1440*23) - (x_iteration*1440) + (x+ column_stride)] = colour_to_use;
								}
								next_y += 2;

								break;
							case 2:
								for(int y = y_start; y < next_y; y++){
									frame[-y*1440 + row_stride + (1440*45) + (x+ column_stride)] = colour_to_use;
								}
								if(increasing){
									next_y += 1;
								} else {
									next_y -= 1;
								}

								if(next_y == y_max){
									increasing = 0;
								}
								break;
							case 3:
								for(int y = 0; y < next_y; y++){
									frame[y*1440 + row_stride + (x_iteration*1440) + (x+ column_stride)] = colour_to_use;
								}
								next_y -= 2;

								break;
						}
					}

				}
			}

			// Flush everything out to DDR from the cache
			Xil_DCacheFlush();

			// Switch active frame to the back buffer (will take place during next vertical blanking period)
			DisplayChangeFrame(&dispCtrl, buff);

			// Wait for the frame to switch before continuing (so after current frame has been drawn)
			DisplayWaitForSync(&dispCtrl);
	}

	return 0;
}
