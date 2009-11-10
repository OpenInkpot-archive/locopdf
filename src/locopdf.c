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

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Edje.h>
#include <Evas.h>
#include <epdf/Epdf.h>

#include <libkeys.h>

#include "dialogs.h"
#include "locopdf.h"
#include "database.h"

#define REL_THEME "themes/themes_oitheme.edj"

/* Uzhosnakh */

static keys_t* keys;

pthread_t thread;

Evas *evas;
Epdf_Document *document;
Epdf_Page     *page;
Ecore_List *pdf_index;
char          *filename;
pthread_mutex_t pdf_renderer_mutex = PTHREAD_MUTEX_INITIALIZER;

int numpages;
int curpage=0;
int nextpage=0;
int curpdfobj=1;
int prerendering=0;
int fitmode=FIT_TEXT_WIDTH;
int readermode=1;
double zoom=1.0;
double zoominc=0.1;
double hpaninc=0.5;
double vpaninc=0.9;

int lefttrim=0;
int righttrim=0;
int toptrim=0;
int bottomtrim=0;


int winwidth=600;
int winheight=800;
/*
 * Returns edje theme file name.
 */

static Evas_Object* get_pdf_object(Evas* canvas)
{
    if(curpdfobj == 1)
        return evas_object_name_find(canvas, "pdfobj1");
    else
        return evas_object_name_find(canvas, "pdfobj2");
}

static void main_win_resize_handler(Ecore_Evas* main_win)
{
    int w, h;
    Evas *canvas = ecore_evas_get(main_win);
    evas_output_size_get(canvas, &w, &h);

    winwidth = w;
    winheight = h;

    Evas_Object *bg = evas_object_name_find(canvas, "background");
    evas_object_resize(bg, w, h);

    render_cur_page(true);
    prerender_next_page();
}

static void main_win_delete_handler(Ecore_Evas* main_win)
{
    ecore_main_loop_quit();
}

char *get_theme_file()
{
 	//char *cwd = get_current_dir_name();
	char *rel_theme;
	asprintf(&rel_theme, "%s/%s", "/usr/share/locopdf", REL_THEME);
    //asprintf(&rel_theme, "%s/%s",cwd, REL_THEME);
	//free(cwd);
	return rel_theme;
}

int get_win_width()
{
    return winwidth;    
}
int get_win_height()
{
    return winheight;    
}
double get_zoom_inc()
{
    return zoominc;    
}
void set_zoom_inc(double newzoominc)
{
    zoominc=newzoominc;    
}
double get_hpan_inc()
{
    return hpaninc;    
}
void set_hpan_inc(double newhpaninc)
{
    hpaninc=newhpaninc;    
}
double get_vpan_inc()
{
    return vpaninc;    
}
void set_vpan_inc(double newvpaninc)
{
    vpaninc=newvpaninc;    
}
int get_lefttrim()
{
    return lefttrim;    
}
void set_lefttrim(int newlefttrim)
{
    lefttrim=newlefttrim;    
}
int get_righttrim()
{
    return righttrim;    
}
void set_righttrim(int newrighttrim)
{
    righttrim=newrighttrim;    
}
int get_toptrim()
{
    return toptrim;    
}
void set_toptrim(int newtoptrim)
{
    toptrim=newtoptrim;    
}
int get_bottomtrim()
{
    return bottomtrim;    
}
void set_bottomtrim(int newbottomtrim)
{
    bottomtrim=newbottomtrim;    
}
int get_fit_mode()
{
    return fitmode;    
}
void set_fit_mode(int newfitmode)
{
    fitmode=newfitmode;
}
int get_reader_mode()
{
    return readermode;    
}
void set_reader_mode(int newreadermode)
{
    readermode=(newreadermode!=0);    
    
}
int get_antialias_mode()
{
    return epdf_fonts_antialias_get() && epdf_lines_antialias_get();
}
void set_antialias_mode(int newantialiasmode)
{
    epdf_fonts_antialias_set(newantialiasmode ? 1 : 0);
    epdf_lines_antialias_set(newantialiasmode ? 1 : 0);
}
int get_num_pages()
{
    return numpages;
}
void goto_page(int newpage)
{
    curpage=newpage;
    reset_cur_panning();
    render_cur_page(true);
    prerender_next_page();
}
int get_cur_page()
{
    return curpage;    
}
Epdf_Document *get_document()
{
    return document;    
}

