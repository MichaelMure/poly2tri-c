#include <stdarg.h>
#include <glib.h>

#include "point.h"
#include "edge.h"
#include "triangle.h"

#include "cdt.h"
#include "visibility.h"

static gboolean  p2tr_cdt_visible_from_tri        (P2trCDT      *self,
                                                   P2trTriangle *tri,
                                                   P2trVector2  *p);

static gboolean  p2tr_cdt_has_empty_circum_circle (P2trCDT      *self,
                                                   P2trTriangle *tri);

static GList*    p2tr_cdt_triangulate_fan         (P2trCDT   *self,
                                                   P2trPoint *center,
                                                   GList     *edge_pts);

static void      p2tr_cdt_flip_fix                (P2trCDT *self,
                                                   GList   *initial_triangles);

static gboolean  p2tr_cdt_try_flip                (P2trCDT   *self,
                                                   P2trEdge  *to_flip,
                                                   GQueue    *new_tris,
                                                   P2trEdge **new_edge);

static void      p2tr_cdt_on_new_point            (P2trCDT   *self,
                                                   P2trPoint *pt);

P2trCDT* p2tr_cdt_new (P2tCDT *cdt)
{
  P2tTrianglePtrArray cdt_tris = p2t_cdt_get_triangles (cdt);
  GHashTable *point_map = g_hash_table_new (g_direct_hash, g_direct_equal);
  P2trCDT *rmesh = g_slice_new (P2trCDT);

  gint i, j;

  rmesh->mesh = p2tr_mesh_new ();
  rmesh->outline = p2tr_pslg_new ();

  /* First iteration over the CDT - create all the points */
  for (i = 0; i < cdt_tris->len; i++)
  {
    P2tTriangle *cdt_tri = triangle_index (cdt_tris, i);
    for (j = 0; j < 3; j++)
      {
        P2tPoint *cdt_pt = p2t_triangle_get_point(cdt_tri, j);
        P2trPoint *new_pt = g_hash_table_lookup (point_map, cdt_pt);

        if (new_pt == NULL)
          {
            new_pt = p2tr_point_new2 (cdt_pt->x, cdt_pt->y);
            g_hash_table_insert (point_map, cdt_pt, new_pt);
          }
      }
  }

  /* Second iteration over the CDT - create all the edges and find the
   * outline */
  for (i = 0; i < cdt_tris->len; i++)
  {
    P2tTriangle *cdt_tri = triangle_index (cdt_tris, i);

    for (j = 0; j < 3; j++)
      {
        P2tPoint *start = p2t_triangle_get_point (cdt_tri, j);
        P2tPoint *end = p2t_triangle_get_point (cdt_tri, (j + 1) % 3);
        int edge_index = p2t_triangle_edge_index (cdt_tri, start, end);

        P2trPoint *start_new = g_hash_table_lookup (point_map, start);
        P2trPoint *end_new = g_hash_table_lookup (point_map, end);

        if (! p2tr_point_has_edge_to (start_new, end_new))
          {
            gboolean constrained = cdt_tri->constrained_edge[edge_index];
            P2trEdge *edge = p2tr_mesh_new_edge (rmesh->mesh, start_new, end_new, constrained);

            /* If the edge is constrained, we should add it to the
             * outline */
            if (constrained)
              p2tr_pslg_add_new_line(rmesh->outline, &start_new->c,
                  &end_new->c);

            /* We only wanted to create the edge now. We will use it
             * later */
            p2tr_edge_unref (edge);
          }
      }
  }

  /* Third iteration over the CDT - create all the triangles */
  for (i = 0; i < cdt_tris->len; i++)
  {
    P2tTriangle *cdt_tri = triangle_index (cdt_tris, i);

    P2trPoint *pt1 = g_hash_table_lookup (point_map, p2t_triangle_get_point (cdt_tri, 0));
    P2trPoint *pt2 = g_hash_table_lookup (point_map, p2t_triangle_get_point (cdt_tri, 1));
    P2trPoint *pt3 = g_hash_table_lookup (point_map, p2t_triangle_get_point (cdt_tri, 2));

    P2trTriangle *new_tri = p2tr_mesh_new_triangle (rmesh->mesh,
        p2tr_point_get_edge_to(pt1, pt2),
        p2tr_point_get_edge_to(pt2, pt3),
        p2tr_point_get_edge_to(pt3, pt1));

    /* We won't do any usage of the triangle, so just unref it */
    p2tr_triangle_unref (new_tri);
  }

  return rmesh;
}

