/*
 * bitstream_handler.h
 *
 *  Created on: Oct 25, 2019
 *      Author: arthur
 */

#ifndef BITSTREAM_HANDLER_H_
#define BITSTREAM_HANDLER_H_

#define MARK_PERIOD			1200
#define SPACE_PERIOD		2400

#define UART_STARTBIT_MASK      (1 << 10)
#define UART_STOPBIT_MASK       1
#define UART_DATA_MASK          0xFF
#define UART_PARITY_MASK        0x1FF

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
int bitstream_to_char(char *c);

void set_data_flag();

#endif /* BITSTREAM_HANDLER_H_ */
