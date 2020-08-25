/**
 * \file   spinnfed/include/fed_debug.h
 * \author Emanuele Parisi
 * \date   April, 2019
 *
 * File containing declarations of debug utilities for printing FED data
 * structures in a human readable formats.
 */

#ifndef SPINNFED_SPINNFED_INCLUDE_FED_DEBUG_H
#define SPINNFED_SPINNFED_INCLUDE_FED_DEBUG_H

#if SPINNFED_ENABLE_DBG_FUNCS

#include <stdio.h>

#include "fed_text.h"
#include "fed_pattern.h"

/**
 * \brief Print a single byte in a human readable format on the standard
 * output.
 *
 * \param byte is the byte to be printed out.
 */
void
fed_debug_print_uint8 (uint8_t byte);

/**
 * \brief Print a buffer of bytes in a human readable format.
 *
 * \param buffer is the stream of bytes to be printed out.
 * \param buffer_size is the size of the stream to be printed.
 */
void
fed_debug_print_uint8_buffer (uint8_t const *buffer,
                              uint8_t buffer_size);

/**
 * \brief Print the FED text on the standard output in a human readable format.
 *
 * \param text is the FED text to be printed out.
 */
void
fed_debug_print_text (fed_text text);

/**
 * \brief Print the FED pattern on the standard output in a human readable
 * format.
 *
 * \param text is the FED pattern to be printed out.
 */
void
fed_debug_print_pattern (fed_pattern pattern);

#endif

#endif // SPINNFED_SPINNFED_INCLUDE_FED_DEBUG_H
