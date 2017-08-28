#pragma link "/usr/local/lib/libmongoc-1.0.so"
#pragma link "/usr/local/lib/libbson-1.0.so"
#pragma include "/usr/local/include/libmongoc-1.0"
#pragma include "/usr/local/include/libbson-1.0"

#include "bcon.h"
#include "mongoc.h"
#include "bson.h"

#include "gwan.h"
#include "string.h"

#include "cells.h"
#include "cellt.h"

// prototype
void insert(xbuf_t *pReply, cell_t *pCell, mongoc_collection_t *pCollection);
void get(xbuf_t *pReply, cell_search_t *pCellSearch, mongoc_collection_t *pCollection);


// Main execution
int main (int   argc, char *argv[])
{
     mongoc_client_t *pClient;
     mongoc_collection_t  *pCollection;

     xbuf_t *pReply = get_reply(argv);

   /*
    * Required to initialize libmongoc's internals
    */
    mongoc_init ();

    pClient = mongoc_client_new ("mongodb://localhost:27017/?appname=rivil");
    pCollection = mongoc_client_get_collection (pClient, "rivil", "cell");

    /*
    * Get HTTP_HEADERS - /GET /POST /PUT /DELETE
    */
    http_t *pHttp = (void*)get_env(argv, HTTP_HEADERS);

    switch(pHttp->h_method) {
         case HTTP_GET:
         {
              cell_search_t *pSearch = (cell_search_t*) malloc(sizeof(cell_search_t));

              // clear memory
              pSearch->pLayerid = 0;
              pSearch->pQuadKey = 0;

              get_arg("layer=", &pSearch->pLayerid, argc, argv);
              get_arg("key=", &pSearch->pQuadKey, argc, argv);

              get(pReply, pSearch, pCollection);

         }
         break;
         case HTTP_POST:
         {
              cell_t *pCell = (cell_t *) malloc(sizeof(cell_t));

              // default value
              pCell->is_numeric = false;

              char *type = 0, *numeric = 0;
              get_arg("key=", &pCell->pQuadKey, argc, argv);
              get_arg("layer=", &pCell->pLayerid, argc, argv);
              get_arg("type=", &type, argc, argv);
              get_arg("is_value_numeric=", &numeric, argc, argv);
              get_arg("value=", &pCell->pValue, argc, argv);

              // if compulsory component is missing
              if(pCell->pQuadKey == NULL) {
                   return HTTP_400_BAD_REQUEST;
              }

              // type
              if(strcasecmp(type, BOUNDARIES) == 0) {
                   pCell->cell_type = T_BOUNDARIES;
              } else if(strcasecmp(type, BUILDINGS) == 0) {
                   pCell->cell_type = T_BUILDINGS;
              } else if(strcasecmp(type, PLACES) == 0) {
                   pCell->cell_type = T_PLACES;
              } else if(strcasecmp(type, TRANSIT) == 0) {
                   pCell->cell_type = T_TRANSIT;
              } else if(strcasecmp(type, POIS) == 0) {
                   pCell->cell_type = T_POIS;
              } else if(strcasecmp(type, WATER) == 0) {
                   pCell->cell_type = T_WATER;
              } else if(strcasecmp(type, ROAD) == 0) {
                   pCell->cell_type = T_ROAD;
              } else if(strcasecmp(type, EARTH) == 0) {
                   pCell->cell_type = T_EARTH;
              } else if(strcasecmp(type, LANDUSE) == 0) {
                   pCell->cell_type = T_LANDUSE;
              } else if(strcasecmp(type, GEOFENCE) == 0) {
                   pCell->cell_type = T_GEOFENCE;
              } else if(strcasecmp(type, DATA) == 0) {
                   pCell->cell_type = T_DATA;
              } else {
                   pCell->cell_type = T_NONE;
              }

              // is_numeric
              if(numeric) {

                   if(strcasecmp(numeric, "1") == 0 || strcasecmp(numeric, "true") == 0 || strcasecmp(numeric, "yes") == 0) {
                        pCell->is_numeric = true;
                   } else if(strcasecmp(numeric, "0") == 0 || strcasecmp(numeric, "false") == 0 || strcasecmp(numeric, "no") == 0) {
                        pCell->is_numeric = false;
                   } else {
                        return HTTP_400_BAD_REQUEST;
                   }
              }

              insert(pReply, pCell, pCollection);
         }
         break;
    }

    mongoc_collection_destroy (pCollection);
    mongoc_client_destroy (pClient);
    mongoc_cleanup();

   return HTTP_200_OK;
}

/***********************************************************************
 * get CELL record based on layer
 ***********************************************************************/