void
p2tr_cdt_validate_edges (P2trCDT *self)
{
  P2trHashSetIter iter;
  P2trEdge *e;

  p2tr_hash_set_iter_init (&iter, self->mesh->edges);
  while (p2tr_hash_set_iter_next (&iter, (gpointer*)&e))
    {
      if (! e->constrained && e->tri == NULL)
        p2tr_exception_geometric ("Found a non constrained edge without a triangle");

      if (e->tri != NULL)
        {
          gboolean found = FALSE;
          gint i = 0;

          for (i = 0; i < 3; i++)
            if (e->tri->edges[i] == e)
              {
                found = TRUE;
                break;
              }

          if (! found)
              p2tr_exception_geometric ("An edge has a triangle to which it does not belong!");
        }
    }
}

gboolean
p2tr_cdt_visible_from_edge (P2trCDT     *self,
                            P2trEdge    *e,
                            P2trVector2 *p)
{
  P2trBoundedLine line;

  p2tr_bounded_line_init (&line, &P2TR_EDGE_START(e)->c, &e->end->c);

  return p2tr_visibility_is_visible_from_edges (self->outline, p, &line, 1);
}

static gboolean
p2tr_cdt_visible_from_tri (P2trCDT      *self,
                           P2trTriangle *tri,
                           P2trVector2  *p)
{
  P2trBoundedLine lines[3];
  gint i;

  for (i = 0; i < 3; i++)
    p2tr_bounded_line_init (&lines[i],
        &P2TR_EDGE_START(tri->edges[i])->c,
        &tri->edges[i]->end->c);

  return p2tr_visibility_is_visible_from_edges (self->outline, p, lines, 3);
}

static gboolean
p2tr_cdt_has_empty_circum_circle (P2trCDT      *self,
                                  P2trTriangle *tri)
{
  P2trCircle circum;
  P2trPoint *p;
  P2trHashSetIter iter;

  p2tr_triangle_get_circum_circle (tri, &circum);

  p2tr_hash_set_iter_init (&iter, self->mesh->points);
  while (p2tr_hash_set_iter_next (&iter, (gpointer*)&p))
    {
      /** TODO: FIXME - is a point on a constrained edge really not a
       * problem?! */
      if (p2tr_point_has_constrained_edge (p)
          /* The points of a triangle can't violate its own empty
           * circumcircle property */
          || p == tri->edges[0]->end
          || p == tri->edges[1]->end
          || p == tri->edges[2]->end)
          continue;

      if (! p2tr_circle_test_point_outside(&circum, &p->c)
          && p2tr_cdt_visible_from_tri (self, tri, &p->c))
          return FALSE;
    }
  return TRUE;
}

void
p2tr_cdt_validate_cdt (P2trCDT *self)
{
  P2trHashSetIter iter;
  P2trTriangle *tri;

  p2tr_hash_set_iter_init (&iter, self->mesh->triangles);
  while (p2tr_hash_set_iter_next (&iter, (gpointer*)&tri))
    if (! p2tr_cdt_has_empty_circum_circle(self, tri))
      p2tr_exception_geometric ("Not a CDT!");
}

P2trPoint*
p2tr_cdt_insert_point (P2trCDT           *self,
                       const P2trVector2 *pc,
                       P2trTriangle      *point_location_guess)
{
  P2trTriangle *tri;
  P2trPoint    *pt;
  gboolean      inserted = FALSE;
  gint          i;

  if (point_location_guess == NULL)
    tri = p2tr_mesh_find_point (self->mesh, pc);
  else
    tri = p2tr_mesh_find_point2 (self->mesh, pc, point_location_guess);

  if (tri == NULL)
    p2tr_exception_geometric ("Tried to add point outside of domain!");

  pt = p2tr_mesh_new_point (self->mesh, pc);

  /* If the point falls on a line, we should split the line */
  for (i = 0; i < 3; i++)
    {
      P2trEdge *edge = tri->edges[i];
      if (p2tr_math_orient2d (& P2TR_EDGE_START(edge)->c,
              &edge->end->c, pc) == P2TR_ORIENTATION_LINEAR)
        {
          p2tr_cdt_split_edge (self, edge, pt);
          inserted = TRUE;
          break;
        }
    }

  if (! inserted)
    /* If we reached this line, then the point is inside the triangle */
    p2tr_cdt_insert_point_into_triangle (self, pt, tri);

  p2tr_cdt_on_new_point (self, pt);

  return pt;
}

