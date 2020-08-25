/**
 * \file   spinnfed/src/fed_text.c
 * \author Emanuele Parisi
 * \date   April, 2019
 *
 * File containing implementations of functions for handling FED encoded texts.
 */

#include "fed_text.h"

/*
 * ADT data structure.
 */

/**
 * \brief ADT representing a text encoded with the FED encoding.
 */
struct fed_text_
{
    /**
     * Buffer storing the encoded stream, the last-byte bit-mask and the
     * effective stream size.
     */
    uint8_t buffer[FED_TEXT_BUFFER_SIZE];
};

/*
 * Module public interface.
 */

fed_text
fed_text_alloc ()
{

    return (fed_text) malloc(sizeof(struct fed_text_));

}

void
fed_text_free (fed_text text)
{

    free(text);

}

uint8_t *
fed_text_get_buffer (fed_text text)
{
    return text->buffer;
}

uint8_t
fed_text_get_mask (fed_text text)
{
    return text->buffer[FED_TEXT_MASK_INDEX];
}

uint8_t
fed_text_get_size (fed_text text)
{
    return text->buffer[FED_TEXT_LENGTH_INDEX];
}

uint8_t
fed_text_at (fed_text text,
             uint8_t index)
{
    return text->buffer[index];
}

uint32_t
fed_text_load_binary (fed_text text,
                      uint8_t const *stream,
                      uint32_t stream_size,
                      uint8_t mask)
{
    uint32_t i = 0;

    // Compute the number of bytes to be loaded, considering that at most
    // FED_TEXT_MAX_ENCODED_SIZE bytes can be stored within the FED text inner
    // buffer.
    if (stream_size > FED_TEXT_MAX_ENCODED_SIZE)
    {
        stream_size = FED_TEXT_MAX_ENCODED_SIZE;
        mask        = (uint8_t) (-1);
    }

    // Copy the encoded stream bytes in the FED text buffer.
    for (i = 0;
         i < stream_size;
         i++)
    {
        text->buffer[i] = stream[i];
    }

    // Set the current encoded stream length anf copy the last byte bit-mask in
    // the FED text buffer.
    text->buffer[FED_TEXT_MASK_INDEX]   = mask;
    text->buffer[FED_TEXT_LENGTH_INDEX] = (uint8_t) stream_size;

    return stream_size;
}
