#ifndef __P2TR_REFINE_VECTOR2_H__
#define __P2TR_REFINE_VECTOR2_H__

#include <glib.h>

typedef struct {
  gdouble x;
  gdouble y;
} P2trVector2;

gdouble       p2tr_vector2_dot       (const P2trVector2 *a, const P2trVector2 *b);
gboolean      p2tr_vector2_is_same   (const P2trVector2 *a, const P2trVector2 *b);
void          p2tr_vector2_sub       (const P2trVector2 *a, const P2trVector2 *b, P2trVector2 *dest);
gdouble       p2tr_vector2_norm      (const P2trVector2 *v);
void          p2tr_vector2_copy      (P2trVector2 *dest, const P2trVector2 *src);

#endif