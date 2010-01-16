#include <libintl.h>
#include <Evas.h>
#include <Edje.h>

#include <libchoicebox.h>
#include <libeoi.h>
#include <libeoi_help.h>

static void help_closed(Evas_Object* help)
{
    Evas* evas = evas_object_evas_get(help);
    Evas_Object* rr = evas_object_name_find(evas, "help-window");

    Evas_Object* f = evas_object_data_get(rr, "prev-focus");

    evas_object_hide(rr);
    evas_object_del(rr);

    if(f)
        evas_object_focus_set(f, 1);
}

static void page_updated_handler(Evas_Object* help,
        int cur_page,
        int total_pages,
        const char* header __attribute__((unused)),
        void* param __attribute__((unused)))
{
    Evas* evas = evas_object_evas_get(help);
    Evas_Object* rr = evas_object_name_find(evas, "help-window");
    choicebox_aux_edje_footer_handler(rr, "footer", cur_page, total_pages);
}

void help_resize(Evas* evas, int w, int h)
{
    Evas_Object* rr = evas_object_name_find(evas, "help-window");
    evas_object_resize(rr, w, h);
}

void help_show(Evas* evas)
{
    Evas_Object* rr = eoi_main_window_create(evas);

    edje_object_part_text_set(rr, "title", gettext("Locopdf: Help"));
    edje_object_part_text_set(rr, "footer", "0/0");

    evas_object_name_set(rr, "help-window");
    evas_object_move(rr, 0, 0);
    int w, h;
    evas_output_size_get(evas, &w, &h);
    evas_object_resize(rr, w, h);
    evas_object_show(rr);

    Evas_Object *help = eoi_help_new(evas, "locopdf", page_updated_handler, help_closed);
    evas_object_name_set(help, "help-widget");
    evas_object_show(help);

    Evas_Object* f = evas_focus_get(evas);
    evas_object_data_set(rr, "prev-focus", f);

    edje_object_part_swallow(rr, "contents", help);
    evas_object_focus_set(help, 1);
}
