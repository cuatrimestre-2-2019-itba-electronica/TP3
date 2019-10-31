#include "bitstream_handler.h"

static int data_flag = 0;

static int bitstream_buffer = 0;
static int n_data = 0;
static int zero_flag = 0;
static int uart_seq_flag = 0;

void char_to_bitstream(char c, int *bitstream)
{
	//uint32_t stream = 0x00;
	//stream |= c << 1;

	// Start
	bitstream[0] = 0;
	int parity_count = 0;

	// Data
    for (int i = 7; i >=0; --i)
    {
    	//bitstream[i+1] = c & 1;
    	//parity_count += c & 1;
    	//c=c>>1;

    	if(c & (1 << i)){
    		bitstream[i+1] = 1;
    		parity_count++;
    	}
    	else bitstream[i+1] = 0;
    }
    // Parity
    bitstream[9] = (parity_count % 2 == 0) ? 1 : 0;
    //bitstream[9] = parity_count & 1;
    // Stop
    bitstream[10] = 1;
}

/*
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
}*/

int bitstream_to_char(char *c){
	if(n_data == 11){
		n_data = 0;
		uart_seq_flag = 0;

		// Check start
		if(bitstream_buffer & UART_STARTBIT_MASK) return -1;

		// Check odd parity
		if((bitstream_buffer >> 1 & UART_PARITY_MASK) % 2 != 0) return -1;

		// Check stop
		if(!(bitstream_buffer & UART_STOPBIT_MASK)) return -1;
		int data = 0;
		int temp = (bitstream_buffer >> 2 & UART_DATA_MASK);
		for(int i=7;i>=0;i--){
			data |= (temp & 1) << i;
			temp >>= 1;
		}

		*c = (char) data;
	}

    return 1;
}

int bitstream_to_char_old(char *c){
	if(n_data == 11){
	        n_data = 0;
	        uart_seq_flag = 0;

	        // Check stop
	        int temp = bitstream_buffer & 1;
	        bitstream_buffer >>= 1;
	        if(!temp) return -1;

	        // Parity check
	        temp = bitstream_buffer & 0x1FF;
	        bitstream_buffer >>= 1;
	        if(temp % 2 != 0)return -1;

	        int data = 0;
	        for(int i=7;i>=0;i--){
	            data |= (bitstream_buffer & 1) << i;
	            bitstream_buffer >>= 1;
	        }

	        // Start check
	        temp = bitstream_buffer & 1;
	        bitstream_buffer >>= 1;
	        if(temp) return -1;

	        *c = (char) data;
	        return 1;
	    }

	return 0;
}

int get_bitstream(){
    if(data_flag){

        data_flag = 0;

        int period = period_test;
        if(period )
        //int d_mark = (period - MARK_PERIOD)*(period - MARK_PERIOD);
        //int d_space = (period - SPACE_PERIOD)*(period - SPACE_PERIOD);
        if(8000 < period && period < 21500){
            one_flag++;
            if(one_flag == 2){
                one_flag = 0;
                bitstream_buffer <<= 1;
                bitstream_buffer |= 1;
                if(uart_seq_flag) n_data++;
            }

        }
        else if(21500< period && period < 37000){
            zero_flag++;
            if(zero_flag == 4){
                zero_flag = 0;
                if(bitstream_buffer & 1 && !uart_seq_flag) uart_seq_flag = 1;
                bitstream_buffer <<= 1;
                n_data++;
            }
        }
    }

    return bitstream_buffer;
}

/*
void call_back(){
	buffer[ptr_W] = FREQ; // get freq
	if (ptr_W++ == 1024) {
		ptr_W = 0;
	}
	N_buf++;
}*/

void set_data_flag(){
	data_flag = 1;
}