/** Insert a point into a triangle. This function assumes the point is
 * inside the triangle - not on one of its edges and not outside of it.
 */
void
p2tr_cdt_insert_point_into_triangle (P2trCDT      *self,
                                     P2trPoint    *P,
                                     P2trTriangle *tri)
{
  GList *new_tris;

  P2trPoint *A = tri->edges[0]->end;
  P2trPoint *B = tri->edges[1]->end;
  P2trPoint *C = tri->edges[2]->end;

  P2trEdge *CA = tri->edges[1];
  P2trEdge *AB = tri->edges[2];
  P2trEdge *BC = tri->edges[0];

  P2trEdge *AP, *BP, *CP;

  p2tr_triangle_remove (tri);

  AP = p2tr_mesh_new_edge (self->mesh, A, P, FALSE);
  BP = p2tr_mesh_new_edge (self->mesh, B, P, FALSE);
  CP = p2tr_mesh_new_edge (self->mesh, C, P, FALSE);

  new_tris = p2tr_utils_new_reversed_pointer_list (3,
      p2tr_mesh_new_triangle (self->mesh, AB, BP, AP->mirror),
      p2tr_mesh_new_triangle (self->mesh, BC, CP, BP->mirror),
      p2tr_mesh_new_triangle (self->mesh, CA, AP, CP->mirror));

  p2tr_edge_unref (CP);
  p2tr_edge_unref (BP);
  p2tr_edge_unref (AP);

  /* Flip fix the newly created triangles to preserve the the
   * constrained delaunay property. The flip-fix function will unref the
   * new triangles for us! */
  p2tr_cdt_flip_fix (self, new_tris);

  g_list_free (new_tris);
}

/**
 * Triangulate a polygon by creating edges to a center point.
 * 1. If there is a NULL point in the polygon, two triangles are not
 *    created (these are the two that would have used it)
 * 2. THE RETURNED TRIANGLES MUST BE UNREFFED!
 */
static GList*
p2tr_cdt_triangulate_fan (P2trCDT   *self,
                          P2trPoint *center,
                          GList     *edge_pts)
{
  GList *new_tris = NULL;
  GList *iter;

  /* We can not triangulate unless at least two points are given */
  if (edge_pts == NULL || edge_pts->next == NULL)
    {
      p2tr_exception_programmatic ("Not enough points to triangulate as"
          " a star!");
    }

  for (iter = edge_pts; iter != NULL; iter = iter->next)
    {
      P2trPoint *A = (P2trPoint*) iter->data;
      P2trPoint *B = (P2trPoint*) g_list_cyclic_next (edge_pts, iter)->data;
      P2trEdge *AB, *BC, *CA;
      P2trTriangle *tri;

      if (A == NULL || B == NULL)
        continue;

      AB = p2tr_point_get_edge_to (A, B);
      BC = p2tr_mesh_new_or_existing_edge (self->mesh, B, center, FALSE);
      CA = p2tr_mesh_new_or_existing_edge (self->mesh, center, A, FALSE);

      tri = p2tr_mesh_new_triangle (self->mesh, AB, BC, CA);
      new_tris = g_list_prepend (new_tris, tri);

      p2tr_edge_unref (BC);
      p2tr_edge_unref (CA);
    }

  return new_tris;
}

/**
 * Insert a point so that is splits an existing edge. This function
 * assumes that the point is on the edge itself and between its
 * end-points.
 * If the edge being split is constrained, then the function returns a
 * list containing both parts resulted from the splitting. In that case,
 * THE RETURNED EDGES MUST BE UNREFERENCED!
 */
