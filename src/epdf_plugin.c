#include <epdf/Epdf.h>
#include <Ecore.h>
#include "plugin.h"

static plugin_ops_t plugin_ops;

loco_document
pdf_document_new(const char *filename)
{
    return (loco_document) epdf_document_new(filename);
}

void
pdf_document_delete(loco_document document)
{
    epdf_document_delete(document);
}

int
pdf_document_page_count_get(const loco_document document)
{
    return epdf_document_page_count_get(document);
}

unsigned char
pdf_document_is_locked(const loco_document doc)
{
    return epdf_document_is_locked(doc);
}

unsigned char
pdf_document_unlock(loco_document doc, const char *password)
{
    return epdf_document_unlock(doc, password);
}

char *
pdf_document_title_get(const loco_document doc)
{
    return epdf_document_title_get(doc);
}

char *
pdf_document_author_get(const loco_document doc)
{
    return epdf_document_author_get(doc);
}

char *
pdf_document_subject_get(const loco_document doc)
{
    return epdf_document_subject_get(doc);
}

char *
pdf_document_keywords_get(const loco_document doc)
{
    return epdf_document_keywords_get(doc);
}

char *
pdf_document_creator_get(const loco_document doc)
{
    return epdf_document_creator_get(doc);
}

char *
pdf_document_producer_get(const loco_document doc)
{
    return epdf_document_producer_get(doc);
}

char *
pdf_document_creation_date_get(const loco_document doc)
{
    return epdf_document_creation_date_get(doc);
}

char *
pdf_document_mod_date_get(const loco_document doc)
{
    return epdf_document_mod_date_get(doc);
}

loco_index_item
pdf_index_item_new()
{
    return epdf_index_item_new();
}

const char *
pdf_index_item_title_get(const loco_index_item item)
{
    return epdf_index_item_title_get(item);
}

Ecore_List *
pdf_index_item_children_get(const loco_index_item item)
{
    return epdf_index_item_children_get(item);
}

loco_link_action_kind
pdf_index_item_action_kind_get(const loco_index_item item)
{
    return epdf_index_item_action_kind_get(item);
}

int
pdf_index_item_page_get(const loco_document document,
                        const loco_index_item item)
{
    return epdf_index_item_page_get(document, item);
}

Ecore_List *
pdf_index_new(const loco_document document)
{
    return epdf_index_new(document);
}

void
pdf_index_delete(Ecore_List * index)
{
    epdf_index_delete(index);
}

loco_page
pdf_page_new(const loco_document doc)
{
    return epdf_page_new(doc);
}

void
pdf_page_delete(loco_page page)
{
    epdf_page_delete(page);
}

void
pdf_page_render(loco_page page, Evas_Object * o)
{
    epdf_page_render(page, o);
}

void
pdf_page_render_slice(loco_page page,
                      Evas_Object * o, int x, int y, int w, int h)
{
    epdf_page_render_slice(page, o, x, y, w, h);
}

void
pdf_page_page_set(loco_page page, int p)
{
    epdf_page_page_set(page, p);
}

int
pdf_page_page_get(const loco_page page)
{
    return epdf_page_page_get(page);
}

void
pdf_page_size_get(const loco_page page, int *width, int *height)
{
    epdf_page_size_get(page, width, height);
}

void
pdf_page_content_geometry_get(const loco_page page, int *x, int *y,
                              int *width, int *height)
{
    epdf_page_content_geometry_get(page, x, y, width, height);
}

void
pdf_page_scale_set(loco_page page, double hscale, double vscale)
{
    return epdf_page_scale_set(page, hscale, vscale);
}

void
pdf_page_scale_get(const loco_page page, double *hscale, double *vscale)
{
    return epdf_page_scale_get(page, hscale, vscale);
}

int
pdf_fonts_antialias_get(void)
{
    return epdf_fonts_antialias_get();
}

void
pdf_fonts_antialias_set(int on)
{
    epdf_fonts_antialias_set(on);
}

int
pdf_lines_antialias_get(void)
{
    return epdf_lines_antialias_get();
}

void
pdf_lines_antialias_set(int on)
{
    epdf_lines_antialias_set(on);
}

plugin_ops_t *
pdf_init()
{
    plugin_ops.document_new = pdf_document_new;
    plugin_ops.document_delete = pdf_document_delete;
    plugin_ops.document_page_count_get = pdf_document_page_count_get;
    plugin_ops.document_is_locked = pdf_document_is_locked;
    plugin_ops.document_unlock = pdf_document_unlock;
    plugin_ops.document_title_get = pdf_document_title_get;
    plugin_ops.document_author_get = pdf_document_author_get;
    plugin_ops.document_subject_get = pdf_document_subject_get;
    plugin_ops.document_keywords_get = pdf_document_keywords_get;
    plugin_ops.document_creator_get = pdf_document_creator_get;
    plugin_ops.document_producer_get = pdf_document_producer_get;
    plugin_ops.document_creation_date_get = pdf_document_creation_date_get;
    plugin_ops.document_mod_date_get = pdf_document_mod_date_get;
    plugin_ops.index_item_new = pdf_index_item_new;
    plugin_ops.index_item_title_get = pdf_index_item_title_get;
    plugin_ops.index_item_children_get = pdf_index_item_children_get;
    plugin_ops.index_item_action_kind_get = pdf_index_item_action_kind_get;
    plugin_ops.index_item_page_get = pdf_index_item_page_get;
    plugin_ops.index_new = pdf_index_new;
    plugin_ops.index_delete = pdf_index_delete;
    plugin_ops.fonts_antialias_get = pdf_fonts_antialias_get;
    plugin_ops.fonts_antialias_set = pdf_fonts_antialias_set;
    plugin_ops.lines_antialias_get = pdf_lines_antialias_get;
    plugin_ops.lines_antialias_set = pdf_lines_antialias_set;
    plugin_ops.page_new = pdf_page_new;
    plugin_ops.page_delete = pdf_page_delete;
    plugin_ops.page_render = pdf_page_render;
    plugin_ops.page_render_slice = pdf_page_render_slice;
    plugin_ops.page_page_set = pdf_page_page_set;
    plugin_ops.page_page_get = pdf_page_page_get;
    plugin_ops.page_size_get = pdf_page_size_get;
    plugin_ops.page_content_geometry_get = pdf_page_content_geometry_get;
    plugin_ops.page_scale_set = pdf_page_scale_set;
    plugin_ops.page_scale_get = pdf_page_scale_get;

    return &plugin_ops;
}

void
pdf_fini()
{
}
