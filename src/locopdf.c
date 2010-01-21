/***************************************************************************
 *   Copyright (C) 2008 by Marc Lajoie                                     *
 *   quickhand@openinkpot.org                                              *
 *   Copyright (C) 2009 Alexander Kerner <lunohod@openinkpot.org>          *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <err.h>
#include <libintl.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Con.h>
#include <Ecore_File.h>
#include <Efreet_Mime.h>
#include <Edje.h>
#include <Evas.h>

#include <libkeys.h>

#include <Ecore_Con.h>
#include "dialogs.h"
#include "help.h"
#include "locopdf.h"
#include "database.h"

#include <xcb/xcb.h>
#include "info_export.h"

#include "plugin.h"
#include "epdf_plugin.h"
#include "edjvu_plugin.h"

/* Uzhosnakh */

static keys_t *keys;

pthread_t thread;

Evas *evas;
loco_document document;
loco_page page;
Ecore_List *pdf_index;
char *filename;
int dbres;
pthread_mutex_t pdf_renderer_mutex = PTHREAD_MUTEX_INITIALIZER;

int numpages;
int curpdfobj = 1;
int prerendering = 0;
int fitmode = FIT_TEXT_WIDTH;
double zoom = 1.0;
double zoominc = 0.1;
double hpaninc = 0.5;
double vpaninc = 0.9;

int lefttrim = 0;
int righttrim = 0;
int toptrim = 0;
int bottomtrim = 0;

int panx = 0;
int pany = 0;
int winwidth = 600;
int winheight = 800;

plugin_ops_t *loco_ops = NULL;

typedef struct {
    int page;

    // page
    int px, py;
    unsigned pw, ph;
    double pscale;

    // vieport
    int x, y;
    unsigned w, h;
} tile_t;

typedef enum {
    CUR = 0,
    PREV,
    NEXT,
    RIGHT,
    LEFT,
} tile_orientation_t;

tile_t cur_tile, next_tile;

static void render_tile(tile_t tile);
static void flip_pages();