GList*
p2tr_cdt_split_edge (P2trCDT   *self,
                     P2trEdge  *e,
                     P2trPoint *C)
{
  /*      W
   *     /|\
   *    / | \
   *   /  |  \      E.Mirror.Tri: YXW
   * X*---*---*Y    E: X->Y
   *   \  |C /      E.Tri: XYV
   *    \ | /
   *     \|/
   *      V
   */
  P2trPoint *X = P2TR_EDGE_START (e), *Y = e->end;
  P2trPoint *V = (e->tri != NULL) ? p2tr_triangle_get_opposite_point(e->tri, e) : NULL;
  P2trPoint *W = (e->mirror->tri != NULL) ? p2tr_triangle_get_opposite_point (e->mirror->tri, e->mirror) : NULL;
  gboolean   constrained = e->constrained;
  P2trEdge  *XC, *CY;
  GList     *new_tris = NULL, *fan = NULL, *new_edges = NULL;

  p2tr_edge_remove (e);

  XC = p2tr_mesh_new_edge (self->mesh, X, C, constrained);
  CY = p2tr_mesh_new_edge (self->mesh, C, Y, constrained);

  fan = p2tr_utils_new_reversed_pointer_list (4, W, X, V, Y);
  new_tris = p2tr_cdt_triangulate_fan (self, C, fan);
  g_list_free (fan);

  /* Now make this a CDT again
   * The new triangles will be unreffed by the flip_fix function, which
   * is good since we receive them with an extra reference!
   */
  p2tr_cdt_flip_fix (self, new_tris);
  g_list_free (new_tris);

  if (constrained)
    {
      /* If this was a subsegment, then both parts of the subsegment
       * should exist */
      if (p2tr_edge_is_removed (XC) || p2tr_edge_is_removed (CY))
        p2tr_exception_geometric ("Subsegments gone!");
      else
        {
          new_edges = g_list_prepend (new_edges, CY);
          new_edges = g_list_prepend (new_edges, XC);
        }
    }
  else
    {
      p2tr_edge_unref (XC);
      p2tr_edge_unref (CY);
    }

  p2tr_cdt_on_new_point (self, C);

  return new_edges;
}

/* This function implements "Lawson's algorithm", also known as "The
 * diagonal swapping algorithm". This algorithm takes a CDT, and a list
 * of triangles that were formed by the insertion of a new point into
 * the triangular mesh, and makes the triangulation a CDT once more. Its
 * logic is explained below:
 *
 *   If a point is added to an existing triangular mesh then
 *   circumcircles are formed for all new triangles formed. If any of
 *   the neighbours lie inside the circumcircle of any triangle, then a
 *   quadrilateral is formed using the triangle and its neighbour. The
 *   diagonals of this quadrilateral are swapped to give a new
 *   triangulation. This process is continued till there are no more
 *   faulty triangles and no more swaps are required.
 *
 * The description above is slightly inaccurate, as it does not consider
 * the case were the diagonals can not be swapped since the
 * quadrilateral is concave (then swapping the diagonals would result in
 * a diagonal outside the quad, which is undesired). This code does also
 * handle that case.
 */

/**
 * THE GIVEN INPUT TRIANGLES MUST BE GIVEN WITH AN EXTRA REFERENCE SINCE
 * THEY WILL BE UNREFFED!
 */
static void
p2tr_cdt_flip_fix (P2trCDT *self,
                   GList   *initial_triangles)
{
  GQueue flipped_edges, tris_to_fix;
  GList *iter;

  g_queue_init (&flipped_edges);
  g_queue_init (&tris_to_fix);

  for (iter = initial_triangles; iter != NULL; iter = iter->next)
    g_queue_push_tail (&tris_to_fix, iter->data);

  while (! g_queue_is_empty (&tris_to_fix))
    {
      P2trTriangle *tri = (P2trTriangle*)g_queue_pop_head (&tris_to_fix);
      P2trCircle   circum_circle;
      gint         i;

      if (p2tr_triangle_is_removed (tri))
        {
          p2tr_triangle_unref (tri);
          continue;
        }

      p2tr_triangle_get_circum_circle (tri, &circum_circle);

      for (i = 0; i < 3; i++)
        {
          P2trEdge  *e = tri->edges[i];
          P2trPoint *opposite;

          if (e->constrained || e->delaunay)
              continue;

          opposite = p2tr_triangle_get_opposite_point (e->mirror->tri, e->mirror);
          if (! p2tr_circle_test_point_outside(&circum_circle, &opposite->c))
            {
              P2trEdge *flipped;
              if (p2tr_cdt_try_flip (self, e, &tris_to_fix, &flipped))
                {
                  g_queue_push_tail (&flipped_edges, flipped);
                  /* Stop iterating this triangle since it doesn't exist
                   * any more */
                  break;
                }
            }
        }

      /* We are finished with the triangle, so unref it as promised */
      p2tr_triangle_unref (tri);
    }

  while (! g_queue_is_empty (&flipped_edges))
    {
      P2trEdge *e = (P2trEdge*) g_queue_pop_head (&flipped_edges);
      e->delaunay = e->mirror->delaunay = FALSE;
      p2tr_edge_unref (e);
    }
}

