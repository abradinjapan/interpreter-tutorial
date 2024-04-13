#ifndef ANVIL__template
#define ANVIL__template

/* Include */
// anvil
#include "essentials.h"

/* Offsets */
// offset type
typedef enum TEMP__ot {
    // offsets
    TEMP__ot__package_start,

    // count
    TEMP__ot__COUNT,
} TEMP__ot;

// offsets
typedef struct TEMP__offsets {
    ANVIL__offset offsets[TEMP__ot__COUNT];
} TEMP__offsets;

/* Build Package */
void TEMP__code__package(ANVIL__workspace* workspace, TEMP__offsets* test_offsets, ESS__offsets* essential_offsets) {
    // write functions
    // TODO

    return;
}

#endif
