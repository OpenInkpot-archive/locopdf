#ifndef _CHOICES_H
#define _CHOICES_H 1

bool
choicebox_pop(Evas_Object *choicebox);

Evas_Object *
choicebox_push(Evas_Object *parent, Evas *canvas,
    choicebox_handler_t handler,
    choicebox_draw_handler_t draw_handler,
    choicebox_close_handler_t close_handler,
    const char *name, int size, int own_edje, void *data);


#endif
