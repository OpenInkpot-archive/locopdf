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

#include <string.h>
#include <ctype.h>

#include <Evas.h>
#include <Ecore_Evas.h>

#include "keyhandler.h"

/* FIXME: HACK */
static void
_key_handler(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
    Evas_Event_Key_Up *e = (Evas_Event_Key_Up *) event_info;
    key_handler_info_t *handler_info = (key_handler_info_t *) data;

    const char *k = e->keyname;
#define HANDLE_ITEM(h, params, lp) { if(handler_info->h) (*handler_info->h)(evas,obj,params, lp);}
#define HANDLE_KEY(h) {if(handler_info->h) (*handler_info->h)(evas,obj);}
    if (!strcmp(k, "Return")) {
        HANDLE_KEY(ok_handler)
    } else if (!strcmp(k, "Escape"))
        HANDLE_KEY(esc_handler)
    else if (isdigit(k[0]) && !k[1]) {
        bool lp = false;
        //if(e->modifiers & EWL_KEY_MODIFIER_ALT)
        if (evas_key_modifier_is_set(e->modifiers, "Alt"))
            lp = true;
        HANDLE_ITEM(item_handler, k[0] - '0', lp)
    } else if (k[0] == 'K' && k[1] == 'P') {
        if (isdigit(k[3]) && !k[4]) {
            bool lp = false;
            //if(e->modifiers & EWL_KEY_MODIFIER_ALT)
            if (evas_key_modifier_is_set(e->modifiers, "Alt"))
                lp = true;
            HANDLE_ITEM(item_handler, k[3] - '0', lp)
        } else if (!strcmp(k, "KP_Add"))
            HANDLE_KEY(plus_handler)
        else if (!strcmp(k, "KP_Subtract"))
            HANDLE_KEY(minus_handler)
    } else if (!strcmp(k, "Up"))
        HANDLE_KEY(nav_right_handler)
    else if (!strcmp(k, "Down"))
        HANDLE_KEY(nav_left_handler)
    else if (!strcmp(k, "Left"))
        HANDLE_KEY(nav_left_handler)
    else if (!strcmp(k, "Right"))
        HANDLE_KEY(nav_right_handler)
    else if (!strcmp(k, "F2"))
        HANDLE_KEY(nav_menubtn_handler)
}

void
set_key_handler(Evas_Object * obj, key_handler_info_t * handler_info)
{
    evas_object_event_callback_add(obj, EVAS_CALLBACK_KEY_UP,
            _key_handler, handler_info);
}
