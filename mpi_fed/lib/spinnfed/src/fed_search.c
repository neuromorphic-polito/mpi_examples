/**
 * \file   spinnfed/src/fed_search.c
 * \author Emanuele Parisi
 * \date   April, 2019
 *
 * File containing implementations of functions for handling searches of FED
 * patterns into FED texts.
 */

#include "fed_search.h"

/*
 * Module private functions.
 */

bool
_fed_search_match_inner (fed_text text,
                         uint8_t text_offset,
                         fed_pattern pattern,
                         uint8_t pattern_shift_index)
{
    bool    match_found    = true;
    uint8_t text_symbol    = 0;
    uint8_t pattern_symbol = 0;
    uint8_t pattern_offset = 0;

    // If the pattern buffer is longer than the text one, return false.
    if (fed_pattern_get_size(pattern,
                             pattern_shift_index) > fed_text_get_size(text))
    {
        match_found = false;
    }

    // Make sure to access always valid positions in the text buffer.
    if (fed_pattern_get_size(pattern,
                             pattern_shift_index) > text_offset) {
        match_found = false;
    }

    pattern_offset = fed_pattern_get_size(pattern,
                                          pattern_shift_index) - (uint8_t) 2;
    while (pattern_offset > 0 && match_found)
    {
        text_symbol    = fed_text_at(text,
                                     text_offset);
        pattern_symbol = fed_pattern_at(pattern,
                                        pattern_shift_index,
                                        pattern_offset);
        match_found    = text_symbol == pattern_symbol;
        text_offset--;
        pattern_offset--;
    }

    return match_found;
}

bool
_fed_search_match_right_border (fed_text text,
                                uint8_t text_offset,
                                fed_pattern pattern,
                                uint8_t pattern_shift_index)
{
    uint8_t pattern_size      = fed_pattern_get_size(pattern,
                                                     pattern_shift_index);
    uint8_t pattern_last      = fed_pattern_at(pattern,
                                               pattern_shift_index,
                                               pattern_size - (uint8_t) 1);
    uint8_t pattern_mask      = fed_pattern_get_last_mask(pattern,
                                                          pattern_shift_index);
    uint8_t text_right_offset = text_offset + (uint8_t) 1;

    return (fed_text_at(text,
                        text_right_offset) & pattern_mask) == pattern_last;
}

bool
_fed_search_match_left_border (fed_text text,
                               uint8_t text_offset,
                               fed_pattern pattern,
                               uint8_t pattern_shift_index)
{
    uint8_t pattern_size     = fed_pattern_get_size(pattern,
                                                    pattern_shift_index);
    uint8_t pattern_first    = fed_pattern_at(pattern,
                                              pattern_shift_index,
                                              0);
    uint8_t pattern_mask     = fed_pattern_get_first_mask(pattern,
                                                          pattern_shift_index);
    uint8_t text_left_offset = text_offset - pattern_size + (uint8_t) 2;

    return (fed_text_at(text,
                        text_left_offset) & pattern_mask) == pattern_first;
}

uint16_t
_fed_text_offset_to_match_offset (uint8_t text_offset,
                                  uint8_t pattern_size,
                                  uint8_t shift_index)
{
    return (uint16_t) (4 * (text_offset - pattern_size + 2) + shift_index);
}

/*
 * Module public functions.
 */

// FIXME very badly written, it can be improved a lot !!!
bool
fed_search (fed_text text,
            fed_pattern pattern,
            uint16_t offset,
            uint16_t *match_offset)
{
    uint8_t  text_offset        = 0;
    uint8_t  pattern_size       = 0;
    uint8_t  i                  = 0;
    uint16_t offset_encoded     = offset >> 2;
    uint8_t  offset_remainder   = (uint8_t)(offset & 0x03);
    bool     search_in_progress = true;
    bool     match_found        = false;

    // Compute the actual text offset the search procedure begin with.
    text_offset = (uint8_t) (offset_encoded + fed_pattern_get_size(pattern,
                                                                   0) - 2);
    if (fed_pattern_get_size(pattern,
                             offset_remainder) != fed_pattern_get_size(pattern,
                                                                       0))
    {
        text_offset++;
    }

    // FED search loop.
    while (search_in_progress)
    {
        uint8_t shift_amount = 0;

        // Try to match pattern and text for the current position with
        // different shifts.
        for (i = offset_remainder;
             i < 4 && !match_found;
             i++)
        {
            pattern_size = fed_pattern_get_size(pattern,
                                                i);
            if (_fed_search_match_inner(text,
                                        text_offset,
                                        pattern,
                                        i) &&
                _fed_search_match_left_border(text,
                                              text_offset,
                                              pattern,
                                              i) &&
                _fed_search_match_right_border(text,
                                               text_offset,
                                               pattern,
                                               i))
            {
                *match_offset = _fed_text_offset_to_match_offset(text_offset,
                                                                 pattern_size,
                                                                 i);
                // FIXME writen as it is, it sucks !
                if (*match_offset >= offset)
                {
                    match_found        = true;
                    search_in_progress = false;
                }
            }
        }

        // If no match is found at the current text position, access the
        // pattern shift table and compute the next text position to be
        // checked.
        if (!match_found)
        {
            shift_amount = fed_pattern_get_shift(pattern,
                                                 fed_text_at(text,
                                                             text_offset +
                                                             (uint8_t) 1));

            // If the shift amount brings the search over the encoded text
            // length, report that the pattern is not found in the text.
            if (fed_text_get_size(text) - shift_amount > text_offset)
            {
                text_offset += shift_amount;
            }
            else
            {
                match_found        = false;
                search_in_progress = false;
            }
        }

        // Once the first iteration if finished, reset the 'offset_remainder'
        // variable, such that every n-shifted pattern buffers are considered
        // in the next iterations.
        offset_remainder = 0;
    }

    return match_found;
}
