/**
 * \file   spinnfed/include/fed_pattern.h
 * \author Emanuele Parisi
 * \date   April, 2019
 *
 * File containing declarations of functions for handling FED encoded patterns.
 */

#ifndef SPINNFED_SPINNFED_INCLUDE_FED_PATTERN_H
#define SPINNFED_SPINNFED_INCLUDE_FED_PATTERN_H

#ifndef FED_ALLOC_EXISTS
    #include <stdlib.h>
#endif
#include <stdint.h>

#include "fed_utils.h"

/*
 * Definitions for designing the FED text data structure.
 */

/**
 * \brief Size of the buffer representing the 0-shifted FED encoded pattern and
 * related metadata.
 */
#define FED_PATTERN_BUFFER_SIZE 32

#if FED_PATTERN_BUFFER_SIZE > 256
#error "FED_PATTERN_BUFFER_SIZE cannot be bigger than 256 Bytes."
#endif

#if FED_PATTERN_BUFFER_SIZE < 4
#error "FED_TEXT_BUFFER_SIZE cannot be smaller than 4 Bytes."
#endif

/*
 * Help definitions.
 */

/**
 * \brief Position in any of the FED pattern buffers where text bit-mask is
 * stored.
 */
#define FED_PATTERN_MASK_INDEX (FED_PATTERN_BUFFER_SIZE - 2)
/**
 * \brief Position in any of the FED buffers where text encoded stream size is
 * stored.
 */
#define FED_PATTERN_LENGTH_INDEX (FED_PATTERN_BUFFER_SIZE - 1)
/**
 * \brief Maximum size in Bytes for the encoded stream.
 *
 * This is equal to the FED_PATTERN_MASK_INDEX by definition. A further
 * definition is added here only for the sake of code readability.
 */
#define FED_PATTERN_MAX_ENCODED_SIZE FED_PATTERN_MASK_INDEX

typedef struct fed_pattern_ *fed_pattern;

/**
 * \brief Allocate and initialize a FED pattern.
 *
 * \return an handle to the created data structure.
 */
fed_pattern
fed_pattern_alloc ();

/**
 * \brief Release the memory used by a FED pattern.
 *
 * \param pattern is the handle to the object to be released.
 */
void
fed_pattern_free (fed_pattern pattern);

/**
 * \brief Getter method for retrieving the FED pattern inner buffer related to
 * the i-th shift.
 *
 * \param pattern is the object which owns the buffer to be retrieved.
 * \param shift_index is the index of the shift buffer to be considered.
 * \return the inner buffer of the target i-th shifted FED pattern.
 */
uint8_t *
fed_pattern_get_buffer (fed_pattern pattern,
                        uint8_t shift_index);

/**
 * \brief Getter method for retrieving the i-th shifted FED pattern size.
 *
 * \param pattern is the object whose text size is queried.
 * \param shift_index is the index of the shift buffer to be considered.
 * \return the size of the encoded i-th shifted pattern stored in the target
 * FED buffer.
 */
uint8_t
fed_pattern_get_size (fed_pattern pattern,
                      uint8_t shift_index);

/**
 * \brief Getter method for retrieving the FED i-th pattern bit-mask related to
 * the first encoded byte.
 *
 * \param pattern is the object whose first byte bit-mask is queried.
 * \param shift_index is the index of the shift buffer to be considered.
 * \return the bit-mask of the first byte of encoded text for the target FED
 * pattern.
 */
uint8_t
fed_pattern_get_first_mask (fed_pattern pattern,
                            uint8_t shift_index);

/**
 * \brief Getter method for retrieving the FED i-th pattern bit-mask related to
 * the last encoded byte.
 *
 * \param pattern is the object whose last byte bit-mask is queried.
 * \param shift_index is the index of the shift buffer to be considered.
 * \return the bit-mask of the last byte of encoded text for the target FED
 * pattern.
 */
uint8_t
fed_pattern_get_last_mask (fed_pattern pattern,
                           uint8_t shift_index);

/**
 * \brief Retrieve the shift table content for the target symbol.
 *
 * \param pattern is the data structure whose shift table is queried.
 * \param symbol is the symbol used for querying the shift table.
 * \return the content of the table entry corresponding to the symbol passed as
 * input.
 */
uint8_t
fed_pattern_get_shift (fed_pattern pattern,
                       uint8_t symbol);

/**
 * \brief Access an encoded byte in the target pattern stream.
 *
 * \param pattern is the handle of the FED pattern whose i-th shifted encoded
 * stream is queried.
 * \param shift_index is the index of the shift buffer to be considered.
 * \param index is the position in the stream to be queried.
 * \return the byte at the target position in the encoded stream.
 */
uint8_t
fed_pattern_at (fed_pattern pattern,
                uint8_t shift_index,
                uint8_t index);

/**
 * \brief Copy the content of the byte stream received as input in the
 * 0-shifted inner buffer.
 *
 * During the copy process, at most FED_PATTERN_MASK_INDEX are copied. The
 * remaining bytes are ignored.
 *
 * \param pattern is the structure the byte stream is copied into.
 * \param stream is the byte stream to be copied.
 * \param stream_size is the size of the byte stream to be copied.
 * \param mask is the bit-mask related to the last byte in the byte stream
 * provided as input.
 * \return the number of encoded bytes actually copied into the text.
 */
uint32_t
fed_pattern_load_binary (fed_pattern pattern,
                         uint8_t const *stream,
                         uint32_t stream_size,
                         uint8_t mask);

/**
 * \brief Initialize the 1-th, 2-th, 3-th shifted buffers and the shift table
 * for the current pattern from the 0-th shifted buffer, which is assumed to be
 * already initialized.
 *
 * \param pattern is the FED pattern whose buffers has to be initialized.
 */
void
fed_pattern_expand_buffer0 (fed_pattern pattern);

#endif // SPINNFED_SPINNFED_INCLUDE_FED_PATTERN_H
