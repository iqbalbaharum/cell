
#ifndef CELLS_H
#define CELLS_H

#include "cellt.h"

typedef struct cell {
    bson_oid_t oid;
    char *pQuadKey;
    char *pLayerid;
    cell_type_t cell_type;
    bool is_numeric;
    char *pValue;
} cell_t;

typedef struct cell_search {
     char *oid;
     char *pLayerid;
     char *pQuadKey;
} cell_search_t;

#endif
