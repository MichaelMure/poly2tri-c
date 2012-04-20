#include <math.h>
#include <glib.h>
#include "vector2.h"

gdouble
p2tr_vector2_dot (const P2trVector2 *a,
                  const P2trVector2 *b)
{
  return a->x * b->x + a->y * b->y;
}

gboolean
p2tr_vector2_is_same (const P2trVector2 *a,
                      const P2trVector2 *b)
{
  if (a == NULL || b == NULL)
    return ! ((a == NULL) ^ (b == NULL));
  else
    return a->x == b->x && a->y == b->y;
}

void
p2tr_vector2_sub (const P2trVector2 *a,
                  const P2trVector2 *b,
                  P2trVector2       *dest)
{
  dest->x = a->x - b->x;
  dest->y = a->y - b->y;
}

gdouble
p2tr_vector2_norm (const P2trVector2 *v)
{
  return sqrt (v->x * v->x + v->y * v->y);
}

void
p2tr_vector2_copy (P2trVector2       *dest,
                   const P2trVector2 *src)
{
  dest->x = src->x;
  dest->y = src->y;
}
