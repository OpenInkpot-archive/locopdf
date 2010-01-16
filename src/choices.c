#include <stdio.h>
#include <string.h>
#include <Evas.h>
#include <Edje.h>
#include <libchoicebox.h>
#include <libeoi.h>
#include "choices.h"

#define DEFAULT_CHOICEBOX_THEME_FILE "/usr/share/choicebox/choicebox.edj"

bool
choicebox_pop(Evas_Object *choicebox)
{
    Evas_Object *parent;
    Evas *canvas = evas_object_evas_get(choicebox);
    Evas_Object *main_canvas_edje = evas_object_name_find(canvas, "main_choicebox_edje");
    parent = evas_object_data_get(choicebox, "parent");

    Evas_Object *focus = (Evas_Object*)evas_object_data_get(choicebox, "prev-focus");

    evas_object_hide(choicebox);
    edje_object_part_unswallow(main_canvas_edje,  choicebox);
    evas_object_del(choicebox);
    edje_object_part_text_set(main_canvas_edje, "footer", "");
    edje_object_part_text_set(main_canvas_edje, "path", "");
    if(parent) {
        edje_object_part_swallow(main_canvas_edje, "contents", parent);
        evas_object_focus_set(parent , true);
        evas_object_show(parent);
    } else {
        evas_object_hide(main_canvas_edje);
        evas_object_del(main_canvas_edje);

        if(focus) {
            evas_object_focus_set(focus, true);
        }
    }

    return true;
}

static
void _page_handler(Evas_Object* self,
                                int a,
                                int b,
                                void* param __attribute__((unused)))
{
    Evas *canvas = evas_object_evas_get(self);
    Evas_Object *main_edje = evas_object_name_find(canvas, "main_choicebox_edje");
    choicebox_aux_edje_footer_handler(main_edje, "footer", a, b);
}

static void _close_handler(Evas_Object* choicebox __attribute__((unused)),
                          void *param __attribute__((unused)))
{
    choicebox_pop(choicebox);
}

Evas_Object *
choicebox_push(Evas_Object *parent, Evas *canvas,
    choicebox_handler_t handler,
    choicebox_draw_handler_t draw_handler,
    choicebox_close_handler_t close_handler,
    const char *name, int size, int own_edje, void *data)
{
    choicebox_info_t info = {
        NULL,
        DEFAULT_CHOICEBOX_THEME_FILE,
        "full",
        own_edje ? THEME_DIR "/items.edj" : DEFAULT_CHOICEBOX_THEME_FILE,
        own_edje ? "item-default" : "item-settings",
        handler,
        draw_handler,
        _page_handler,
        close_handler ? close_handler : _close_handler,
    };

    Evas_Object *main_canvas_edje = evas_object_name_find(canvas, "main_choicebox_edje");
    if(!main_canvas_edje) {
        main_canvas_edje = eoi_main_window_create(canvas);
        evas_object_name_set(main_canvas_edje, "main_choicebox_edje");
        evas_object_move(main_canvas_edje, 0, 0);
        evas_object_resize(main_canvas_edje, 600, 800);
        evas_object_show(main_canvas_edje);
    }

    Evas_Object* choicebox = choicebox_new(canvas, &info, data);
    if(!choicebox) {
         printf("no choicebox\n");
        return NULL;
    }
    eoi_register_fullscreen_choicebox(choicebox);
    choicebox_set_size(choicebox, size);
    evas_object_name_set(choicebox, name);
    if (parent) {
        edje_object_part_unswallow(main_canvas_edje, parent);
        evas_object_hide(parent);
    } else
        evas_object_data_set(choicebox, "prev-focus", evas_focus_get(canvas));
    evas_object_data_set(choicebox, "parent", parent);
    edje_object_part_swallow(main_canvas_edje, "contents", choicebox);

    choicebox_aux_subscribe_key_up(choicebox);
    evas_object_focus_set(choicebox, true);
    evas_object_show(choicebox);
    return choicebox;
}