void get(xbuf_t *pReply, cell_search_t *pCellSearch, mongoc_collection_t *pCollection) {

     bson_t *pQuery = 0;
     const bson_t *pCell = NULL;

     xbuf_t xbuf;
     xbuf_init(&xbuf);

     jsn_t *pDocument = jsn_add_node(0, "users");
     jsn_t *pResults = jsn_add_array(pDocument, "cells", 0);

     pQuery = bson_new();

     if(pCellSearch->pLayerid != NULL) {
          BSON_APPEND_UTF8 (pQuery, "layer", pCellSearch->pLayerid);
     }

     if(pCellSearch->pQuadKey != NULL) {
          BSON_APPEND_UTF8 (pQuery, "key", pCellSearch->pQuadKey);
     }

     mongoc_cursor_t *pCursor = mongoc_collection_find_with_opts (pCollection, pQuery, NULL, NULL);

     if(pResults) {

          jsn_t *pNode;
          bson_iter_t iter;

          while(mongoc_cursor_next (pCursor, &pCell)) {
               pNode = jsn_add_node(pResults, "");

               // Loop through the document to find
               // specific key: value
               if(bson_iter_init(&iter, pCell)){
                    while (bson_iter_next (&iter)) {
                         // _id
                         if(strcmp("_id", bson_iter_key(&iter)) == 0) {
                              bson_oid_t oid;
                              char oidstr[25];

                              if(BSON_ITER_HOLDS_OID(&iter)) {
                                   bson_oid_copy (bson_iter_oid (&iter), &oid);
                                   bson_oid_to_string(&oid, oidstr);
                                   jsn_add_string(pNode, "id", oidstr);
                              }

                         } else if (strcmp("key", bson_iter_key (&iter)) == 0) {

                              jsn_add_string(pNode, "key", bson_iter_utf8(&iter, NULL));

                         } else if (strcmp("layer", bson_iter_key (&iter)) == 0) {

                              jsn_add_string(pNode, "layer", bson_iter_utf8(&iter, NULL));

                         } else if (strcmp("type", bson_iter_key (&iter)) == 0) {

                              jsn_add_string(pNode, "type", bson_iter_utf8(&iter, NULL));

                         } else if (strcmp("is_numeric", bson_iter_key (&iter)) == 0) {

                              jsn_add_bool(pNode, "is_numeric", bson_iter_bool(&iter));

                         } else if (strcmp("value", bson_iter_key (&iter)) == 0) {

                              jsn_add_string(pNode, "value", bson_iter_utf8(&iter, NULL));

                         }
                    }
               }
          }

          // jsn_free(pNode);
     }

     char *pText = jsn_totext(&xbuf, pDocument, 0);
     xbuf_xcat(pReply, "%s", pText);

     jsn_free(pResults);
     jsn_free(pDocument);
     bson_destroy(pQuery);
}

/***********************************************************************
 * insert CELL record to db
 ***********************************************************************/
void insert(xbuf_t *pReply, cell_t *pCell, mongoc_collection_t *pCollection) {

     bson_t *pDocument = bson_new();

    // id
    bson_oid_init (&pCell->oid, NULL);
    BSON_APPEND_OID (pDocument, "_id", &pCell->oid);
    BSON_APPEND_UTF8 (pDocument, "key", pCell->pQuadKey);
    BSON_APPEND_UTF8 (pDocument, "layer", pCell->pLayerid);

    switch(pCell->cell_type) {
         case T_BOUNDARIES:
               BSON_APPEND_UTF8(pDocument, "type", BOUNDARIES);
         break;
         case T_BUILDINGS:
               BSON_APPEND_UTF8(pDocument, "type", BUILDINGS);
         break;
         case T_PLACES:
               BSON_APPEND_UTF8(pDocument, "type", PLACES);
         break;
         case T_TRANSIT:
               BSON_APPEND_UTF8(pDocument, "type", TRANSIT);
         break;
         case T_POIS:
               BSON_APPEND_UTF8(pDocument, "type", POIS);
         break;
         case T_WATER:
               BSON_APPEND_UTF8(pDocument, "type", WATER);
         break;
         case T_ROAD:
               BSON_APPEND_UTF8(pDocument, "type", ROAD);
         break;
         case T_EARTH:
               BSON_APPEND_UTF8(pDocument, "type", EARTH);
         break;
         case T_LANDUSE:
               BSON_APPEND_UTF8(pDocument, "type", LANDUSE);
         break;
         case T_GEOFENCE:
               BSON_APPEND_UTF8(pDocument, "type", GEOFENCE);
         break;
         case T_DATA:
               BSON_APPEND_UTF8(pDocument, "type", DATA);
         break;
         default:
               BSON_APPEND_UTF8(pDocument, "type", NONE);
    }

    bson_append_bool (pDocument, "is_numeric", -1, pCell->is_numeric);

    if(pCell->pValue != NULL) {
          BSON_APPEND_UTF8(pDocument, "value", pCell->pValue);
    }

    // TODO timestamp!
    // time_t rawtime;
    // time(&rawtime);
    // struct tm *created = localtime(&rawtime);
    // BSON_APPEND_DATE_TIME (document, "created", mktime (created) * 1000);

    bson_error_t error;

    if (!mongoc_collection_insert (pCollection, MONGOC_INSERT_NONE, pDocument, NULL, &error)) {
       xbuf_xcat(pReply, "API Error : %s", error.message);
    } else {
      char *json = bson_as_json(pDocument, NULL);
      // return complete json
      xbuf_xcat(pReply, json, sizeof(json) - 1);
      bson_free(json);
    }

    bson_destroy(pDocument);
}
