#include <math.h>
#include <glib.h>
#include "vector2.h"

gdouble
p2tr_vector2_dot (const P2trVector2 *a,
                  const P2trVector2 *b)
{
  return P2TR_VECTOR2_DOT (a, b);
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

void
p2tr_vector2_center (const P2trVector2 *a,
                     const P2trVector2 *b,
                     P2trVector2       *dest)
{
  dest->x = (a->x + b->x) * 0.5;
  dest->y = (a->y + b->y) * 0.5;
}

gdouble
p2tr_vector2_norm (const P2trVector2 *v)
{
  return sqrt (P2TR_VECTOR2_LEN_SQ (v));
}

void
p2tr_vector2_copy (P2trVector2       *dest,
                   const P2trVector2 *src)
{
  dest->x = src->x;
  dest->y = src->y;
}
