#include <glib.h>
#include "circle.h"

gboolean
p2tr_circle_test_point_outside (P2trCircle  *circle,
                                P2trVector2 *pt)
{
  gdouble dx = circle->center.x - pt->x;
  gdouble dy = circle->center.y - pt->y;

  gdouble d_squared = dx * dx + dy * dy;
  gdouble radius_squared = circle->radius * circle->radius;

  return d_squared > radius_squared;
}