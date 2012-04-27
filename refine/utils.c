#include <stdarg.h>
#include <glib.h>

#include "utils.h"

GList*
p2tr_utils_new_reversed_pointer_list (int count, ...)
{
  int i;
  va_list args;
  GList *result = NULL;
  
  va_start (args, count);
  for (i = 0; i < count; i++)
    result = g_list_prepend (result, va_arg (args, gpointer));
  va_end (args);
  
  return result;
}