static tile_t
get_tile(tile_t curtile, tile_orientation_t orient)
{
    int x, y, cwidth, cheight, width, height;
    int _lefttrim = 0, _righttrim = 0, _toptrim = 0, _bottomtrim = 0;

    tile_t tile = {
        .page = -1,
        .px = 0,
        .py = 0,
        .pw = 0,
        .ph = 0,
        .pscale = 1.0,
        .x = 0,
        .y = 0,
        .w = 0,
        .h = 0,
    };

    tile.page = -1;

    if (orient == PREV) {
        if (curtile.w > 0 && curtile.y > 0) {
            tile = curtile;
            tile.y -= (tile.h * vpaninc);
            if (tile.y < 0)
                tile.y = 0;

            return tile;
        }
    }

    if (orient == NEXT) {
        if (curtile.w > 0 && curtile.y + curtile.h < curtile.ph) {
            tile = curtile;
            if (0 && tile.y + tile.h * vpaninc > tile.ph - tile.h)
                tile.y = tile.ph - tile.h;
            else
                tile.y += (tile.h * vpaninc);

            return tile;
        }
    }

    if (orient == RIGHT) {
        if (curtile.w > 0 && curtile.x + curtile.w < curtile.pw) {
            tile = curtile;
            tile.x += (tile.w * hpaninc);
            if (tile.x + tile.w > tile.pw)
                tile.x = tile.pw - tile.w;
        }
        return tile;
    }

    if (orient == LEFT) {
        if (curtile.w > 0 && curtile.x > 0) {
            tile = curtile;
            tile.x -= (tile.w * hpaninc);
            if (tile.x < 0)
                tile.x = 0;
        }
        return tile;
    }

    do {

        if (orient == PREV)
            curtile.page--;
        if (orient == NEXT)
            curtile.page++;

        if (orient == PREV && curtile.page < 0)
            return tile;

        if ((orient == CUR || orient == NEXT)
            && curtile.page > (numpages - 1))
            return tile;

        if(loco_ops->page_page_set) loco_ops->page_page_set(page, curtile.page);
        if(loco_ops->page_size_get) loco_ops->page_size_get(page, &width, &height);
        if(loco_ops->page_content_geometry_get) loco_ops->page_content_geometry_get(page, &x, &y, &cwidth, &cheight);

        if (orient == CUR && (cwidth == 0 || cheight == 0))
            curtile.page++;

    } while (cwidth == 0 || cheight == 0);

    if (fitmode == FIT_TEXT_WIDTH && width != cwidth && height != cheight) {
        _lefttrim = x - 10;
        _righttrim = width - x - cwidth - 5;
        _bottomtrim = y - 10;
        _toptrim = height - y - cheight - 10;
    }

    tile.page = curtile.page;
    tile.x = curtile.x;
    tile.y = 0;
    tile.h = curtile.h;
    tile.w = curtile.w;

    // fix it!
    if (!tile.w)
        tile.w = get_win_width();

    // fix it!
    if (!tile.h)
        tile.h = get_win_height();

    double fitwidthzoom =
        ((double) tile.w) / ((double) (width - _lefttrim - _righttrim)) *
        zoom;
    double fitheightzoom =
        ((double) tile.h) / ((double) (height - _toptrim - _bottomtrim)) *
        zoom;

    double scalex;
    double scaley;

    if (fitmode == FIT_WIDTH || fitmode == FIT_TEXT_WIDTH) {
        scalex = fitwidthzoom;
        scaley = fitwidthzoom;
    } else if (fitmode == FIT_HEIGHT) {
        scalex = fitheightzoom;
        scaley = fitheightzoom;
    } else if (fitmode == FIT_BEST) {
        if (fitwidthzoom <= fitheightzoom) {
            scalex = fitwidthzoom;
            scaley = fitwidthzoom;
        } else {
            scalex = fitheightzoom;
            scaley = fitheightzoom;
        }

    } else if (fitmode == FIT_STRETCH) {
        scalex = fitwidthzoom;
        scaley = fitheightzoom;

    } else if (fitmode == FIT_NO) {
        scalex = 1.0;
        scaley = 1.0;

    } else {
        err(1, "Unknown fitmode passed to background renderer: %d",
            fitmode);
    }

    tile.pscale = scalex;

    if (!_lefttrim && !_righttrim && !_toptrim && !_bottomtrim) {
        tile.px = 0;
        tile.py = 0;
        tile.pw = width * scalex;
        tile.ph = height * scaley;
    } else {
        tile.px = (int) (((double) _lefttrim) * scalex);
        tile.py = (int) (((double) _toptrim) * scaley);
        tile.pw =
            (int) (((double) (width - _lefttrim - _righttrim)) * scalex);
        tile.ph =
            (int) (((double) (height - _toptrim - _bottomtrim)) * scaley);
    }

    if (orient == PREV) {
        tile.y = tile.ph - tile.h;
        if (tile.y < 0)
            tile.y = 0;
    }

    return tile;
}

static void
main_win_resize_handler(Ecore_Evas *main_win)
{
    int w, h;
    Evas *canvas = ecore_evas_get(main_win);
    evas_output_size_get(canvas, &w, &h);

    winwidth = w;
    winheight = h;

    cur_tile.w = w;
    cur_tile.h = h;

    Evas_Object *bg = evas_object_name_find(canvas, "background");
    evas_object_resize(bg, w, h);

    Evas_Object *choicebox = evas_object_name_find(canvas, "main_choicebox_edje");
    if(choicebox)
        evas_object_resize(choicebox, w, h);

    help_resize(canvas, w, h);

    render_cur_page();
}

static void
main_win_delete_handler(Ecore_Evas *main_win)
{
    ecore_main_loop_quit();
}

int
get_win_width()
{
    return winwidth;
}

int
get_win_height()
{
    return winheight;
}

void
set_zoom(double new_zoom)
{
    zoom = new_zoom;
}

double
get_zoom()
{
    return zoom;
}

double
get_zoom_inc()
{
    return zoominc;
}

void
set_zoom_inc(double newzoominc)
{
    zoominc = newzoominc;
}

double
get_hpan_inc()
{
    return hpaninc;
}

void
set_hpan_inc(double newhpaninc)
{
    hpaninc = newhpaninc;
}

