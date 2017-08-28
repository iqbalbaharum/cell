

#ifndef CELLT_H
#define CELLT_H

#include "string.h"

#define NONE "none"
#define BOUNDARIES "boundaries"
#define BUILDINGS "buildings"
#define PLACES "places"
#define TRANSIT "transit"
#define POIS "pois"
#define WATER "water"
#define ROAD "road"
#define EARTH "earth"
#define LANDUSE "landuse"
#define GEOFENCE "geofence"
#define DATA "data"


typedef enum CELL_TYPE {
     T_NONE = 0,
     T_BOUNDARIES,
     T_BUILDINGS,
     T_PLACES,
     T_TRANSIT,
     T_POIS,
     T_WATER,
     T_ROAD,
     T_EARTH,
     T_LANDUSE,
     T_GEOFENCE,
     T_DATA
} cell_type_t;


#endif