void render_cur_page(bool next)
{
    char pdfobjstr[20];
    sprintf(pdfobjstr,"pdfobj%d",curpdfobj);

    pthread_mutex_lock(&pdf_renderer_mutex);

    Evas_Object *pdfobj=evas_object_name_find(evas,pdfobjstr);

    int x, y, cwidth, cheight, width, height;

    do {
        epdf_page_page_set(page,curpage);
        epdf_page_size_get (page, &width, &height);
        epdf_page_content_geometry_get(page, &x, &y, &cwidth, &cheight);
        if(cwidth == 0 || cheight == 0)
        {
            if(next)
                curpage++;
            else
                curpage--;
        }
    } while(curpage >= 0 && (cwidth == 0 || cheight == 0));

    if(fitmode == FIT_TEXT_WIDTH)
    {
        lefttrim = x - 10;
        righttrim = width - x - cwidth;
        bottomtrim = y - 10;
        toptrim = height - y - cheight - 10;
    }

    double fitwidthzoom=((double)get_win_width())/((double)(width-lefttrim-righttrim))*zoom;
    double fitheightzoom=((double)get_win_height())/((double)(height-toptrim-bottomtrim))*zoom;
    
    
    double scalex;
    double scaley;
    
    if(fitmode==FIT_WIDTH || fitmode == FIT_TEXT_WIDTH)
    {
        scalex=fitwidthzoom;    
        scaley=fitwidthzoom;
    }
    else if(fitmode==FIT_HEIGHT)
    {
        scalex=fitheightzoom;
        scaley=fitheightzoom;
    }
    else if(fitmode==FIT_BEST)
    {
        if(fitwidthzoom<=fitheightzoom)
        {
            scalex=fitwidthzoom;
            scaley=fitwidthzoom;
        }
        else
        {
            scalex=fitheightzoom;
            scaley=fitheightzoom;
        }
        
    }
    else if(fitmode==FIT_STRETCH)
    {
        scalex=fitwidthzoom;
        scaley=fitheightzoom;
    
    }
    else if(fitmode==FIT_NO)
    {
        scalex=1.0;
        scaley=1.0;
        
    }
    else
    {
        err(1, "Unknown fitmode passed to page rendering function: %d", fitmode);
    }

    
    epdf_page_scale_set (page,scalex,scaley);
    //epdf_page_scale_set (page,1.0,1.0);
    //epdf_page_scale_set(page,zoom,zoom);
    if(!lefttrim && !righttrim && !toptrim && !bottomtrim)
    {
        epdf_page_render (page,pdfobj);
    }
    else
    {
        epdf_page_render_slice (page,pdfobj,(int)(((double)lefttrim)*scalex),(int)(((double)toptrim)*scaley),(int)(((double)(width-lefttrim-righttrim))*scalex),(int)(((double)(height-toptrim-bottomtrim))*scaley));
                             
        
    }

    if(fitmode == FIT_TEXT_WIDTH)
        lefttrim = righttrim = toptrim = bottomtrim;

    pthread_mutex_unlock(&pdf_renderer_mutex);

    //fprintf(stderr,"\nwidth=%d,height=%d,ltrim=%d,rtrim=%d,ttrim=%d,btrim=%d,fwzoom=%f,fhzoom=%f\n",width,height,lefttrim,righttrim,toptrim,bottomtrim,fitwidthzoom,fitheightzoom);
}
void *thread_func(void *vptr_args)
{
    if(curpage>=(numpages-1))
        return NULL;

    pthread_mutex_lock(&pdf_renderer_mutex);

    Evas_Object *pdfobj;
    if(curpdfobj==1)
        pdfobj=evas_object_name_find(evas,"pdfobj2");
    else
        pdfobj=evas_object_name_find(evas,"pdfobj1");

    int x, y, cwidth, cheight, width, height;
    int p = curpage;

    do {
        epdf_page_page_set(page,++p);
        epdf_page_size_get (page, &width, &height);
        epdf_page_content_geometry_get(page, &x, &y, &cwidth, &cheight);
    } while(p < (numpages-1) && (cwidth == 0 || cheight == 0));

    if(curpage>=(numpages-1))
    {
        pthread_mutex_unlock(&pdf_renderer_mutex);
        return NULL;
    }
    nextpage = p;

    if(fitmode == FIT_TEXT_WIDTH) {
        lefttrim = x - 10;
        righttrim = width - x - cwidth;
        bottomtrim = y - 10;
        toptrim = height - y - cheight - 10;
    }

    double fitwidthzoom=((double)get_win_width())/((double)(width-lefttrim-righttrim))*zoom;
    double fitheightzoom=((double)get_win_height())/((double)(height-toptrim-bottomtrim))*zoom;
    
    double scalex;
    double scaley;
    
    if(fitmode==FIT_WIDTH || fitmode == FIT_TEXT_WIDTH)
    {
        scalex=fitwidthzoom;    
        scaley=fitwidthzoom;
    }
    else if(fitmode==FIT_HEIGHT)
    {
        scalex=fitheightzoom;
        scaley=fitheightzoom;
    }
    else if(fitmode==FIT_BEST)
    {
        if(fitwidthzoom<=fitheightzoom)
        {
            scalex=fitwidthzoom;
            scaley=fitwidthzoom;
        }
        else
        {
            scalex=fitheightzoom;
            scaley=fitheightzoom;
        }
        
    }
    else if(fitmode==FIT_STRETCH)
    {
        scalex=fitwidthzoom;
        scaley=fitheightzoom;
    
    }
    else if(fitmode==FIT_NO)
    {
        scalex=1.0;
        scaley=1.0;
        
    }
    else
    {
        err(1, "Unknown fitmode passed to background renderer: %d", fitmode);
    }
    
    epdf_page_scale_set (page,scalex,scaley);
    //epdf_page_scale_set (page,1.0,1.0);
    //epdf_page_scale_set(page,zoom,zoom);
    if(!lefttrim && !righttrim && !toptrim && !bottomtrim)
    {
        epdf_page_render (page,pdfobj);
    }
    else
    {
        epdf_page_render_slice (page,pdfobj,(int)(((double)lefttrim)*scalex),(int)(((double)toptrim)*scaley),(int)(((double)(width-lefttrim-righttrim))*scalex),(int)(((double)(height-toptrim-bottomtrim))*scaley));
                             
        
    }
    //prerendering=0;

    if(fitmode == FIT_TEXT_WIDTH)
        lefttrim = righttrim = toptrim = bottomtrim;

    pthread_mutex_unlock(&pdf_renderer_mutex);

    return NULL;

}
int are_legal_coords(int x1,int y1,int x2,int y2)
{
    
    int xs_in_range=((x1>0&&x1<get_win_width())||(x2>0&&x2<get_win_width()));
    int ys_in_range=((y1>0&&y1<get_win_height())||(y2>0&&y2<get_win_height()));
    int xs_opposite=(x1<=0&&x2>=get_win_width());
    int ys_opposite=(y1<=0&&y2>=get_win_height());
    if((ys_in_range && xs_in_range) || (ys_in_range&& xs_opposite) || (xs_in_range && ys_opposite) || (xs_opposite && ys_opposite))
        return 1;
    return 0;
    
    
}
void pan_cur_page(int panx,int pany)
{
    Evas_Object *pdfobj;
    if(curpdfobj==1)
        pdfobj=evas_object_name_find(evas,"pdfobj1");
    else
        pdfobj=evas_object_name_find(evas,"pdfobj2"); 
    int x,y,w,h;
    evas_object_geometry_get(pdfobj,&x,&y,&w,&h);
    
    
    if(are_legal_coords(x+panx,y+pany,x+w+panx,y+h+pany))
        evas_object_move (pdfobj,x+panx,y+pany);
}

