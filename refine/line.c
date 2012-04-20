#include <glib.h>
#include "line.h"

void
p2tr_line_init (P2trLine    *line,
                gdouble      a,
                gdouble      b,
                gdouble      c)
{
  line->a = a;
  line->b = b;
  line->c = c;
}

gboolean
p2tr_line_different_sides (const P2trLine    *line,
                           const P2trVector2 *pt1,
                           const P2trVector2 *pt2)
{
  gdouble side1 = line->a * pt1->x + line->b * pt1->y + line->c;
  gdouble side2 = line->a * pt2->x + line->b * pt2->y + line->c;

  /* Signs are different if the product is negative */
  return side1 * side2 < 0;
}

P2trLineRelation
p2tr_line_intersection (const P2trLine    *l1,
                        const P2trLine    *l2,
                        P2trVector2       *out_intersection)
{
  /* In order to find the intersection, we intend to solve
   * the following set of equations:
   *
   *   ( A1 B1 ) ( x ) = ( -C1 )
   *   ( A2 B2 ) ( y ) = ( -C2 )
   *
   * We can simplify the solution using Cramers Rule which
   * gives the following results:
   *
   *   x = (-C1 * B2) - (-C2 * B1) / (A1 * B2 - A2 * B1)
   *   y = (A1 * -C2) - (A2 * -C1) / (A1 * B2 - A2 * B1)
   */
  double d = l1->a * l2->b - l2->a * l1->b;

  /* If the denominator in the result of applying Cramers rule
   * is zero, then the lines have exactly the same slope, meaning
   * they are either exactly the same or they are parallel and
   * never intersect */
  if (d == 0)
    {
      /* We want to check if the offsets of boths the lines are the
       * same, i.e. whether:  C1 / A1 = C2 / A2
       * This test can be done without zero division errors if we do
       * it in like this:     C1 * A2 = C2 * A1
       */
      if (l1->c * l2->a == l1->a * l2->c)
        return P2TR_LINE_RELATION_SAME;
      else
        return P2TR_LINE_RELATION_PARALLEL;
    }

  if (out_intersection != NULL)
    {
      out_intersection->x = (-l1->c * l2->b + l2->c * l1->b) / d;
      out_intersection->y = (l1->a * -l2->c + l2->a * l1->c) / d;
    }

  return P2TR_LINE_RELATION_INTERSECTING;
}