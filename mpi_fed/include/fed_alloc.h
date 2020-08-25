//
// Created by evelina on 5/27/19.
//

#include "sark.h"

#ifndef MPI_TEMPLATE_FED_ALLOC_H
#define MPI_TEMPLATE_FED_ALLOC_H

#ifndef FED_ALLOC_EXISTS
#define FED_ALLOC_EXISTS

/* ----- MALLOC OVERRIDE FOR SPINNAKER ----- */
void *malloc(int size) {
    return sark_alloc(1, size);
}
/* ----------------------------------------- */

/* ----- FREE OVERRIDE FOR SPINNAKER ----- */
void free(void *alloc) {
    sark_free(alloc);
}
/* --------------------------------------- */
#endif //FED_ALLOC_EXISTS

#endif //MPI_TEMPLATE_FED_ALLOC_H