void reset_cur_panning()
{
    Evas_Coord x;
    Evas_Object *pdfobj;
    if(curpdfobj==1)
        pdfobj=evas_object_name_find(evas,"pdfobj1");
    else
        pdfobj=evas_object_name_find(evas,"pdfobj2"); 

    evas_object_geometry_get(pdfobj, &x, NULL, NULL, NULL);
    evas_object_move(evas_object_name_find(evas,"pdfobj1"), x, 0);
    evas_object_move(evas_object_name_find(evas,"pdfobj2"), x, 0);
}
void reset_next_panning()
{
    reset_cur_panning();
    return;
}
void ensure_thread_dead()
{
    if(prerendering)
        pthread_join(thread, NULL);
    prerendering=0;

}

void prerender_next_page()
{
    ensure_thread_dead();
    prerendering=1;
    pthread_create(&thread, NULL, thread_func, NULL);
}


void flip_pages()
{
    Evas_Object *active,*inactive;
    if(curpdfobj==1)
    {
        active=evas_object_name_find(evas,"pdfobj1");
        inactive=evas_object_name_find(evas,"pdfobj2");
        curpdfobj=2;
    }
    else
    {
        active=evas_object_name_find(evas,"pdfobj2");
        inactive=evas_object_name_find(evas,"pdfobj1");
        curpdfobj=1;
    }
    evas_object_hide(active);
    evas_object_show(inactive);
}