double
get_vpan_inc()
{
    return vpaninc;
}

void
set_vpan_inc(double newvpaninc)
{
    vpaninc = newvpaninc;
}

int
get_lefttrim()
{
    return lefttrim;
}

void
set_lefttrim(int newlefttrim)
{
    lefttrim = newlefttrim;
}

int
get_righttrim()
{
    return righttrim;
}

void
set_righttrim(int newrighttrim)
{
    righttrim = newrighttrim;
}

int
get_toptrim()
{
    return toptrim;
}

void
set_toptrim(int newtoptrim)
{
    toptrim = newtoptrim;
}

int
get_bottomtrim()
{
    return bottomtrim;
}

void
set_bottomtrim(int newbottomtrim)
{
    bottomtrim = newbottomtrim;
}

int
get_fit_mode()
{
    return fitmode;
}

void
set_fit_mode(int newfitmode)
{
    fitmode = newfitmode;
}

int
get_antialias_mode()
{
    int ret = 0;

    if(loco_ops->fonts_antialias_get)
        ret = loco_ops->fonts_antialias_get();

    if(loco_ops->lines_antialias_get)
        ret += loco_ops->lines_antialias_get();

    return ret;
}

void
set_antialias_mode(int newantialiasmode)
{

    if(loco_ops->fonts_antialias_set)
        loco_ops->fonts_antialias_set(newantialiasmode ? 1 : 0);
    if(loco_ops->lines_antialias_set)
        loco_ops->lines_antialias_set(newantialiasmode ? 1 : 0);
}

int
get_num_pages()
{
    return numpages;
}

void
goto_page(int newpage)
{
    if(newpage < 0)
    	return;
    cur_tile.page = newpage;
    render_cur_page();
}

int
get_cur_page()
{
    return cur_tile.page;
}

loco_document
get_document()
{
    return document;
}

plugin_ops_t *get_ops()
{
    return loco_ops;
}

void
render_cur_page()
{
    pthread_mutex_lock(&pdf_renderer_mutex);
    cur_tile = get_tile(cur_tile, CUR);
    render_tile(cur_tile);
    flip_pages();
    pthread_mutex_unlock(&pdf_renderer_mutex);

    prerender_next_page();
}

static void
render_tile(tile_t tile)
{
    if (tile.page < 0 || tile.w < 0 || tile.h < 0)
        return;

    Evas_Object *pdfobj;

    if (curpdfobj == 1)
        pdfobj = evas_object_name_find(evas, "pdfobj2");
    else
        pdfobj = evas_object_name_find(evas, "pdfobj1");

    if(loco_ops->page_page_set) loco_ops->page_page_set(page, tile.page);
    if(loco_ops->page_scale_set) loco_ops->page_scale_set(page, tile.pscale, tile.pscale);

    if(loco_ops->page_render_slice) loco_ops->page_render_slice(page,
                           pdfobj,
                           tile.px + tile.x,
                           tile.py + tile.y, tile.w, tile.h);
}

void *
thread_func(void *vptr_args)
{
    pthread_mutex_lock(&pdf_renderer_mutex);

    next_tile = get_tile(cur_tile, NEXT);
    if (next_tile.page < 0) {
        pthread_mutex_unlock(&pdf_renderer_mutex);
        return NULL;
    }
    render_tile(next_tile);

    pthread_mutex_unlock(&pdf_renderer_mutex);

    return NULL;
}

int
are_legal_coords(int x1, int y1, int x2, int y2)
{

    int xs_in_range = ((x1 > 0 && x1 < get_win_width())
                       || (x2 > 0 && x2 < get_win_width()));
    int ys_in_range = ((y1 > 0 && y1 < get_win_height())
                       || (y2 > 0 && y2 < get_win_height()));
    int xs_opposite = (x1 <= 0 && x2 >= get_win_width());
    int ys_opposite = (y1 <= 0 && y2 >= get_win_height());
    if ((ys_in_range && xs_in_range) || (ys_in_range && xs_opposite)
        || (xs_in_range && ys_opposite) || (xs_opposite && ys_opposite))
        return 1;
    return 0;
}

