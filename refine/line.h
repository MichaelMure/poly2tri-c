#ifndef __P2TC_REFINE_LINE_H__
#define __P2TC_REFINE_LINE_H__

#include <glib.h>
#include "vector2.h"

/* A line is the equation of the following form:
 *   a * X + b * Y + c = 0
 */
typedef struct {
  gdouble a, b, c;
} P2trLine;

typedef enum
{
  P2TR_LINE_RELATION_INTERSECTING = 0,
  P2TR_LINE_RELATION_PARALLEL = 1,
  P2TR_LINE_RELATION_SAME = 2
} P2trLineRelation;

void              p2tr_line_init            (P2trLine    *line,
                                             gdouble      a,
                                             gdouble      b,
                                             gdouble      c);

gboolean          p2tr_line_different_sides (const P2trLine    *line,
                                             const P2trVector2 *pt1,
                                             const P2trVector2 *pt2);

P2trLineRelation  p2tr_line_intersection    (const P2trLine    *l1,
                                             const P2trLine    *l2,
                                             P2trVector2       *out_intersection);

#endif