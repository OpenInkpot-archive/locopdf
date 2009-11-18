#ifndef PLUGIN_H
#define PLUGIN_H

#include <Ecore.h>
#include "locopdf.h"

struct _plugin_ops_t {
    loco_document(*document_new) (const char *filename);
    void (*document_delete) (loco_document document);
    int (*document_page_count_get) (const loco_document document);
    unsigned char (*document_is_locked) (const loco_document doc);
    unsigned char (*document_unlock) (loco_document doc,
                                          const char *password);
    char *(*document_title_get) (const loco_document doc);
    char *(*document_author_get) (const loco_document doc);
    char *(*document_subject_get) (const loco_document doc);
    char *(*document_keywords_get) (const loco_document doc);
    char *(*document_creator_get) (const loco_document doc);
    char *(*document_producer_get) (const loco_document doc);
    char *(*document_creation_date_get) (const loco_document doc);
    char *(*document_mod_date_get) (const loco_document doc);
     loco_index_item(*index_item_new) ();
    const char *(*index_item_title_get) (const loco_index_item item);
    Ecore_List *(*index_item_children_get) (const loco_index_item
                                                item);
     loco_link_action_kind(*index_item_action_kind_get) (const
                                                             loco_index_item
                                                             item);
    int (*index_item_page_get) (const loco_document document,
                                    const loco_index_item item);
    Ecore_List *(*index_new) (const loco_document document);
    void (*index_delete) (Ecore_List * index);
     loco_page(*page_new) (const loco_document doc);
    void (*page_delete) (loco_page page);
    void (*page_render) (loco_page page, Evas_Object * o);
    void (*page_render_slice) (loco_page page,
                                   Evas_Object * o, int x, int y, int w,
                                   int h);
    void (*page_page_set) (loco_page page, int p);
    int (*page_page_get) (const loco_page page);
    void (*page_size_get) (const loco_page page, int *width,
                               int *height);
    void (*page_content_geometry_get) (const loco_page page, int *x,
                                           int *y, int *width,
                                           int *height);
    void (*page_scale_set) (loco_page page, double hscale,
                                double vscale);
    void (*page_scale_get) (const loco_page page, double *hscale,
                                double *vscale);
    int (*fonts_antialias_get) (void);
    void (*fonts_antialias_set) (int on);
    int (*lines_antialias_get) (void);
    void (*lines_antialias_set) (int on);
};

#endif                          /* PLUGIN_H */
