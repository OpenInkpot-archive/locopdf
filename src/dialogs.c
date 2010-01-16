/***************************************************************************
 *   Copyright (C) 2008 by Marc Lajoie                                     *
 *   quickhand@openinkpot.org                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <libintl.h>

#include "dialogs.h"
#include "locopdf.h"
#include "plugin.h"

#include <Edje.h>
#include <libchoicebox.h>
#include <libeoi.h>

#include "choices.h"
#include "entry.h"

#define _(x) x

#define ASIZE(x) sizeof((x))/sizeof((x)[0])

#define GETTITLE \
    edje_object_part_text_get(evas_object_name_find(canvas, "main_choicebox_edje"), "title");
#define SETTITLE(x) \
    edje_object_part_text_set(evas_object_name_find(canvas, "main_choicebox_edje"), "title", (x));

void FitModeDialog(Evas *canvas, Evas_Object *parent, int item_num);

extern plugin_ops_t *loco_ops;

typedef struct menu_item_t {
    void (*draw) (Evas_Object *self);
    void (*select) (Evas_Object *self, int item_num);
    void *arg;
} menu_item_t;

typedef struct parent_t {
    Evas_Object *parent;
    int item_num;
    const char *prev_title;
} parent_t;

const char *fit_strings[] = {
    _("Fit Text Width"),
    _("Fit Width"),
    _("Fit Height"),
    _("Best Fit"),
    _("Stretch Fit"),
    _("No Fit"),
};

static bool refresh = false;

void goto_entry_handler(Evas_Object* entry,
        long num,
        void* param)
{
    if(param)
        free(param);

    if(num > 0 && num <= get_num_pages())
        goto_page(num - 1);
}

void
GotoPageEntry(Evas *e, Evas_Object *obj)
{
    char *tempstr;
    asprintf(&tempstr, "%s (%d/%d) ",
            gettext("Go To Page"),
            get_cur_page() + 1,
            get_num_pages());

    entry_n(obj, e,
            goto_entry_handler, "goto-page-entry",
            tempstr, tempstr);
}

// Options dialogs
void hpan_draw(Evas_Object *self)
{
    char *c;
    asprintf(&c, "%d%%", (int)(get_hpan_inc() * 100));
    if(c) {
        edje_object_part_text_set(self, "value", c);
        free(c);
    } else
        edje_object_part_text_set(self, "value", "");

    edje_object_part_text_set(self, "title", gettext("Horizontal Panning"));
}

void hpan_entry_handler(Evas_Object* entry,
        long num,
        void* param)
{
    if(num <= 0 || num > 100)
        return;

    set_hpan_inc(1.0 * num / 100);

    parent_t *p = (parent_t*)param;
    choicebox_invalidate_item(p->parent, p->item_num);
    free(p);
}

void hpan_select(Evas_Object *self, int item_num)
{
    parent_t *p = (parent_t*)malloc(sizeof(parent_t));
    p->parent = self;
    p->item_num = item_num;

    entry_n(self, evas_object_evas_get(self),
            hpan_entry_handler, "hpan-entry",
            gettext("Horizontal Panning"), p);
}

void vpan_entry_handler(Evas_Object* entry,
        long num,
        void* param)
{
    if(num <= 0 || num > 100)
        return;

    set_vpan_inc(1.0 * num / 100);

    parent_t *p = (parent_t*)param;
    choicebox_invalidate_item(p->parent, p->item_num);
    free(p);
}

void vpan_draw(Evas_Object *self)
{
    char *c;
    asprintf(&c, "%d%%", (int)(get_vpan_inc() * 100));
    if(c) {
        edje_object_part_text_set(self, "value", c);
        free(c);
    } else
        edje_object_part_text_set(self, "value", "");

    edje_object_part_text_set(self, "title", gettext("Vertical Panning"));
}

void vpan_select(Evas_Object *self, int item_num)
{
    parent_t *p = (parent_t*)malloc(sizeof(parent_t));
    p->parent = self;
    p->item_num = item_num;

    entry_n(self, evas_object_evas_get(self),
            vpan_entry_handler, "vpan-entry",
            gettext("Vertical Panning"), p);
}

void zoom_entry_handler(Evas_Object* entry,
        long num,
        void* param)
{
    if(num <= 0)
        return;

    set_zoom(1.0 * num / 100);

    if(param) {
        refresh = true;

        parent_t *p = (parent_t*)param;
        choicebox_invalidate_item(p->parent, p->item_num);
        free(p);
    }
}

void zoom_entry_handler_2(Evas_Object* entry,
        long num,
        void* param)
{
    if(num <= 0)
        return;

    set_zoom(1.0 * num / 100);

    if(param)
        free(param);

    render_cur_page();
}

void zoom_draw(Evas_Object *self)
{
    char *c;
    asprintf(&c, "%d%%", (int)(get_zoom() * 100));
    if(c) {
        edje_object_part_text_set(self, "value", c);
        free(c);
    } else
        edje_object_part_text_set(self, "value", "");

    edje_object_part_text_set(self, "title", gettext("Zoom"));
}

void zoom_select(Evas_Object *self, int item_num)
{
    parent_t *p = (parent_t*)malloc(sizeof(parent_t));
    p->parent = self;
    p->item_num = item_num;

    entry_n(self, evas_object_evas_get(self),
            zoom_entry_handler, "zoom-entry",
            gettext("Zoom"), p);
}

void zoom_entry(Evas *canvas)
{
    char *tempstr;
    asprintf(&tempstr, "%s (%d%%) ",
            gettext("Zoom"),
            (int)(get_zoom() * 100));

    entry_n(NULL, canvas,
            zoom_entry_handler_2, "zoom-entry",
            tempstr, tempstr);
}

void zoom_inc_entry_handler(Evas_Object* entry,
        long num,
        void* param)
{
    if(num <= 0)
        return;

    set_zoom_inc(1.0 * num / 100);

    parent_t *p = (parent_t*)param;
    choicebox_invalidate_item(p->parent, p->item_num);
    free(p);
}

void zinc_draw(Evas_Object *self)
{
    char *c;
    asprintf(&c, "%d%%", (int)(get_zoom_inc() * 100));
    if(c) {
        edje_object_part_text_set(self, "value", c);
        free(c);
    } else
        edje_object_part_text_set(self, "value", "");

    edje_object_part_text_set(self, "title", gettext("Zoom Increment"));
}

void zinc_select(Evas_Object *self, int item_num)
{
    parent_t *p = (parent_t*)malloc(sizeof(parent_t));
    p->parent = self;
    p->item_num = item_num;

    entry_n(self, evas_object_evas_get(self),
            zoom_inc_entry_handler, "zoom-inc-entry",
            gettext("Zoom Increment"), p);
}

void fit_draw(Evas_Object *self)
{
    char *c = strdup(gettext(fit_strings[get_fit_mode()]));
    if(c) {
        edje_object_part_text_set(self, "value", c);
        free(c);
    } else
        edje_object_part_text_set(self, "value", "");

    edje_object_part_text_set(self, "title", gettext("Fit Mode"));
}

void fit_select(Evas_Object *self, int item_num)
{
    FitModeDialog(evas_object_evas_get(self), self, item_num);
}

struct menu_item_t preferences_menu_items[] = {
    {&hpan_draw, &hpan_select, 0},
    {&vpan_draw, &vpan_select, 0},
    {&zoom_draw, &zoom_select, 0},
    {&zinc_draw, &zinc_select, 0},
    {&fit_draw, &fit_select, 0},
};

static void preferences_draw_handler(Evas_Object* choicebox __attribute__((unused)),
                         Evas_Object* item,
                         int item_num,
                         int page_position __attribute__((unused)),
                         void* param __attribute__((unused)))
{
    preferences_menu_items[item_num].draw(item);
}

static
void preferences_handler(Evas_Object* choicebox __attribute__((unused)),
                    int item_num,
                    bool is_alt __attribute__((unused)),
                    void* param)
{
    preferences_menu_items[item_num].select(choicebox, item_num);
}

void preferences_close_handler(Evas_Object* choicebox,
        void* param)
{
    choicebox_pop(choicebox);

    if(!refresh)
        return;

    render_cur_page();

    refresh = false;
}

void
PreferencesDialog(Evas *canvas, Evas_Object *parent)
{
	choicebox_push(NULL, canvas,
        preferences_handler, preferences_draw_handler, preferences_close_handler, "settings", ASIZE(preferences_menu_items), 0, NULL);

    SETTITLE(gettext("Settings"));
}

// fitmode choicebox

static void fit_mode_draw_handler(Evas_Object* choicebox __attribute__((unused)),
                         Evas_Object* item,
                         int item_num,
                         int page_position __attribute__((unused)),
                         void* param __attribute__((unused)))
{
	edje_object_part_text_set(item, "text", gettext(fit_strings[item_num]));
}

static
void fit_mode_handler(Evas_Object* choicebox __attribute__((unused)),
                    int item_num,
                    bool is_alt __attribute__((unused)),
                    void* param)
{
    parent_t *p = (parent_t*)param;

    if (get_fit_mode() != item_num) {
        set_fit_mode(item_num);

		choicebox_invalidate_item(p->parent, p->item_num);
		choicebox_pop(choicebox);

        if(p->prev_title) {
            Evas *canvas = evas_object_evas_get(choicebox);
            SETTITLE(p->prev_title);
        }
        free(p);

        refresh = true;
    }
}

static void fit_close_handler(Evas_Object* choicebox __attribute__((unused)),
                          void *param __attribute__((unused)))
{
    parent_t *p = (parent_t*)param;
    if(p) {
        if(p->prev_title) {
            Evas *canvas = evas_object_evas_get(choicebox);
            SETTITLE(p->prev_title);
        }
        free(p);
    }

    choicebox_pop(choicebox);
}

void
FitModeDialog(Evas *canvas, Evas_Object *parent, int item_num)
{
    parent_t *p = (parent_t*)malloc(sizeof(parent_t));
    p->parent = parent;
    p->item_num = item_num;
    p->prev_title = GETTITLE;
	choicebox_push(parent, canvas,
        fit_mode_handler, fit_mode_draw_handler, fit_close_handler, "fit-mode", ASIZE(fit_strings), 1, p);

    SETTITLE(gettext("Fit Mode"));
}

// TOC Choicebox

typedef struct toc_items_t toc_items_t;
struct toc_items_t {
    loco_index_item curitem;
    Ecore_List *l;
    toc_items_t *prev;
};

#define TITLE(x) loco_ops->index_item_title_get((loco_index_item)(x))

static void toc_draw_handler(Evas_Object* choicebox __attribute__((unused)),
                         Evas_Object* item,
                         int item_num,
                         int page_position __attribute__((unused)),
                         void* param __attribute__((unused)))
{
    toc_items_t *toc = (toc_items_t*)evas_object_data_get(choicebox, "toc-items");

    char *s;

    if(toc->curitem && item_num < 2) {
        if(item_num == 0)
            s = strdup("..");
        else if(item_num == 1)
            s = strdup(TITLE(toc->curitem));
    } else {
        if(toc->curitem)
            item_num -= 2;

        Ecore_List *l = ecore_list_index_goto(toc->l, item_num);
        Ecore_List *cl = loco_ops->index_item_children_get(l);

        if(cl && !ecore_list_empty_is(cl))
            asprintf(&s, "+ %s", TITLE(l));
        else
            s = strdup(TITLE(l));
    }

    edje_object_part_text_set(item, "text", s);
    if(s)
        free(s);
}

static
void toc_handler(Evas_Object* choicebox __attribute__((unused)),
                    int item_num,
                    bool is_alt __attribute__((unused)),
                    void* param)
{
    toc_items_t *toc = (toc_items_t*)evas_object_data_get(choicebox, "toc-items");

    if(toc->curitem && item_num < 2) {
        if(item_num == 0) {
            toc_items_t *tmp = toc;
            toc = toc->prev;
            free(tmp);

            int cnt = ecore_list_count(toc->l);
            if(toc->curitem)
                cnt += 2;

            evas_object_data_set(choicebox, "toc-items", toc);
            choicebox_set_size(choicebox, cnt);
            choicebox_invalidate_interval(choicebox, 0, cnt);
            if(cnt > 0) {
                choicebox_scroll_to(choicebox, 0);
                choicebox_set_selection(choicebox, -1);
            }
        } else if(item_num == 1) {
            choicebox_pop(choicebox);
            goto_page(loco_ops->index_item_page_get(get_document(), toc->curitem));
        }
    } else {
        if(toc->curitem)
            item_num -= 2;

        loco_index_item curitem =
            (loco_index_item) ecore_list_index_goto(toc->l, item_num);
        Ecore_List *childlist = loco_ops->index_item_children_get(curitem);

        if (!childlist) {
            choicebox_pop(choicebox);
            goto_page(loco_ops->index_item_page_get(get_document(), curitem));
        } else {
            toc_items_t *tmp = toc;
            toc = (toc_items_t*)malloc(sizeof(toc_items_t));

            toc->prev = tmp;
            toc->curitem = curitem;
            toc->l = childlist;

            int cnt = 2 + ecore_list_count(toc->l);
            evas_object_data_set(choicebox, "toc-items", toc);
            choicebox_set_size(choicebox, cnt);
            choicebox_invalidate_interval(choicebox, 0, cnt);
            if(cnt > 0) {
                choicebox_scroll_to(choicebox, 0);
                choicebox_set_selection(choicebox, -1);
            }
        }
    }
}

static void toc_close_handler(Evas_Object* choicebox __attribute__((unused)),
                          void *param __attribute__((unused)))
{
    toc_items_t *toc, *prev;
    toc = (toc_items_t*)evas_object_data_get(choicebox, "toc-items");
    while(toc) {
        prev = toc->prev;
        free(toc);
        toc = prev;
    }

    choicebox_pop(choicebox);
}

void
TOCDialog(Evas *canvas, Evas_Object *parent, Ecore_List *list)
{
    toc_items_t *toc = (toc_items_t*)malloc(sizeof(toc_items_t));

    toc->prev = NULL;
    toc->curitem = NULL;
    toc->l = list;

	Evas_Object *choicebox = choicebox_push(NULL, canvas,
        toc_handler, toc_draw_handler, toc_close_handler, "toc-choicebox", ecore_list_count(list), 1, NULL);

    evas_object_data_set(choicebox, "toc-items", toc);

    SETTITLE(gettext("Table Of Contents"));
}
