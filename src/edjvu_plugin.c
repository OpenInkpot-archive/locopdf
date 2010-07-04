#include <edjvu/Edjvu.h>
#include <Eina.h>
#include "plugin.h"

static plugin_ops_t plugin_ops;

loco_document
djvu_document_new(const char *filename)
{
    return (loco_document) edjvu_document_new(filename);
}

void
djvu_document_delete(loco_document document)
{
    edjvu_document_delete(document);
}

int
djvu_document_page_count_get(const loco_document document)
{
    return edjvu_document_page_count_get(document);
}

/*
unsigned char
djvu_document_is_locked(const loco_document doc)
{
    return edjvu_document_is_locked(doc);
}

unsigned char
djvu_document_unlock(loco_document doc, const char *password)
{
    return edjvu_document_unlock(doc, password);
}

char *
djvu_document_title_get(const loco_document doc)
{
    return edjvu_document_title_get(doc);
}

char *
djvu_document_author_get(const loco_document doc)
{
    return edjvu_document_author_get(doc);
}

char *
djvu_document_subject_get(const loco_document doc)
{
    return edjvu_document_subject_get(doc);
}

char *
djvu_document_keywords_get(const loco_document doc)
{
    return edjvu_document_keywords_get(doc);
}

char *
djvu_document_creator_get(const loco_document doc)
{
    return edjvu_document_creator_get(doc);
}

char *
djvu_document_producer_get(const loco_document doc)
{
    return edjvu_document_producer_get(doc);
}

char *
djvu_document_creation_date_get(const loco_document doc)
{
    return edjvu_document_creation_date_get(doc);
}

char *
djvu_document_mod_date_get(const loco_document doc)
{
    return edjvu_document_mod_date_get(doc);
}

*/

loco_index_item
djvu_index_item_new()
{
    return edjvu_index_item_new();
}

const char *
djvu_index_item_title_get(const loco_index_item item)
{
    return edjvu_index_item_title_get(item);
}

Eina_List *
djvu_index_item_children_get(const loco_index_item item)
{
    return edjvu_index_item_children_get(item);
}

/*
loco_link_action_kind
djvu_index_item_action_kind_get(const loco_index_item item)
{
    return edjvu_index_item_action_kind_get(item);
}
*/

int
djvu_index_item_page_get(const loco_document document,
                        const loco_index_item item)
{
    return edjvu_index_item_page_get(document, item);
}

Eina_List *
djvu_index_new(const loco_document document)
{
    return edjvu_index_new(document);
}

void
djvu_index_delete(Eina_List * index)
{
    edjvu_index_delete(index);
}

loco_page
djvu_page_new(const loco_document doc)
{
    return edjvu_page_new(doc);
}

void
djvu_page_delete(loco_page page)
{
    edjvu_page_delete(page);
}

void
djvu_page_render(loco_page page, Evas_Object * o)
{
    edjvu_page_render(page, o);
}

void
djvu_page_render_slice(loco_page page,
                      Evas_Object * o, int x, int y, int w, int h)
{
    edjvu_page_render_slice(page, o, x, y, w, h);
}

void
djvu_page_page_set(loco_page page, int p)
{
    edjvu_page_page_set(page, p);
}

int
djvu_page_page_get(const loco_page page)
{
    return edjvu_page_page_get(page);
}

void
djvu_page_size_get(const loco_page page, int *width, int *height)
{
    edjvu_page_size_get(page, width, height);
}

void
djvu_page_content_geometry_get(const loco_page page, int *x, int *y,
                              int *width, int *height)
{
    edjvu_page_content_geometry_get(page, x, y, width, height);
}

void
djvu_page_scale_set(loco_page page, double hscale, double vscale)
{
    return edjvu_page_scale_set(page, hscale, vscale);
}

void
djvu_page_scale_get(const loco_page page, double *hscale, double *vscale)
{
    return edjvu_page_scale_get(page, hscale, vscale);
}

/*
int
djvu_fonts_antialias_get(void)
{
    return edjvu_fonts_antialias_get();
}

void
djvu_fonts_antialias_set(int on)
{
    edjvu_fonts_antialias_set(on);
}

int
djvu_lines_antialias_get(void)
{
    return edjvu_lines_antialias_get();
}

void
djvu_lines_antialias_set(int on)
{
    edjvu_lines_antialias_set(on);
}
*/

plugin_ops_t *
djvu_init()
{
    plugin_ops.document_new = djvu_document_new;
    plugin_ops.document_delete = djvu_document_delete;
    plugin_ops.document_page_count_get = djvu_document_page_count_get;
    plugin_ops.document_is_locked = NULL; //djvu_document_is_locked;
    plugin_ops.document_unlock = NULL; // djvu_document_unlock;
    plugin_ops.document_title_get = NULL; //djvu_document_title_get;
    plugin_ops.document_author_get = NULL; //djvu_document_author_get;
    plugin_ops.document_subject_get = NULL; //djvu_document_subject_get;
    plugin_ops.document_keywords_get = NULL; //djvu_document_keywords_get;
    plugin_ops.document_creator_get = NULL; //djvu_document_creator_get;
    plugin_ops.document_producer_get = NULL; //djvu_document_producer_get;
    plugin_ops.document_creation_date_get = NULL; //djvu_document_creation_date_get;
    plugin_ops.document_mod_date_get = NULL; //djvu_document_mod_date_get;
    plugin_ops.index_item_new = djvu_index_item_new;
    plugin_ops.index_item_title_get = djvu_index_item_title_get;
    plugin_ops.index_item_children_get = djvu_index_item_children_get;
    plugin_ops.index_item_action_kind_get = NULL; //djvu_index_item_action_kind_get;
    plugin_ops.index_item_page_get = djvu_index_item_page_get;
    plugin_ops.index_new = djvu_index_new;
    plugin_ops.index_delete = djvu_index_delete;
    plugin_ops.fonts_antialias_get = NULL; //djvu_fonts_antialias_get;
    plugin_ops.fonts_antialias_set = NULL; //djvu_fonts_antialias_set;
    plugin_ops.lines_antialias_get = NULL; //djvu_lines_antialias_get;
    plugin_ops.lines_antialias_set = NULL; //djvu_lines_antialias_set;
    plugin_ops.page_new = djvu_page_new;
    plugin_ops.page_delete = djvu_page_delete;
    plugin_ops.page_render = djvu_page_render;
    plugin_ops.page_render_slice = djvu_page_render_slice;
    plugin_ops.page_page_set = djvu_page_page_set;
    plugin_ops.page_page_get = djvu_page_page_get;
    plugin_ops.page_size_get = djvu_page_size_get;
    plugin_ops.page_content_geometry_get = djvu_page_content_geometry_get;
    plugin_ops.page_scale_set = djvu_page_scale_set;
    plugin_ops.page_scale_get = djvu_page_scale_get;

    return &plugin_ops;
}

void
djvu_fini()
{
}
