/*
 * bitstream_handler.h
 *
 *  Created on: Oct 25, 2019
 *      Author: arthur
 */

#ifndef BITSTREAM_HANDLER_H_
#define BITSTREAM_HANDLER_H_

#define MARK_FREQ		1.2
#define SPACE_FREQ		2.2

static int data_flag = 0;

/**
 * @brief Convert a char into a UART type bitstream
 * @param c Character to convert
 * @param bitstream Destination bitstream
*/
void char_to_bitstream(char c, int *bitstream);

/**
 * @brief Convert a UART type bitstream to a char
 * @param bitstream Source bitstream
 * @param c Destination character
 * @return -1 if the bitstream is incorrect 1 otherwise
*/
int bitstream_to_char(int *bitstream, char *c);

void set_data_flag();

#endif /* BITSTREAM_HANDLER_H_ */