/**
 * Try to flip a given edge. If successfull, return the new edge on
 * @ref new_edge, append the new triangles to @ref new_tris and return
 * TRUE.
 * THE NEW TRIANGLES MUST BE UNREFFED!
 * THE NEW EDGE MUST BE UNREFFED!
 */
static gboolean
p2tr_cdt_try_flip (P2trCDT   *self,
                   P2trEdge  *to_flip,
                   GQueue    *new_tris,
                   P2trEdge **new_edge)
{
  /*    C
   *  / | \
   * B-----A    to_flip: A->B
   *  \ | /     to_flip.Tri: ABC
   *    D
   */
  P2trPoint *A, *B, *C, *D;
  P2trTriangle *ABC, *ADB;
  P2trEdge *DC;

  new_edge = NULL;

  if (to_flip->constrained || to_flip->delaunay)
    {
      return FALSE;
    }

  A = P2TR_EDGE_START (to_flip);
  B = to_flip->end;
  C = p2tr_triangle_get_opposite_point (to_flip->tri, to_flip);
  D = p2tr_triangle_get_opposite_point (to_flip->mirror->tri, to_flip->mirror);

  ABC = to_flip->tri;
  ADB = to_flip->mirror->tri;

  /* Check if the quadriliteral ADBC is concave (because if it is, we
   * can't flip the edge) */
  if (p2tr_triangle_get_angle_at(ABC, A) + p2tr_triangle_get_angle_at(ADB, A) >= G_PI)
      return FALSE;
  if (p2tr_triangle_get_angle_at(ABC, B) + p2tr_triangle_get_angle_at(ADB, B) >= G_PI)
      return FALSE;

  p2tr_edge_remove (to_flip);

  DC = p2tr_mesh_new_edge (self->mesh, D, C, FALSE);
  DC->delaunay = DC->mirror->delaunay = TRUE;

  g_queue_push_tail (new_tris, p2tr_mesh_new_triangle (self->mesh,
      p2tr_point_get_edge_to (C, A),
      p2tr_point_get_edge_to (A, D),
      DC));

  g_queue_push_tail (new_tris, p2tr_mesh_new_triangle (self->mesh,
      p2tr_point_get_edge_to (D, B),
      p2tr_point_get_edge_to (B, C),
      DC->mirror));

  *new_edge = DC;

  return TRUE;
}

/* Whenever a new point was inserted, it may disturb triangles
 * that are extremly skinny and therefor their circumscribing
 * circles are very large and will contain that point, even though they
 * may be very far from that point.
 * We have no choice but to check these and fix them if necessary
 */
static void
p2tr_cdt_on_new_point (P2trCDT   *self,
                       P2trPoint *pt)
{
  GList *bad_tris = NULL;
  P2trTriangle *tri;
  P2trHashSetIter iter;

  p2tr_hash_set_iter_init (&iter, self->mesh->triangles);
  while (p2tr_hash_set_iter_next (&iter, (gpointer*)&tri))
    {
      if (p2tr_triangle_circumcircle_contains_point (tri, &pt->c)
          != P2TR_INCIRCLE_OUT)
        {
          bad_tris = g_list_prepend (bad_tris, tri);
          p2tr_triangle_ref (tri);
        }
    }

  p2tr_cdt_flip_fix (self, bad_tris);
  g_list_free (bad_tris);
}

