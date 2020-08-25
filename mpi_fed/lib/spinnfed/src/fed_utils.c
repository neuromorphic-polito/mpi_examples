/**
 * \file   spinnfed/src/fed_utils.c
 * \author Emanuele Parisi
 * \date   April, 2019
 *
 * File containing implementations of utility functions related to the FED
 * algorithm.
 */

#include "fed_utils.h"

uint8_t
fed_utils_encode_ascii (uint8_t ascii)
{
	return (ascii & (uint8_t) 0x06) >> 1;
}

uint8_t
fed_utils_encode_4mer (char const *sequence,
                       uint8_t *encoded_mask)
{
	uint8_t i              = 0;
	uint8_t encoded_symbol = 0;
	uint8_t encoded_byte   = 0;

	*encoded_mask = 0x00;
	for (i = 0;
	     i < 4 && sequence[i] != '\0';
	     i++)
	{
		encoded_symbol = fed_utils_encode_ascii((uint8_t) sequence[i]);
		encoded_byte |= encoded_symbol << (6 - 2 * i);
		*encoded_mask |= 0x03 << (6 - 2 * i);
	}

	return encoded_byte;
}
