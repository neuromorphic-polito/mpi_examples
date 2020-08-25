/**
 * \file   spinnfed/include/fed_text.h
 * \author Emanuele Parisi
 * \date   April, 2019
 *
 * File containing declarations of functions for handling FED encoded texts.
 */

#ifndef SPINNFED_SPINNFED_INCLUDE_FED_TEXT_H
#define SPINNFED_SPINNFED_INCLUDE_FED_TEXT_H

#ifndef FED_ALLOC_EXISTS
    #include <stdlib.h>
#endif
#include <stdint.h>

#include "fed_utils.h"

/*
 * Definitions for designing the FED text data structure.
 */

/**
 * \brief Size of the buffer representing the FED encoded text and related
 * metadata.
 */
#define FED_TEXT_BUFFER_SIZE 256

#if FED_TEXT_BUFFER_SIZE > 256
#error "FED_TEXT_BUFFER_SIZE cannot be bigger than 256 Bytes."
#endif

#if FED_TEXT_BUFFER_SIZE < 3
#error "FED_TEXT_BUFFER_SIZE cannot be smaller than 3 Bytes."
#endif

/*
 * Help definitions.
 */

/**
 * \brief Position in the FED buffer where text bit-mask is stored.
 */
#define FED_TEXT_MASK_INDEX (FED_TEXT_BUFFER_SIZE - 2)
/**
 * \brief Position in the FED buffer where text encoded stream size is stored.
 */
#define FED_TEXT_LENGTH_INDEX (FED_TEXT_BUFFER_SIZE - 1)
/**
 * \brief Maximum size in Bytes for the encoded stream.
 *
 * This is equal to the FED_TEXT_MASK_INDEX by definition. A further definition
 * is added here only for the sake of code readability.
 */
#define FED_TEXT_MAX_ENCODED_SIZE FED_TEXT_MASK_INDEX

typedef struct fed_text_ *fed_text;

/**
 * \brief Allocate and initialize a FED text.
 *
 * \return the pointer to the created data structure.
 */
fed_text
fed_text_alloc ();

/**
 * \brief Release the memory used by a FED text.
 *
 * \param text is the handle to the object to be released.
 */
void
fed_text_free (fed_text text);

/**
 * \brief Getter method for retrieving the FED text inner buffer.
 *
 * \param text is the object which owns the buffer to be retrieved.
 * \return the inner buffer of the target FED text.
 */
uint8_t *
fed_text_get_buffer (fed_text text);

/**
 * \brief Getter method for retrieving the FED text size.
 *
 * \param text is the object whose text size is queried.
 * \return the size of the encoded text stored in the target FED text.
 */
uint8_t
fed_text_get_size (fed_text text);

/**
 * \brief Getter method for retrieving the FED text bit-mask.
 *
 * \param text is the object whose bit-mask is queried.
 * \return the bit-mask of the last byte of encoded text for the target FED
 * text.
 */
uint8_t
fed_text_get_mask (fed_text text);

/**
 * \brief Access an encoded byte in the target text stream.
 *
 * \param text is the handle of the FED text whose encoded stream is queried.
 * \param index is the position in the stream to be queried.
 * \return the byte at the target position in the encoded stream.
 */
uint8_t
fed_text_at (fed_text text,
             uint8_t index);

/**
 * \brief Copy the content of the byte stream received as input in the text
 * inner buffer.
 *
 * During the copy process, at most FED_TEXT_MAX_ENCODED_SIZE are copied. The
 * remaining bytes are ignored.
 *
 * \param text is the structure the byte stream is copied into.
 * \param stream is the byte stream to be copied.
 * \param stream_size is the size of the byte stream to be copied.
 * \param mask is the bit-mask related to the last byte in the byte stream
 * provided as input.
 * \return the number of encoded bytes actually copied into the text.
 */
uint32_t
fed_text_load_binary (fed_text text,
                      uint8_t const *stream,
                      uint32_t stream_size,
                      uint8_t mask);

#endif // SPINNFED_SPINNFED_INCLUDE_FED_TEXT_H
