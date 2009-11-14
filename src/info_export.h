#ifndef INFO_EXPORT_H
#define INFO_EXPORT_H

#include <Ecore_X.h>

void wprop_set_active_win_id(Ecore_X_Window root, Ecore_X_Window win);
void wprop_set_string(Ecore_X_Window win, const char *prop,
                      const char *value);
void wprop_set_int(Ecore_X_Window win, const char *prop, int value);

#endif                          /* INFO_EXPORT_H */
