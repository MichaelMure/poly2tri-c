/* Given a polygon "Poly" (a list of bounded lines), a point "P" and a
 * a planar straight line graph (pslg) "PSLG", the question is if
 * there is a straight line from "P" to some point in/on "Poly" so that
 * the line does not intersect any of the lines of "PSLG".
 *
 * In this file is an algorithm for answering the above question, and
 * it's pseudo-code is hereby presented:
 *
 * IsVisible(G, Poly, P):
 * ----------------------
 * { G = (V,E) - The PSLG where E is the set of edges in the graph   }
 * { Poly      - A polygon (a list of edges)                         }
 * { P         - The point we are checking whether is "visible" from }
 * {             Poly                                                }
 *
 * W <- Some point in T (for example, center of weight)
 *
 * # A set of edges from @PSLG known to intersect some potential lines
 * # from @P to @Poly
 * KnownBlocks <- {}
 *
 * # A set of points on @Poly which should be checked whether a line
 * # from them to @P is not obstructed by any of the edges of @PSLG
 * SecondPoint <- {W}
 *
 * WHILE SecondPoint is not empty:
 *
 *   # Pick some point to check
 *   S <- Some point from SecondPoint
 *   SecondPoint <- SecondPoint \ {S}
 *
 *   PS <- The infinite line going through P and S
 *
 *   IF PS intersects @Poly:
 *
 *     IF there is an edge B=(u,v) (from E) that intersects PS so that
 *        the intersection is between @Poly and @P:
 *
 *       IF B is not in KnownBlocks:
 *         # Try new lines going through the end points of this edge
 *         SecondPoint <- SecondPoint + {u,v}
 *         KnownBlocks <- KnownBlocks + {B}
 *
 *       ELSE:
 *         # Nothing to do - we already tried/are trying to handle this
 *         # blocking edge by going around it
 *       
 *     ELSE:
 *       RETURN "Visible"
 *
 *   ELSE:
 *     # PS doesn't help anyway, no matter if it's blocked or not,
 *     # since it does not reach the polygon
 *
 * RETURN "Ocluded"
 */

#include <glib.h>
#include "bounded-line.h"
#include "pslg.h"


static gboolean
find_closest_intersection (P2trPSLG    *pslg,
                           P2trLine    *line,
                           P2trVector2 *close_to,
                           P2trVector2 *dest)
{
    gdouble distance_sq = G_MAXDOUBLE;
    gboolean found = FALSE;

    const P2trBoundedLine *pslg_line = NULL;
    P2trPSLGIter pslg_iter;
    P2trVector2 temp;

    p2tr_pslg_iter_init (&pslg_iter, pslg);

    while (p2tr_pslg_iter_next (&pslg_iter, &pslg_line))
    {
        if (p2tr_line_intersection (&pslg_line->infinite, line, &temp)
            == P2TR_LINE_RELATION_INTERSECTING)
        {
            gdouble test_distance_sq = P2TR_VECTOR2_DISTANCE_SQ (&temp, close_to);

            found = TRUE;

            if (test_distance_sq < distance_sq)
            {
                distance_sq = test_distance_sq;
            }
        }
    }

    if (found && dest != NULL)
        p2tr_vector2_copy (dest, &temp);

    return found;
}

static void
find_point_in_polygon (P2trPSLG    *polygon,
                       P2trVector2 *out_point)
{
    P2trPSLGIter iter;
    const P2trBoundedLine *line = NULL;

    g_assert (p2tr_pslg_size (polygon) > 1);

    p2tr_pslg_iter_init (&iter, polygon);
    p2tr_pslg_iter_next (&iter, &line);

    out_point->x = (line->start.x + line->end.x) / 2;
    out_point->y = (line->start.y + line->end.y) / 2;
}

#ifdef EXPERIMENTAL_VISIBILITY
static P2trBoundedLine*
pslg_line_intersection (P2trPSLG        *pslg,
                        P2trBoundedLine *line)
{
    P2trPSLGIter iter;
    P2trBoundedLine *pslg_line = NULL;

    p2tr_pslg_iter_init (&iter, pslg);
    while (p2tr_pslg_iter_next (&iter, &pslg_line))
        if (p2tr_bounded_line_intersect (line, pslg_line))
            return pslg_line;

    return NULL;
}

