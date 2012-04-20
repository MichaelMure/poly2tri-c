#include <glib.h>
#include "bounded-line.h"

P2trBoundedLine*
p2tr_bounded_line_new (const P2trVector2 *start,
                       const P2trVector2 *end)
{
  P2trBoundedLine* line = g_slice_new (P2trBoundedLine);
  p2tr_bounded_line_init (line, start, end);
  return line;
}

void
p2tr_bounded_line_init (P2trBoundedLine   *line,
                        const P2trVector2 *start,
                        const P2trVector2 *end)
{
  /* Traditional line Equation:
   * y - mx - n = 0   <==>   y = mx + n
   * Slope Equation:
   * m = dy / dx
   * Slope + Traditional:
   * dx * y - dy * x - dx * n = 0
   * And the remaining part can be found as
   * dx * y0 - dy * x0 = dx * n
   * So the final equation is:
   * dx * y - dy * x - (dx * y0 - dy * x0) = 0
   */
  gdouble dx = end->x - start->x;
  gdouble dy = end->y - start->y;

  gdouble dxXn = start->y * dx - start->x * dy;

  p2tr_line_init(&line->infinite, -dy, dx, -dxXn);

  p2tr_vector2_copy(&line->start, start);
  p2tr_vector2_copy(&line->end, end);
}

gboolean
p2tr_bounded_line_intersect (const P2trBoundedLine *l1,
                             const P2trBoundedLine *l2)
{
  return p2tr_line_different_sides (&l1->infinite, &l2->start, &l2->end)
    && p2tr_line_different_sides (&l2->infinite, &l1->start, &l1->end);
}

void
p2tr_bounded_line_free (P2trBoundedLine *line)
{
  g_slice_free (P2trBoundedLine, line);
}