void
ensure_thread_dead()
{
    if (prerendering)
        pthread_join(thread, NULL);
    prerendering = 0;
}

void
prerender_next_page()
{
    ensure_thread_dead();
    prerendering = 1;
    pthread_create(&thread, NULL, thread_func, NULL);
}

static void
flip_pages()
{
    Evas_Object *active, *inactive;
    if (curpdfobj == 1) {
        active = evas_object_name_find(evas, "pdfobj1");
        inactive = evas_object_name_find(evas, "pdfobj2");
        curpdfobj = 2;
    } else {
        active = evas_object_name_find(evas, "pdfobj2");
        inactive = evas_object_name_find(evas, "pdfobj1");
        curpdfobj = 1;
    }
    evas_object_hide(active);
    evas_object_show(inactive);
}

static void
next_page()
{
    ensure_thread_dead();
    if (next_tile.page < 0)
        return;
    cur_tile = next_tile;
    flip_pages();
    prerender_next_page();
}

static void
prev_page()
{
    pthread_mutex_lock(&pdf_renderer_mutex);
    tile_t t = get_tile(cur_tile, PREV);
    if (t.page < 0) {
        pthread_mutex_unlock(&pdf_renderer_mutex);
        return;
    }
    next_tile = cur_tile;
    cur_tile = t;
    render_tile(cur_tile);
    flip_pages();
    pthread_mutex_unlock(&pdf_renderer_mutex);
}

static void
pan_left()
{
    pthread_mutex_lock(&pdf_renderer_mutex);
    tile_t t = get_tile(cur_tile, LEFT);
    if (t.page < 0) {
        pthread_mutex_unlock(&pdf_renderer_mutex);
        return;
    }
    cur_tile = t;
    render_tile(cur_tile);
    flip_pages();
    pthread_mutex_unlock(&pdf_renderer_mutex);

    prerender_next_page();
}

static void
pan_right()
{
    pthread_mutex_lock(&pdf_renderer_mutex);
    tile_t t = get_tile(cur_tile, RIGHT);
    if (t.page < 0) {
        pthread_mutex_unlock(&pdf_renderer_mutex);
        return;
    }

    cur_tile = t;
    render_tile(cur_tile);
    flip_pages();
    pthread_mutex_unlock(&pdf_renderer_mutex);

    prerender_next_page();
}

/* GUI */

/* Main key handler */

static void
_quit()
{
    ecore_main_loop_quit();
}

static void
_settings(Evas *canvas)
{
    Evas_Object *bgobj = evas_object_name_find(canvas, "background");
    PreferencesDialog(evas, bgobj);
}

static void
_help(Evas *canvas)
{
    help_show(canvas);
}

static void
_zoom(Evas *canvas)
{
    zoom_entry(canvas);
}

static void
_zoom_in(Evas *canvas)
{
    zoom += zoominc;
    if (zoom > 5.0)
        zoom = 5.0;
    render_cur_page();
}

static void
_zoom_out(Evas *canvas)
{
    zoom -= zoominc;
    if (zoom < 0.2)
        zoom = 0.2;
    render_cur_page();
}

static void
_go_to_page(Evas *canvas)
{
    Evas_Object *bgobj = evas_object_name_find(canvas, "background");
    GotoPageEntry(canvas, bgobj);
}

static void
_toc(Evas *canvas)
{
    if (pdf_index) {
        Evas_Object *bgobj = evas_object_name_find(canvas, "background");
        TOCDialog(canvas, bgobj, pdf_index);
    }
}

void
reset_zoom_and_pan()
{
    zoom = 1.0;
    cur_tile.y = 0;
    cur_tile.x = 0;
    render_cur_page();
}