gboolean
p2tr_pslg_visibility_check (P2trPSLG    *pslg,
                            P2trVector2 *point,
                            P2trPSLG    *polygon)
{
    P2trPSLG *known_blocks;
    GArray   *second_points;
    gboolean  found_visibility_path = FALSE;

    /* W <- Some point in T (for example, center of weight) */
    P2trVector2 W;
    find_point_in_polygon (polygon, &W);

    /* KnownBlocks <- {} */
    known_blocks = p2tr_pslg_new ();

    /* SecondPoint <- {W} */
    second_points   = g_array_new (FALSE, FALSE, sizeof(P2trVector2));
    g_array_append_val (second_points, W);

    while ((! found_visibility_path) && second_points->len > 0)
    {
        P2trVector2 S;
        P2trBoundedLine PS;
        P2trVector2 poly_intersection;

        /* S <- Some point from SecondPoint */
        p2tr_vector2_copy (&S, &g_array_index(second_points, P2trVector2, 0));
        /* SecondPoint <- SecondPoint \ {S} */
        g_array_remove_index_fast (second_points, 0);

        /* PS <- The infinite line going through P and S */
        p2tr_bounded_line_init (&PS, &S, point);

        /* IF PS intersects @Poly */
        if (find_closest_intersection (polygon, &PS.infinite, point, &poly_intersection))
        {
            P2trBoundedLine PS_exact, *B;

            /* IF there is an edge B=(u,v) (from E) that intersects PS */
            p2tr_bounded_line_init (&PS_exact, point, &poly_intersection);
            B = pslg_line_intersection (pslg, &PS_exact);

            if (B != NULL)
            {
                /* IF B is not in KnownBlocks: */
                if (! p2tr_pslg_contains_line (known_blocks, B))
                {
                    /* SecondPoint <- SecondPoint + {u,v} */
                    g_array_append_val (second_points, B->start);
                    g_array_append_val (second_points, B->end);
                    /* KnownBlocks <- KnownBlocks + {B} */
                    p2tr_pslg_add_existing_line (known_blocks, B);
                }
            }
            else
            {
                found_visibility_path = TRUE;
            }
        }
    }

    g_array_free (second_points, TRUE);
    p2tr_pslg_free (known_blocks);

    return found_visibility_path;
}
#else

static gboolean
TryVisibilityAroundBlock(P2trPSLG        *PSLG,
                         P2trVector2     *P,
                         P2trPSLG        *ToSee,
                         P2trPSLG        *KnownBlocks,
                         GQueue          *BlocksForTest,
                         /* Try on the edges of this block */
                         const P2trBoundedLine *BlockBeingTested,
                         const P2trVector2     *SideOfBlock)
{
  const P2trVector2 *S = SideOfBlock;
  P2trVector2 ClosestIntersection;
  P2trBoundedLine PS;
  
  p2tr_bounded_line_init (&PS, P, S);

  if (find_closest_intersection (ToSee, &PS.infinite, P, &ClosestIntersection))
    {
      P2trPSLGIter iter;
      P2trBoundedLine PK;
      const P2trBoundedLine *Segment = NULL;
      p2tr_bounded_line_init (&PK, P, &ClosestIntersection);

      p2tr_pslg_iter_init (&iter, PSLG);
      while (p2tr_pslg_iter_next (&iter, &Segment))
        {
          if (Segment == BlockBeingTested)
              continue;

          /* If we have two segments with a shared point,
           * the point should not be blocked by any of them
           */
          if (p2tr_vector2_is_same (SideOfBlock, &(Segment->start))
              || p2tr_vector2_is_same (SideOfBlock, &(Segment->end)))
              continue;

          if (p2tr_bounded_line_intersect (Segment, &PK))
            {
              if (g_queue_find (BlocksForTest, Segment))
                {
                  g_queue_push_tail (BlocksForTest, (P2trBoundedLine*)Segment);
                }
              /* obstruction found! */
              return FALSE;
            }
        }

      /* No obstruction! */
      return TRUE;
    }
  /* No intersection for this attempt, continue */
  return FALSE;
}

/**
 * Check if a point is "visible" from any one or more of the edges in a
 * given group.
 * Formally: Check if there is a line from @ref P to any of the edges in
 * @ref Edges so that the line does not cross any of the lines of the
 * PSLG @ref PSLG
 */
gboolean
IsVisibleFromEdges (P2trPSLG    *PSLG,
                   P2trVector2 *P,
                   P2trPSLG    *Edges)
{
    gboolean  found_visibility_path = FALSE;
    P2trPSLG *KnownBlocks = p2tr_pslg_new ();
    GQueue   *BlocksForTest = g_queue_new ();

    P2trVector2 W;
    find_point_in_polygon (Edges, &W);

    if (TryVisibilityAroundBlock(PSLG, P, Edges, KnownBlocks, BlocksForTest, NULL, &W))
        found_visibility_path = TRUE;

    while (! g_queue_is_empty (BlocksForTest) && ! found_visibility_path)
      {
        const P2trBoundedLine *Block = (P2trBoundedLine*)g_queue_pop_head (BlocksForTest);

        if (p2tr_pslg_contains_line (KnownBlocks, Block))
            continue;
        else if (TryVisibilityAroundBlock(PSLG, P, Edges, KnownBlocks, BlocksForTest, Block, &Block->start)
            || TryVisibilityAroundBlock(PSLG, P, Edges, KnownBlocks, BlocksForTest, Block, &Block->end))
          {
            found_visibility_path = TRUE;
          }
        else
            p2tr_pslg_add_existing_line (KnownBlocks, Block);
      }

    p2tr_pslg_free (KnownBlocks);
    g_queue_free (BlocksForTest);

    return found_visibility_path;
}
#endif

gboolean
p2tr_visibility_is_visible_from_edges (P2trPSLG              *pslg,
                                       P2trVector2           *p,
                                       const P2trBoundedLine *lines,
                                       guint                  line_count)
{
  P2trPSLG *edges = p2tr_pslg_new ();
  gint i;
  gboolean result;
  
  for (i = 0; i < line_count; i++)
    p2tr_pslg_add_existing_line (edges, &lines[i]);
  
  result = IsVisibleFromEdges (pslg, p, edges);
  
  p2tr_pslg_free (edges);
  return result;
}