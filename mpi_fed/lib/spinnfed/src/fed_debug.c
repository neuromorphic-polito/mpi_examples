/**
 * \file   spinnfed/src/fed_debug.c
 * \author Emanuele Parisi
 * \date   April, 2019
 *
 * File containing implementations of debug utilities for printing FED data
 * structures in a human readable formats.
 */

#if SPINNFED_ENABLE_DBG_FUNCS

#include "fed_debug.h"

void
fed_debug_print_uint8 (uint8_t byte)
{
	uint8_t i    = 0;
	uint8_t mask = 0;

	for (i = 0; i < 8; i++)
	{
		mask = (uint8_t) (0x80 >> i);
		printf("%d",
		       (byte & mask) >> (uint8_t) (7 - i));
	}
}

void
fed_debug_print_uint8_buffer (uint8_t const *buffer,
                              uint8_t buffer_size)
{
	uint8_t i = 0;

	for (i = 0;
	     i < buffer_size - 1;
	     i++)
	{
		fed_debug_print_uint8(buffer[i]);
		printf(" ");
	}
	fed_debug_print_uint8(buffer[i]);
}

void
fed_debug_print_text (fed_text text)
{
	printf("=== TEXT ===\n");
	printf("Buffer : ");
	fed_debug_print_uint8_buffer(fed_text_get_buffer(text),
	                             fed_text_get_size(text));
	printf("\n");
	printf("Mask   : ");
	fed_debug_print_uint8(fed_text_get_mask(text));
	printf("\n");
	printf("Size   : %d\n",
	       fed_text_get_size(text));
}

void
fed_debug_print_pattern (fed_pattern pattern)
{
	uint8_t i = 0;

	printf("=== PATTERN ===\n");
	for (i = 0;
	     i < 4;
	     i++)
	{
		printf("Buffer %d   : ",
		       i);
		fed_debug_print_uint8_buffer(fed_pattern_get_buffer(pattern,
		                                                    i),
		                             fed_pattern_get_size(pattern,
		                                                  i));
		printf("\n");
		printf("First mask : ");
		fed_debug_print_uint8(fed_pattern_get_first_mask(pattern,
		                                                 i));
		printf("\n");
		printf("Last mask  : ");
		fed_debug_print_uint8(fed_pattern_get_last_mask(pattern,
		                                                    i));
		printf("\n");
		printf("Size       : %d\n",
		       fed_pattern_get_size(pattern,
		                            i));
	}
}

#endif