static void next_page()
{
    if(curpage>=(numpages-1))
        return;
    //curpage++;
    curpage = nextpage;
    //pthread_join(thread, NULL);
    //
    ensure_thread_dead();
    reset_next_panning();
    flip_pages();
    prerender_next_page();
}

static void prev_page()
{
    if(curpage <= 0)
        return;
    curpage--;
    reset_cur_panning();
    render_cur_page(false);
    prerender_next_page();
}

/* GUI */

/* Main key handler */

static void _quit()
{
    ecore_main_loop_quit();
}

static void _settings(Evas* canvas)
{
    Evas_Object* bgobj = evas_object_name_find(canvas, "background");
    PreferencesDialog(evas, bgobj);
}

static void _zoom_in(Evas* canvas)
{
    Evas_Object* pdfobj = get_pdf_object(canvas);
    int x,y,w,h;
    evas_object_geometry_get(pdfobj,&x,&y,&w,&h);
    long int new_w = lround(((double)w)*(zoom+zoominc)/zoom);
    long int new_h = lround(((double)h)*(zoom+zoominc)/zoom);
    if(are_legal_coords(x,y,x+new_w,y+new_h))
    {
        zoom += zoominc;
        render_cur_page(true);
        prerender_next_page();
    }
}

static void _zoom_out(Evas* canvas)
{
    if(zoom > zoominc)
    {
        Evas_Object* pdfobj = get_pdf_object(canvas);
        int x,y,w,h;
        evas_object_geometry_get(pdfobj, &x, &y, &w, &h);
        long int new_w = lround(((double)w) * (zoom - zoominc) / zoom);
        long int new_h = lround(((double)h) * (zoom - zoominc) / zoom);
        if(are_legal_coords(x, y, x+new_w, y+new_h))
        {
            zoom -= zoominc;
            render_cur_page(true);
            prerender_next_page();
        }
    }
}

static void _page_up(Evas* canvas)
{
    if(readermode)
    {
        Evas_Object *pdfobj = get_pdf_object(canvas);
        long int pan_amt=lround(((double)get_win_height())*vpaninc);
        int x,y,w,h;
        evas_object_geometry_get(pdfobj,&x,&y,&w,&h);

        if(y < 0 && y + pan_amt > 0)
            pan_amt = -y;

        if(y + pan_amt <= 0 && are_legal_coords(x,y+pan_amt,x+w,y+h+pan_amt))
            pan_cur_page(0,pan_amt);
        else if(curpage > 0) {
            prev_page();

            Evas_Object *pdfobj = get_pdf_object(canvas);
            evas_object_geometry_get(pdfobj,&x,&y,&w,&h);
            if(winheight < h)
                pan_cur_page(0, winheight - h);
        }
    }
    else
        prev_page();
}

