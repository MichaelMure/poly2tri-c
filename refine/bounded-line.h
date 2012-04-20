#ifndef __P2TC_REFINE_BOUNDED_LINE_H__
#define __P2TC_REFINE_BOUNDED_LINE_H__

#include <glib.h>
#include "vector2.h"
#include "line.h"

typedef struct
{
  P2trLine infinite;
  P2trVector2 start, end;
} P2trBoundedLine;

P2trBoundedLine*  p2tr_bounded_line_new       (const P2trVector2 *start,
                                               const P2trVector2 *end);

void              p2tr_bounded_line_init      (P2trBoundedLine   *line,
                                               const P2trVector2 *start,
                                               const P2trVector2 *end);

gboolean          p2tr_bounded_line_intersect (const P2trBoundedLine *l1,
                                               const P2trBoundedLine *l2);

void              p2tr_bounded_line_free      (P2trBoundedLine *line);

#endif