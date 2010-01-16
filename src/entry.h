#ifndef ENTRYBOX_H
#define ENTRYBOX_H

#include <Evas.h>

typedef void (*entry_handler_t)(Evas_Object* entry,
                                    long num,
                                    void* param);

Evas_Object *
entry_n(Evas_Object *parent, Evas *canvas,
        entry_handler_t handler,
        const char *name, const char *text, void *data);

#endif