static void
_key_handler(void *data, Evas *canvas, Evas_Object *obj,
             void *event_info)
{
    Evas_Event_Key_Up *e = (Evas_Event_Key_Up *) event_info;
    const char *action = keys_lookup_by_event(keys, "main", e);
    if (!action)
        return;

    if (!strcmp(action, "Quit"))
        _quit();
    else if (!strcmp(action, "Settings"))
        _settings(canvas);
    else if (!strcmp(action, "Zoom"))
        _zoom(canvas);
    else if (!strcmp(action, "Help"))
        _help(canvas);
    else if (!strcmp(action, "PageUp") || !strcmp(action, "PanUp"))
        prev_page();
    else if (!strcmp(action, "PageDown") || !strcmp(action, "PanDown"))
        next_page();
    else if (!strcmp(action, "ZoomIn"))
        _zoom_in(canvas);
    else if (!strcmp(action, "ZoomOut"))
        _zoom_out(canvas);
    else if (!strcmp(action, "GoToPage"))
        _go_to_page(canvas);
    else if (!strcmp(action, "PanLeft"))
        pan_left();
    else if (!strcmp(action, "PanRight"))
        pan_right();
    else if (!strcmp(action, "ToC"))
        _toc(canvas);
    else if (!strcmp(action, "ResetView"))
        reset_zoom_and_pan();
}

void
save_global_settings(char *filename)
{
    begin_transaction();
    set_setting_INT(filename, "current_page", cur_tile.page);
    set_setting_DOUBLE(filename, "zoom_increment", zoominc);
    set_setting_DOUBLE(filename, "current_zoom", zoom);
    set_setting_DOUBLE(filename, "h_pan_increment", hpaninc);
    set_setting_DOUBLE(filename, "v_pan_increment", vpaninc);

    /*
       set_setting_INT(filename,"left_trim",lefttrim);
       set_setting_INT(filename,"right_trim",righttrim);
       set_setting_INT(filename,"top_trim",toptrim);
       set_setting_INT(filename,"bottom_trim",bottomtrim);
     */
    set_setting_INT(filename, "fit_mode", fitmode);
    commit_transaction();
}

void
restore_global_settings(char *filename)
{
    int temp11;                 //,temp12,temp13,temp14;
    double temp21, temp22;
    temp11 = get_setting_INT(filename, "current_page");
    if (temp11 >= 0)
        cur_tile.page = (int) temp11;
    else
        cur_tile.page = 0;

    temp21 = get_setting_DOUBLE(filename, "zoom_increment");
    temp22 = get_setting_DOUBLE(filename, "current_zoom");
    if (temp21 > 0)
        zoominc = temp21;
    else
        zoominc = 0.1;

    if (temp22 > 0)
        zoom = temp22;
    else
        zoom = 1.0;

    temp21 = get_setting_DOUBLE(filename, "h_pan_increment");
    temp22 = get_setting_DOUBLE(filename, "v_pan_increment");
    if (temp21 > 0)
        hpaninc = temp21;
    else
        hpaninc = 0.5;

    if (temp22 > 0)
        vpaninc = temp22;
    else
        vpaninc = 0.9;


    /*
       temp11=get_setting_INT(filename,"left_trim");
       temp12=get_setting_INT(filename,"right_trim");
       temp13=get_setting_INT(filename,"top_trim");
       temp14=get_setting_INT(filename,"bottom_trim");
       if(temp11>=0 && temp12>=0 && temp13>=0 && temp14>=0)
       {
       lefttrim=temp11;
       righttrim=temp12;
       toptrim=temp13;
       bottomtrim=temp14;

       }
     */
    temp11 = get_setting_INT(filename, "fit_mode");
    if (temp11 >= 0)
        fitmode = temp11;
    else
        fitmode = FIT_TEXT_WIDTH;
}

static void
set_active_win_id(Ecore_Evas *ee)
{
    Ecore_X_Window root =
        ((xcb_screen_t *) ecore_x_default_screen_get())->root;
    Ecore_X_Window window = ecore_evas_software_x11_window_get(ee);

    wprop_set_active_win_id(root, window);
}


static void
set_properties(Ecore_Evas *ee)
{
    Ecore_X_Window window = ecore_evas_software_x11_window_get(ee);

    if(loco_ops->document_author_get)
        wprop_set_string(window, "ACTIVE_DOC_AUTHOR",
                         loco_ops->document_author_get(document));
    if(loco_ops->document_title_get)
        wprop_set_string(window, "ACTIVE_DOC_TITLE",
                         loco_ops->document_title_get(document));
    else
        wprop_set_string(window, "ACTIVE_DOC_TITLE",
                         ecore_file_file_get(filename));
    wprop_set_string(window, "ACTIVE_DOC_FILENAME",
                     ecore_file_file_get(filename));
    wprop_set_string(window, "ACTIVE_DOC_FILEPATH",
                     ecore_file_dir_get(filename));
}

