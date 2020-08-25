/**
 * \file   spinnfed/fed.h
 * \author Emanuele Parisi
 * \date   April, 2019
 *
 * General header for including all the SpiNNFED library headers at once.
 */

#ifndef SPINNFED_FED_H
#define SPINNFED_FED_H

#include "fed_utils.h"
#include "fed_text.h"
#include "fed_pattern.h"
#include "fed_search.h"

#if SPINNFED_ENABLE_DBG_FUNCS
#include "spinnfed/include/fed_debug.h"
#endif

#endif // SPINNFED_FED_H