static void _page_down(Evas* canvas)
{
    if(readermode)
    {
        Evas_Object *pdfobj = get_pdf_object(canvas);
        long int pan_amt=-lround(((double)get_win_height())*vpaninc);
        int x,y,w,h;
        evas_object_geometry_get(pdfobj,&x,&y,&w,&h);

        int endoffset = y + h;
        if(endoffset >= winheight && endoffset + pan_amt < winheight)
            pan_amt = winheight - endoffset;

        if(pan_amt && are_legal_coords(x,y+pan_amt,x+w,y+h+pan_amt))
            pan_cur_page(0,pan_amt);
        else
            next_page();
    }
    else
        next_page();
}

static void _pan(Evas* canvas, int dx, int dy)
{
    pan_cur_page(lround(dx*((double)get_win_width())*hpaninc),
                 lround(dy*((double)get_win_height())*vpaninc));
}

static void _go_to_page(Evas* canvas)
{
    Evas_Object* bgobj = evas_object_name_find(canvas, "background");
    GotoPageEntry(canvas, bgobj);
}

static void _toc(Evas* canvas)
{
    if(pdf_index)
    {
        Evas_Object* bgobj = evas_object_name_find(canvas, "background");
        TOCDialog(canvas, bgobj, pdf_index);
    }
}

void reset_zoom_and_pan()
{
    zoom = 1.0;
    evas_object_move(evas_object_name_find(evas,"pdfobj1"), 0, 0);
    evas_object_move(evas_object_name_find(evas,"pdfobj2"), 0, 0);

    render_cur_page(true);
    prerender_next_page();
}

static void _key_handler(void* data, Evas* canvas, Evas_Object* obj,
                         void* event_info)
{
    Evas_Event_Key_Up* e = (Evas_Event_Key_Up*)event_info;
    const char* action = keys_lookup_by_event(keys, "main", e);
    if(!action) return;

    if(!strcmp(action, "Quit"))
        _quit();
    else if(!strcmp(action, "Settings"))
        _settings(canvas);
    else if(!strcmp(action, "PageUp"))
        _page_up(canvas);
    else if(!strcmp(action, "PageDown"))
        _page_down(canvas);
    else if(!strcmp(action, "ZoomIn"))
        _zoom_in(canvas);
    else if(!strcmp(action, "ZoomOut"))
        _zoom_out(canvas);
    else if(!strcmp(action, "GoToPage"))
        _go_to_page(canvas);
    else if(!strcmp(action, "PanUp"))
        _pan(canvas, 0, -1);
    else if(!strcmp(action, "PanDown"))
        _pan(canvas, 0, 1);
    else if(!strcmp(action, "PanLeft"))
        _pan(canvas, -1, 0);
    else if(!strcmp(action, "PanRight"))
        _pan(canvas, 1, 0);
    else if(!strcmp(action, "ToC"))
        _toc(canvas);
    else if(!strcmp(action, "ResetView"))
        reset_zoom_and_pan();
}

void save_global_settings(char *filename)
{
    begin_transaction();
    set_setting_INT(filename,"current_page",curpage);
    set_setting_DOUBLE(filename,"zoom_increment",zoominc);
    set_setting_DOUBLE(filename,"current_zoom",zoom);
    set_setting_DOUBLE(filename,"h_pan_increment",hpaninc);
    set_setting_DOUBLE(filename,"v_pan_increment",vpaninc);
    set_setting_INT(filename,"left_trim",lefttrim);
    set_setting_INT(filename,"right_trim",righttrim);
    set_setting_INT(filename,"top_trim",toptrim);
    set_setting_INT(filename,"bottom_trim",bottomtrim);
    set_setting_INT(filename,"fit_mode",fitmode);
    set_setting_INT(filename,"reader_mode",readermode);
    commit_transaction();
}
void restore_global_settings(char *filename)
{
    int temp11,temp12,temp13,temp14;
    double temp21,temp22;
    temp11=get_setting_INT(filename,"current_page");
    if(temp11>=0)
        curpage=temp11;
    
    
    temp21=get_setting_DOUBLE(filename,"zoom_increment");
    temp22=get_setting_DOUBLE(filename,"current_zoom");
    if(temp21>0 && temp22>0)
    {
        zoominc=temp21;
        zoom=temp22;
    }
    temp21=get_setting_DOUBLE(filename,"h_pan_increment");
    temp22=get_setting_DOUBLE(filename,"v_pan_increment");
    if(temp21>0 && temp22>0)
    {
        hpaninc=temp21;
        vpaninc=temp22;
        
    }
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
    temp11=get_setting_INT(filename,"reader_mode");
    if(temp11==0 || temp11==1)
    {
        readermode=temp11;    
        
    }
    temp11=get_setting_INT(filename,"fit_mode");
    if(temp11>=0)
    {
        fitmode=temp11;    
    }
}