static bool
open_file(const char *file)
{
    restore_global_settings((char *) file);

    const char *mime = efreet_mime_type_get(file);
    if(!mime)
        return false;
    if(!strcmp(mime, "image/vnd.djvu"))
        loco_ops = djvu_init();
    else if(!strcmp(mime, "application/pdf"))
        loco_ops = pdf_init();
    else
        return false;

    if(!loco_ops)
        return false;

    if(loco_ops->document_new)
        document = loco_ops->document_new(file);
    if (!document) {
        // manage error here
        fprintf(stderr, "Error Opening Document\n");
        return false;
    }

    if(loco_ops->document_page_count_get)
        numpages = loco_ops->document_page_count_get(document);
    if(loco_ops->page_new)
        page = loco_ops->page_new(document);
    if (!page) {
        // manage error here
        fprintf(stderr, "Error Processing Document");

        return false;
    }
    curpdfobj = 1;

    filename = strdup(file);

    if(loco_ops->index_new)
        pdf_index = loco_ops->index_new(document);

    set_antialias_mode(true);
    if (dbres != (-1)) {
        int am = get_setting_INT((char*)file, "antialias");
        if (am >= 0)
            set_antialias_mode(am);
    }

    render_cur_page();

    return true;
}

static void
close_file()
{
    ensure_thread_dead();

    if (dbres != (-1)) {
        save_global_settings(filename);
        Evas_Object *pdfobj;
        if (curpdfobj == 1)
            pdfobj = evas_object_name_find(evas, "pdfobj1");
        else
            pdfobj = evas_object_name_find(evas, "pdfobj2");
        int x, y, w, h;
        evas_object_geometry_get(pdfobj, &x, &y, &w, &h);
        set_setting_INT(filename, "current_x", x);
        set_setting_INT(filename, "current_y", y);
        set_setting_INT(filename, "antialias", get_antialias_mode());
    }
    if(loco_ops->index_delete) loco_ops->index_delete(pdf_index);
    if(loco_ops->page_delete) loco_ops->page_delete(page);
    if(loco_ops->document_delete) loco_ops->document_delete(document);

    free(filename);
    filename = NULL;
}

typedef struct {
    char *msg;
    int size;
} client_data_t;

static int
_client_add(void *param, int ev_type, void *ev)
{
    Ecore_Con_Event_Client_Add *e = ev;
    client_data_t *msg = malloc(sizeof(client_data_t));
    msg->msg = strdup("");
    msg->size = 0;
    ecore_con_client_data_set(e->client, msg);
    return 0;
}

static int
_client_del(void *param, int ev_type, void *ev)
{
    Ecore_Con_Event_Client_Del *e = ev;
    client_data_t *msg = ecore_con_client_data_get(e->client);

    /* Handle */

    if (!msg->msg[0]) {
        /* Skip it: ecore-con internal bug */
    } else if (msg->size < 1) {
        fprintf(stderr, "No filename was received\n");
    } else {
        Ecore_Evas *win = (Ecore_Evas *) param;

        char *oldfile = strdup(filename);

        close_file();

        if (!open_file(msg->msg))
            open_file(oldfile);

        if (oldfile)
            free(oldfile);

        set_properties(win);

        ecore_evas_show(win);
        ecore_evas_raise(win);
    }

    free(msg->msg);
    free(msg);
    return 0;
}

static int
_client_data(void *param, int ev_type, void *ev)
{
    Ecore_Con_Event_Client_Data *e = ev;
    client_data_t *msg = ecore_con_client_data_get(e->client);
    msg->msg = realloc(msg->msg, msg->size + e->size);
    memcpy(msg->msg + msg->size, e->data, e->size);
    msg->size += e->size;
    return 0;
}

