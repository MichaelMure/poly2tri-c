#ifndef __P2TC_REFINE_CIRCLE_H__
#define __P2TC_REFINE_CIRCLE_H__

#include <glib.h>
#include "vector2.h"

typedef struct {
  P2trVector2 center;
  gdouble radius;
} P2trCircle;

gboolean  p2tr_circle_test_point_outside (P2trCircle *circle, P2trVector2 *pt);

#endif