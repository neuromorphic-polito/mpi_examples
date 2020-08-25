/**
 * \file   spinnfed/include/fed_search.h
 * \author Emanuele Parisi
 * \date   April, 2019
 *
 * File containing declarations of functions for handling searches of FED
 * patterns into FED texts.
 */

#ifndef SPINNFED_SPINNFED_INCLUDE_FED_SEARCH_H
#define SPINNFED_SPINNFED_INCLUDE_FED_SEARCH_H

#include <stdbool.h>

#include "fed_pattern.h"
#include "fed_text.h"

/**
 * \brief Look for the first exact match of the pattern in the text, starting
 * from the specified offset.
 *
 * \param text is the sequence the target pattern is looked into.
 * \param pattern is the sequence to be looked for.
 * \param offset is the first position considered in the plain sequence.
 * \param match_offset is the offset of the first match of the pattern in the
 * plain text sequence starting from the specified offset.
 * \return true if a match is found, false otherwise.
 */
bool
fed_search (fed_text text,
            fed_pattern pattern,
            uint16_t offset,
            uint16_t *match_offset);

#endif // SPINNFED_SPINNFED_INCLUDE_FED_SEARCH_H