static void
usage()
{
    fprintf(stderr, "Usage: locopdf <pdf file>\n");
}

int main(int argc, char *argv[])
{
    Ecore_Evas *ee;
    
    Evas_Object *bg,*o1,*o2;

    if(argc != 2) {
        usage();
        return 1;
    }

    keys = keys_alloc("locopdf");

    /* initialize our libraries */
    evas_init();
    ecore_init();
    ecore_evas_init();
    edje_init();

    set_antialias_mode(true);
    /* setup database */
    
    const char *homedir=getenv("HOME");
    char *settingsdir;
    asprintf(&settingsdir,"%s/%s",homedir,".locopdf/");
    if(!(ecore_file_exists(settingsdir) && ecore_file_is_dir(settingsdir)))
    {
        ecore_file_mkpath(settingsdir);
    }
    free(settingsdir);
    char *dbfile;
    asprintf(&dbfile,"%s/%s",homedir,".locopdf/files.db");
    int dbres=init_database(dbfile);
    free(dbfile);
    if(dbres!=(-1))
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
    evas_object_event_callback_add(bg, EVAS_CALLBACK_KEY_UP, _key_handler, NULL);
    evas_object_show(bg);
    

    /* mutex for epdf access */
    pthread_mutexattr_t   mta;
    pthread_mutex_init(&pdf_renderer_mutex, &mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
    
    filename=argv[1];
    document = epdf_document_new (argv[1]);
    if (!document) {
    // manage error here
        fprintf(stderr,"Error Opening Document\n");
        return 1;
    }

    numpages=epdf_document_page_count_get(document);
    page = epdf_page_new (document);
    if (!page) {
    // manage error here
        fprintf(stderr,"Error Processing Document");
    }
    curpdfobj=1;
    
    pdf_index=epdf_index_new (document);
    
    o2 = evas_object_image_add (evas);
    evas_object_move (o2, 0, 0);
    evas_object_name_set(o2, "pdfobj2");
    //evas_object_show (o2);

    o1 = evas_object_image_add (evas);
    

    int init_x = 0;
    int init_y = 0;
    if(dbres != -1)
    {
        init_x = get_setting_INT(argv[1], "current_x");
        init_y = get_setting_INT(argv[1], "current_y");
    }
    evas_object_name_set(o1, "pdfobj1");
    evas_object_show (o1);
    if(dbres!=(-1))
    {
        int am=get_setting_INT(argv[1],"antialias");
        if(am>=0)
            set_antialias_mode(am);
    }
    
    render_cur_page(true);
    prerender_next_page();
    

    /* start the main event loop */
    ecore_main_loop_begin();
    
    /* when the main event loop exits, shutdown our libraries */
    if(dbres!=(-1))
    {
        save_global_settings(argv[1]);
        Evas_Object *pdfobj;
        if(curpdfobj==1)
            pdfobj=evas_object_name_find(evas,"pdfobj1");
        else
            pdfobj=evas_object_name_find(evas,"pdfobj2"); 
        int x,y,w,h;
        evas_object_geometry_get(pdfobj,&x,&y,&w,&h);
        set_setting_INT(argv[1],"current_x",x);
        set_setting_INT(argv[1],"current_y",y);
        set_setting_INT(argv[1],"antialias",get_antialias_mode());
        fini_database();
    }
    evas_object_del (o1);
    evas_object_del (o2);
    evas_object_del (bg);
    epdf_index_delete(pdf_index);
    epdf_page_delete (page);
    epdf_document_delete (document);

    pthread_mutex_destroy(&pdf_renderer_mutex);
    
    
    edje_shutdown();
    ecore_evas_shutdown();
    ecore_shutdown();
    evas_shutdown();


}
