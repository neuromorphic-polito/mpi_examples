/**
 * \file   spinnfed/src/fed_pattern.c
 * \author Emanuele Parisi
 * \date   April, 2019
 *
 * File containing implementations of functions for handling FED encoded
 * patterns.
 */

#include "fed_pattern.h"

/*
 * ADT data structure.
 */

/**
 * The size of the shift table is a constant that the FED library user is not
 * allowed to see and/or modify.
 */
#define FED_PATTERN_SHIFT_TABLE_SIZE 256

/**
 * \brief ADT representing a pattern encoded with the FED encoding.
 */
struct fed_pattern_
{
    /**
     * Set of buffers representing the 0, 1, 2 and 3-shifted representation for
     * a FED-encoded pattern.
     */
    uint8_t buffer[4][FED_PATTERN_BUFFER_SIZE];
    /**
     * Shift table for running the FED search procedure.
     */
    uint8_t shift_table[FED_PATTERN_SHIFT_TABLE_SIZE];
};

/*
 * Module private functions.
 */

void
_fed_pattern_compute_shift_table (fed_pattern pattern)
{
    uint16_t i            = 0;
    uint8_t  j            = 0;
    uint8_t  shift_amount = 0;
    uint8_t  pattern_size = 0;

    // Set the default value for every entry in the shift table, to be used in
    // case the given symbol never appears in the encoded pattern.
    for (i = 0; i < FED_PATTERN_SHIFT_TABLE_SIZE; i++)
    {
        pattern->shift_table[i] = fed_pattern_get_size(pattern,
                                                       0) - (uint8_t) 1;
    }

    // Compute the shift table for the target FED pattern.
    for (i = 0;
         i < 4;
         i++)
    {
        pattern_size = fed_pattern_get_size(pattern,
                                            i);
        for (j       = 1;
             j < pattern_size - 1;
             j++)
        {
            shift_amount = (uint8_t) (pattern_size - j - 1);
            if (shift_amount < pattern->shift_table[pattern->buffer[i][j]])
            {
                pattern->shift_table[pattern->buffer[i][j]] = shift_amount;
            }
        }
    }
}

/*
 * Module public interface.
 */

fed_pattern
fed_pattern_alloc ()
{
    return (fed_pattern) malloc(sizeof(struct fed_pattern_));

}

void
fed_pattern_free (fed_pattern pattern)
{

    free(pattern);

}

uint8_t *
fed_pattern_get_buffer (fed_pattern pattern,
                        uint8_t shift_index)
{
    return pattern->buffer[shift_index];
}

uint8_t
fed_pattern_get_size (fed_pattern pattern,
                      uint8_t shift_index)
{
    return pattern->buffer[shift_index][FED_PATTERN_LENGTH_INDEX];
}

uint8_t
fed_pattern_get_first_mask (fed_pattern pattern,
                            uint8_t shift_index)
{
    return (uint8_t) (-1) >> (2 * shift_index);
}

uint8_t
fed_pattern_get_last_mask (fed_pattern pattern,
                           uint8_t shift_index)
{
    return pattern->buffer[shift_index][FED_PATTERN_MASK_INDEX];
}

uint8_t
fed_pattern_get_shift (fed_pattern pattern,
                       uint8_t symbol)
{
    return pattern->shift_table[symbol];
}

uint8_t
fed_pattern_at (fed_pattern pattern,
                uint8_t shift_index,
                uint8_t index)
{
    return pattern->buffer[shift_index][index];
}

uint32_t
fed_pattern_load_binary (fed_pattern pattern,
                         uint8_t const *stream,
                         uint32_t stream_size,
                         uint8_t mask)
{
    uint8_t i = 0;

    // Compute the number of bytes to be loaded, considering that at most
    // FED_PATTERN_MAX_ENCODED_SIZE bytes can be loaded.
    if (stream_size > FED_PATTERN_MAX_ENCODED_SIZE)
    {
        stream_size = FED_PATTERN_MAX_ENCODED_SIZE;
        mask        = (uint8_t) (-1);
    }

    // Copy the encoded stream bytes in the FED pattern buffer.
    for (i = 0;
         i < stream_size;
         i++)
    {
        pattern->buffer[0][i] = stream[i];
    }

    // Set the current encoded stream length and copy the last byte bit-mask in
    // the 0-shifted FED pattern buffer.
    pattern->buffer[0][FED_PATTERN_LENGTH_INDEX] = (uint8_t) stream_size;
    pattern->buffer[0][FED_PATTERN_MASK_INDEX]   = mask;

    return stream_size;
}

void
fed_pattern_expand_buffer0 (fed_pattern pattern)
{
    uint8_t *buffers[]   = {pattern->buffer[0],
                            pattern->buffer[1],
                            pattern->buffer[2],
                            pattern->buffer[3]};
    uint8_t shift        = 0;
    uint8_t last_mask    = 0;
    uint8_t encoded_size = 0;
    uint8_t i            = 0;

    // Initialize the shift_index, and the byte bit-mask from buffer 0, which
    // is assumed to be already initialized.
    encoded_size = fed_pattern_get_size(pattern,
                                        0);
    last_mask    = fed_pattern_get_last_mask(pattern,
                                             0);
    if (last_mask == (uint8_t) (-1))
    {
        // If all the bits from the buffer-0 last byte are all to be
        // considered, the first shift make the sequence one byte longer.
        last_mask = 0xc0;
        encoded_size++;
    }
    else
    {
        last_mask = (uint8_t) ((last_mask >> 2) | 0xc0);
    }

    // Initialize the pattern shifted buffers.
    for (shift = 1;
         shift < 4;
         shift++)
    {
        buffers[shift][FED_PATTERN_MASK_INDEX]   = last_mask;
        buffers[shift][FED_PATTERN_LENGTH_INDEX] = encoded_size;

        // Compute the shifted patterns from the encoded sequence in the first
        // buffer.
        for (i = 0;
             i < encoded_size;
             i++)
        {
            if (i == 0)
            {
                // The first byte in the encoded pattern has only to be shifted
                // forward by two positions.
                buffers[shift][i] = buffers[0][i] >> (2 * shift);
            }
            else if (i == encoded_size - 1 &&
                     encoded_size != buffers[0][FED_PATTERN_LENGTH_INDEX])
            {
                buffers[shift][i] = buffers[0][i - 1] << (8 - 2 * shift);
            }
            else
            {
                buffers[shift][i] = (buffers[0][i] >> (2 * shift)) |
                                    (buffers[0][i - 1] << (8 - 2 * shift));
            }
        }

        // Update the last byte bit-mask and the encoded sequence length for
        // the next iteration.
        if (last_mask == (uint8_t) (-1))
        {
            last_mask = 0xc0;
            encoded_size++;
        }
        else
        {
            last_mask = (uint8_t) ((last_mask >> 2) | 0xc0);
        }
    }

    // Once the encoded pattern is propagated to every buffer, compute the
    // shift-table for the current pattern.
    _fed_pattern_compute_shift_table(pattern);
}
