#ifndef __P2TR_REFINE_VECTOR2_H__
#define __P2TR_REFINE_VECTOR2_H__

#include <glib.h>

typedef struct {
  gdouble x;
  gdouble y;
} P2trVector2;

gdouble       p2tr_vector2_dot       (P2trVector2 *a, P2trVector2 *b);
gboolean      p2tr_vector2_is_same   (P2trVector2 *a, P2trVector2 *b);
void          p2tr_vector2_sub       (P2trVector2 *a, P2trVector2 *b, P2trVector2 *dest);
gdouble       p2tr_vector2_norm      (P2trVector2 *v);
void          p2tr_vector2_copy      (P2trVector2 *dest, P2trVector2 *src);

#endif