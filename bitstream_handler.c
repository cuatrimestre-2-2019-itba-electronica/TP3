#include "bitstream_handler.h"

// Variables for circular buffer
static char buffer[1024];
static int ptr_R = 0;
static int ptr_W = 0;
static int N_buf = 0;

void char_to_bitstream(char c, int *bitstream)
{
	// Start
	bitstream[0] = 0;
	int parity_count = 0;

	// Data
    for (int i = 7; i >= 0; --i)
    {
    	if(c & (1 << i)){
    		bitstream[i+1] = 1;
    		parity_count++;
    	}
    	else bitstream[i+1] = 0;
    }
    // Parity
    bitstream[9] = (parity_count % 2 == 0) ? 1 : 0;

    // Stop
    bitstream[10] = 1;
}

int bitstream_to_char(int *bitstream, char *c)
{
	int binary_dato = 0;
	int parity_count = 0;

	// Check start
	if(bitstream[0] != 0) return -1;

	for (int i = 7; i >= 0; --i)
	{
		binary_dato += bitstream[i+1] << i;
		if(bitstream[i+1]) parity_count++;
	}

	if(bitstream[9]) parity_count++;

	// Parity check

	if(parity_count % 2 == 0) return -1;

	*c = (char) binary_dato;

	return 1;
}

int * get_bitstream(){
	if(data_flag){
		int bitstream[11];

		if(MARK_FREQ - buffer[ptr_R] < SPACE_FREQ - buffer[ptr_R]){

		}

		data_flag = 0;
	}
}

void call_back(){
//	buffer[ptr_W] = FREQ; // get freq
	if (ptr_W++ == 1024) {
		ptr_W = 0;
	}
	N_buf++;
}

void set_data_flag(){
	data_flag = 1;
}
