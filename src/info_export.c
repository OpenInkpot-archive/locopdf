#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_X_Atoms.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_atom.h>

#include "info_export.h"

static inline xcb_connection_t *
connection_get()
{
#if NO_XCB
    return (xcb_connection_t *) NULL;
#else
    return (xcb_connection_t *) ecore_x_connection_get();
#endif
}

static
 xcb_atom_t
get_atom(const char *name)
{
    xcb_intern_atom_cookie_t cookie;
    xcb_generic_error_t *err;
    xcb_intern_atom_reply_t *reply;
    xcb_connection_t *xcb_conn = connection_get();
    if (!xcb_conn)
        return XCB_NONE;
    cookie = xcb_intern_atom(xcb_conn, 0, strlen(name), name);

    reply = xcb_intern_atom_reply(xcb_conn, cookie, &err);
    if (err) {
        free(err);
        return XCB_NONE;
    }
    xcb_atom_t atom = reply->atom;
    free(reply);
    return atom;
}

void
wprop_set_active_win_id(Ecore_X_Window root, Ecore_X_Window win)
{
    xcb_atom_t active_doc_window_id = get_atom("ACTIVE_DOC_WINDOW_ID");
    if (!active_doc_window_id) {
        printf("No atom\n");
        return;
    }

    xcb_atom_t window_atom = get_atom("WINDOW");
    if (!window_atom) {
        printf("Can't get atom WINDOW\n");
        return;
    }

    xcb_connection_t *conn = connection_get();
    if (!conn)
        return;
    xcb_change_property(conn,
                        XCB_PROP_MODE_REPLACE,
                        root,
                        active_doc_window_id,
                        window_atom,
                        sizeof(xcb_window_t) * 8,
                        1, (unsigned char *) &win);
}

void
wprop_set_string(Ecore_X_Window win, const char *prop, const char *value)
{
    xcb_connection_t *conn = connection_get();
    if (!conn)
        return;
    xcb_atom_t atom = get_atom(prop);
    if (!atom) {
        printf("Can't get atom %s\n", prop);
        return;
    }
    xcb_atom_t utf8_string = get_atom("UTF8_STRING");
    if (!atom) {
        printf("Can't get atom UTF8_STRING\n");
        return;
    }
    xcb_change_property(conn,
                        XCB_PROP_MODE_REPLACE,
                        win, atom, utf8_string, 8, strlen(value), value);
}

void
wprop_set_int(Ecore_X_Window win, const char *prop, int value)
{
    xcb_connection_t *conn = connection_get();
    if (!conn)
        return;
    xcb_atom_t atom = get_atom(prop);
    if (!atom) {
        printf("Can't get atom %s\n", prop);
        return;
    }
    xcb_atom_t integer_atom = get_atom("INTEGER");
    if (!atom) {
        printf("Can't get atom INTEGER\n");
        return;
    }
    xcb_change_property(conn,
                        XCB_PROP_MODE_REPLACE,
                        win,
                        atom,
                        integer_atom, 32, 1, (unsigned char *) &value);
}
