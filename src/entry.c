/***************************************************************************
 *   Copyright (C) 2009 Alexander Kerner <lunohod@openinkpot.org>          *
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
#include <ctype.h>
#include <string.h>

#include <Evas.h>
#include <Edje.h>

#include "entry.h"

#define MAXDIGITS	6

typedef struct entry_number_t entry_number_t;
struct entry_number_t {
	const char *text;
	long number;
	int cnt;

    entry_handler_t handler;
};

static void entry_close(Evas_Object *obj, void *param)
{
    evas_object_hide(obj);

    Evas_Object *focus = (Evas_Object*)evas_object_data_get(obj, "prev-focus");
    if(focus)
        evas_object_focus_set(focus, 1);

    entry_number_t *d = (entry_number_t*)param;
    if(d)
        free(d);

    evas_object_del(obj);
}

static void entry_key_handler(void* param, Evas* e, Evas_Object* o, void* event_info)
{
    Evas_Event_Key_Down* ev = (Evas_Event_Key_Down*)event_info;

	entry_number_t *number = (entry_number_t*)param;

	if(isdigit(ev->keyname[3]) && !ev->keyname[4] && number->cnt < MAXDIGITS) {
		if(number->number > 0)
			number->number *= 10;
		else
			number->number = 0;

		number->number += ev->keyname[3] - '0';
		if(number->number > 0)
			number->cnt++;

		char *t;
		if(number->number > 0)
			asprintf(&t, "%s: %-*ld", number->text, MAXDIGITS, number->number);
		else
			asprintf(&t, "%s: %-*c", number->text, MAXDIGITS, 0);
		edje_object_part_text_set(o, "entrytext", t);
		free(t);
	}
	if(!strcmp(ev->keyname, "Escape")) {
		if(number->cnt > 0) {
			number->cnt--;
			number->number /= 10;

			char *t;
			if(number->number > 0)
				asprintf(&t, "%s: %-*ld", number->text, MAXDIGITS, number->number);
			else
				asprintf(&t, "%s: %-*c", number->text, MAXDIGITS, 0);
			edje_object_part_text_set(o, "entrytext", t);
			free(t);
		} else {
            entry_number_t n = *number;
            void *data = evas_object_data_get(o, "custom-data");
            entry_close(o, param);
            n.handler(o, -1, data);
		}
	}
    if(!strcmp(ev->keyname, "Return")) {
        entry_number_t n = *number;
        void *data = evas_object_data_get(o, "custom-data");
        entry_close(o, param);
        n.handler(o, n.number, data);
    }
}

Evas_Object *
entry_n(Evas_Object *parent, Evas *canvas,
    entry_handler_t handler,
    const char *name, const char *text, void *data)
{
	entry_number_t *l_data = (entry_number_t*)malloc(sizeof(entry_number_t));
	l_data->text = text;
	l_data->number = -1;
	l_data->cnt = 0;
    l_data->handler = handler;


	Evas_Object *obj = edje_object_add(canvas);
	evas_object_name_set(obj, name);
	edje_object_file_set(obj, THEME_DIR "/entrybox.edj", "entrybox");
	evas_object_move(obj, 0, 0);
	evas_object_resize(obj, 600, 800);

    evas_object_data_set(obj, "private-data", l_data);
    evas_object_data_set(obj, "custom-data", data);
    evas_object_data_set(obj, "prev-focus", evas_focus_get(canvas));

	char *t;
	asprintf(&t, "%s: %-*d", text, MAXDIGITS, 999999);
	edje_object_part_text_set(obj, "entrytext", t);
	free(t);

	Evas_Coord x, y, w, h, w2, h2;
	evas_object_geometry_get(
			edje_object_part_object_get(obj, "entrytext"),
			&x, &y, &w, &h);

	asprintf(&t, "%s: %-*c", text, MAXDIGITS, 0);
	edje_object_part_text_set(obj, "entrytext", t);
	free(t);

	w += 40;
	h += 20;
    evas_output_size_get(canvas, &w2, &h2);

	evas_object_resize(obj, w, h);
	evas_object_move(obj, (w2 - w)/2, (h2 - h)/2);
	evas_object_show(obj);

	evas_object_focus_set(obj, 1);
	evas_object_event_callback_add(obj,
			EVAS_CALLBACK_KEY_UP,
			&entry_key_handler,
			l_data);

    return obj;
}