static bool
check_running_instance(const char *file)
{
    if (!file)
        return true;

    Ecore_Con_Server *server
        =
        ecore_con_server_add(ECORE_CON_LOCAL_USER, "locopdf-singleton", 0,
                             NULL);

    if (!server) {
        /* Somebody already listens there */
        server = ecore_con_server_connect(ECORE_CON_LOCAL_USER,
                                          "locopdf-singleton", 0, NULL);

        if (!server)
            return false;

        char *buf = (char *) malloc(strlen(file) + 1);
        strcpy(buf, file);
        buf[strlen(file)] = '\0';
        ecore_con_server_send(server, buf, strlen(file) + 1);
        free(buf);
        ecore_con_server_flush(server);
        ecore_con_server_del(server);

        return true;
    }

    return false;
}

static void
usage()
{
    fprintf(stderr, "Usage: locopdf <pdf file>\n");
}

int
main(int argc, char *argv[])
{
    Ecore_Evas *ee;

    Evas_Object *bg, *o1, *o2;

    if (argc != 2) {
        usage();
        return 1;
    }

    ecore_init();
    ecore_con_init();

    if (check_running_instance(argv[1])) {
        ecore_con_shutdown();
        ecore_shutdown();
        return 0;
    }

    /* initialize our libraries */
    evas_init();
    ecore_evas_init();
    edje_init();

    efreet_mime_init();

    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "C");
    textdomain("locopdf");

    keys = keys_alloc("locopdf");

    /* setup database */

    const char *homedir = getenv("HOME");
    char *settingsdir;
    asprintf(&settingsdir, "%s/%s", homedir, ".locopdf/");
    if (!(ecore_file_exists(settingsdir)
          && ecore_file_is_dir(settingsdir))) {
        ecore_file_mkpath(settingsdir);
    }
    free(settingsdir);
    char *dbfile;
    asprintf(&dbfile, "%s/%s", homedir, ".locopdf/files.db");
    dbres = init_database(dbfile);
    free(dbfile);
    if (dbres != (-1))
        restore_global_settings(argv[1]);

    /* create our Ecore_Evas and show it */
    ee = ecore_evas_software_x11_new(0, 0, 0, 0, 600, 800);

    ecore_evas_borderless_set(ee, 0);
    ecore_evas_shaped_set(ee, 0);
    ecore_evas_title_set(ee, "LoCoPDF");
    ecore_evas_show(ee);

    ecore_evas_callback_resize_set(ee, main_win_resize_handler);
    ecore_evas_callback_delete_request_set(ee, main_win_delete_handler);

    /* get a pointer our new Evas canvas */
    evas = ecore_evas_get(ee);

    /* create our white background */
    bg = evas_object_rectangle_add(evas);
    evas_object_color_set(bg, 255, 255, 255, 255);
    evas_object_move(bg, 0, 0);
    evas_object_resize(bg, 600, 800);
    evas_object_name_set(bg, "background");
    evas_object_focus_set(bg, 1);
    evas_object_event_callback_add(bg, EVAS_CALLBACK_KEY_UP, _key_handler,
                                   NULL);
    evas_object_show(bg);

    /* mutex for epdf access */
    pthread_mutexattr_t mta;
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&pdf_renderer_mutex, &mta);
    pthread_mutexattr_destroy(&mta);

    o2 = evas_object_image_add(evas);
    evas_object_move(o2, 0, 0);
    evas_object_name_set(o2, "pdfobj2");

    o1 = evas_object_image_add(evas);
    evas_object_move(o1, 0, 0);
    evas_object_name_set(o1, "pdfobj1");

    set_active_win_id(ee);

    if (!open_file(argv[1]))
        return 1;

    set_properties(ee);

    ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, _client_add, NULL);
    ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, _client_data,
                            NULL);
    ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL, _client_del, ee);

    /* start the main event loop */
    ecore_main_loop_begin();

    close_file();

    fini_database();
    evas_object_del(o1);
    evas_object_del(o2);
    evas_object_del(bg);

    pthread_mutex_destroy(&pdf_renderer_mutex);

    efreet_mime_shutdown();
    edje_shutdown();
    ecore_evas_shutdown();
    ecore_con_shutdown();
    ecore_shutdown();
    evas_shutdown();
}
