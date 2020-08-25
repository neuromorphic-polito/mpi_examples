/**
 * \file   spinnfed/include/fed_utils.h
 * \author Emanuele Parisi
 * \date   April, 2019
 *
 * File containing declarations of utility functions related to the FED
 * algorithm.
 */

#ifndef SPINNFED_INCLUDE_SPINNFED_FED_UTILS_H
#define SPINNFED_INCLUDE_SPINNFED_FED_UTILS_H

#include <stdint.h>

/**
 * \brief Encode an ASCII character into a FED-like 2-bit representation.
 *
 * The FED encoding algorithm associated 2-bits to each DNA symbol according to
 * the following scheme: A: 00, C: 01, T: 10, G: 11.
 *
 * \param ascii is the plain ASCII character to be encoded.
 * \return the byte containing the 2-bit encoded representation of the ASCII
 * character passed as input.
 */
uint8_t
fed_utils_encode_ascii (uint8_t ascii);

/**
 * \brief Encode four ASCII symbols from a plain sequence.
 *
 * \param sequence is the pointer to the first of the four characters to be
 * considered for the conversion.
 * \param encoded_mask is the pointer to the encoded byte bit-mask storage.
 * \return the FED-encoded representation of the target 4-mer.
 */
uint8_t
fed_utils_encode_4mer (char const* sequence,
                       uint8_t* encoded_mask);

#endif // SPINNFED_INCLUDE_SPINNFED_FED_UTILS_H
