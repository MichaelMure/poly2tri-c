#include <stdio.h>
#include <math.h>
#include <glib.h>

#include "utils.h"
#include "math.h"

#include "point.h"
#include "edge.h"
#include "triangle.h"

#include "mesh.h"
#include "cdt.h"
#include "cluster.h"

#include "delaunay-terminator.h"

/* The code in this file is based on the "Delaunay Terminator" algorithm
 * - an algorithm for refining constrained delaunay triangulations. The
 * algorithm itself appears in a paper by Jonathan Richard Shewchuk as
 * described below:
 *
 *   Delaunay Refinement Algorithms for Triangular Mesh Generation
 *   Computational Geometry: Theory and Applications 22(1–3):21–74, May 2002
 *   Jonathan Richard Shewchuk
 *   http://www.cs.berkeley.edu/~jrs/papers/2dj.pdf
 */

gboolean
p2tr_cdt_test_encroachment_ignore_visibility (const P2trVector2 *w,
                                              P2trEdge          *e)
{
  return p2tr_math_diametral_circle_contains (&P2TR_EDGE_START(e)->c,
      &e->end->c, w);
}

gboolean
p2tr_cdt_is_encroached_by (P2trCDT     *self,
                           P2trEdge    *e,
                           P2trVector2 *p)
{
  if (! e->constrained)
      return FALSE;

  return p2tr_cdt_test_encroachment_ignore_visibility (p, e)
      && p2tr_cdt_visible_from_edge (self, e, p);
}


P2trHashSet*
p2tr_cdt_get_segments_encroached_by (P2trCDT     *self,
                                     P2trVector2 *C)
{
  P2trHashSet *encroached_edges = p2tr_hash_set_new (g_direct_hash,
      g_direct_equal, (GDestroyNotify) p2tr_edge_unref);

  P2trHashSetIter iter;
  P2trEdge *e;
  
  p2tr_hash_set_iter_init (&iter, self->mesh->edges);
  while (p2tr_hash_set_iter_next (&iter, (gpointer*)&e))
    if (e->constrained
        && p2tr_hash_set_contains (encroached_edges, e->mirror) == FALSE
        && p2tr_cdt_is_encroached_by (self, e, C))
      {
        p2tr_hash_set_insert (encroached_edges, e);
      }

  return encroached_edges;
}

gboolean
p2tr_cdt_is_encroached (P2trEdge *E)
{
  P2trTriangle *T1 = E->tri;
  P2trTriangle *T2 = E->mirror->tri;

  if (! E->constrained)
      return FALSE;
  
  return (T1 != NULL && p2tr_cdt_test_encroachment_ignore_visibility (&p2tr_triangle_get_opposite_point (T1, E)->c, E))
      || (T2 != NULL && p2tr_cdt_test_encroachment_ignore_visibility (&p2tr_triangle_get_opposite_point (T2, E)->c, E));
}

/* ****************************************************************** */
/* Now for the algorithm itself                                       */
/* ****************************************************************** */

static gboolean
SplitPermitted (P2trDelaunayTerminator *self, P2trEdge *s, gdouble d);

static void
SplitEncroachedSubsegments (P2trDelaunayTerminator *self, gdouble theta, P2trTriangleTooBig delta);

static void
NewVertex (P2trDelaunayTerminator *self, P2trPoint *v, gdouble theta, P2trTriangleTooBig delta);

static gdouble
ShortestEdgeLength (P2trTriangle *tri);

static gboolean
TolerantIsShorter (P2trEdge *toTest, P2trEdge *reference);

static inline gdouble
LOG2 (gdouble v);

static gboolean
TolerantIsPowerOfTwoLength (gdouble length);

static void
ChooseSplitVertex(P2trEdge *e, P2trVector2 *dst);



static inline gint
triangle_quality_compare (P2trTriangle *t1, P2trTriangle *t2)
{
  gdouble a1 = p2tr_triangle_smallest_non_constrained_angle (t1);
  gdouble a2 = p2tr_triangle_smallest_non_constrained_angle (t2);
  
  return (a1 < a2) ? -1 : ((a1 == a2) ? 0 : 1);
}

P2trDelaunayTerminator*
p2tr_dt_new (gdouble theta, P2trTriangleTooBig delta, P2trCDT *cdt)
{
  P2trDelaunayTerminator *self = g_slice_new (P2trDelaunayTerminator);
  self->Qt = g_sequence_new (NULL);
  g_queue_init (&self->Qs);
  self->delta = delta;
  self->theta = theta;
  self->mesh = cdt;
  return self;
}

static void
p2tr_dt_enqueue_tri (P2trDelaunayTerminator *self,
                     P2trTriangle           *tri)
{
  p2tr_triangle_ref (tri);
  g_sequence_insert_sorted (self->Qt, tri, (GCompareDataFunc)triangle_quality_compare, NULL);
}

static inline gboolean
p2tr_dt_tri_queue_is_empty (P2trDelaunayTerminator *self)
{
  return g_sequence_iter_is_end (g_sequence_get_begin_iter (self->Qt));
}

static P2trTriangle*
p2tr_dt_dequeue_tri (P2trDelaunayTerminator *self)
{
  GSequenceIter *first = g_sequence_get_begin_iter (self->Qt);

  /* If we have an empty sequence, return NULL */
  if (p2tr_dt_tri_queue_is_empty (self))
    return NULL;
  else
    {
      P2trTriangle *ret = g_sequence_get (first);
      g_sequence_remove (first);
      return ret;
    }
}

static void
p2tr_dt_enqueue_segment (P2trDelaunayTerminator *self,
                         P2trEdge               *E)
{
  if (! E->constrained)
    p2tr_exception_programmatic ("Tried to append a non-segment!");

  p2tr_edge_ref (E);
  g_queue_push_tail (&self->Qs, E);
}

static P2trEdge*
p2tr_dt_dequeue_segment (P2trDelaunayTerminator *self)
{
  if (g_queue_is_empty (&self->Qs))
    return NULL;
  else
    return g_queue_pop_head (&self->Qs);
}

static gboolean
p2tr_dt_segment_queue_is_empty (P2trDelaunayTerminator *self)
{
  return g_queue_is_empty (&self->Qs);
}

static gboolean
false_too_big (P2trTriangle *tri)
{
  return FALSE;
}

void p2tr_dt_refine (P2trDelaunayTerminator *self,
                     gint                    max_steps)
{
  P2trHashSetIter hs_iter;
  P2trEdge *s;
  P2trTriangle *t;
  gint steps;
  
  p2tr_hash_set_iter_init (&hs_iter, self->mesh->mesh->edges);
    while (p2tr_hash_set_iter_next (&hs_iter, (gpointer*)&s))
    if (s->constrained && p2tr_cdt_is_encroached (s))
      p2tr_dt_enqueue_segment (self, s);

  SplitEncroachedSubsegments (self, 0, false_too_big);

  p2tr_hash_set_iter_init (&hs_iter, self->mesh->mesh->triangles);
  while (p2tr_hash_set_iter_next (&hs_iter, (gpointer*)&t))
    if (p2tr_triangle_smallest_non_constrained_angle (t) < self->theta)
      p2tr_dt_enqueue_tri (self, t);

  steps = 0;
  while (! p2tr_dt_tri_queue_is_empty (self) && steps < max_steps)
    {
      t = p2tr_dt_dequeue_tri (self);

      if (p2tr_hash_set_contains (self->mesh->mesh->triangles, t))
        {
          P2trCircle tCircum;
          P2trVector2 *c;
          P2trTriangle *triContaining_c_NOREF;
          P2trHashSet *E;
          
          p2tr_triangle_get_circum_circle (t, &tCircum);
          c = &tCircum.center;

          triContaining_c_NOREF = p2tr_mesh_find_point_local (self->mesh->mesh, c, t);

          /* If no edge is encroached, then this must be
           * inside the triangulation domain!!! */
          if (triContaining_c_NOREF == NULL)
            p2tr_exception_geometric ("Should not happen!");

          /* Now, check if this point would encroach any edge
           * of the triangulation */
          E = p2tr_cdt_get_segments_encroached_by (self->mesh, c);

          if (p2tr_hash_set_size (E) == 0)
            {
              P2trPoint *cPoint;
              cPoint = p2tr_cdt_insert_point (self->mesh, c, triContaining_c_NOREF);
              NewVertex (self, cPoint, self->theta, self->delta);
              
              p2tr_point_unref (cPoint);
            }
          else
            {
              gdouble d = ShortestEdgeLength(t);
              
              p2tr_hash_set_iter_init (&hs_iter, E);
              while (p2tr_hash_set_iter_next (&hs_iter, (gpointer*)&s))
                if (self->delta (t) || SplitPermitted(self, s, d))
                  p2tr_dt_enqueue_segment (self, s);

              if (! p2tr_dt_segment_queue_is_empty (self))
                {
                  p2tr_dt_enqueue_tri (self, t);
                  SplitEncroachedSubsegments(self, self->theta, self->delta);
                }
            }
          p2tr_hash_set_free (E);
      }

      p2tr_triangle_unref (t);
    }
}

static gboolean
SplitPermitted (P2trDelaunayTerminator *self, P2trEdge *s, gdouble d)
{
  P2trCluster *startCluster = p2tr_cluster_get_for (P2TR_EDGE_START(s), s);
  P2trCluster *endCluster =   p2tr_cluster_get_for (s->end, s);
  P2trCluster *S_NOREF;
  GList *iter;
  
  gboolean permitted = FALSE;
  
  if (! TolerantIsPowerOfTwoLength (p2tr_edge_get_length (s))
      /* True when different, meaning both null or both exist */
      || ((startCluster != NULL) ^ (endCluster == NULL)))
    {
      permitted = TRUE;
    }
  
  if (! permitted)
    {
      S_NOREF = (startCluster != NULL) ? startCluster : endCluster;

      for (iter = g_queue_peek_head_link (&S_NOREF->edges); iter != NULL; iter = iter->next)
        if (TolerantIsShorter((P2trEdge*) iter->data, s)) /* e shorter than s */
          {
            permitted = TRUE;
            break;
          }
    }
  
  if (! permitted)
    {
      gdouble rmin = p2tr_edge_get_length(s) * sin(S_NOREF->min_angle / 2);
      if (rmin >= d)
        permitted = TRUE;
    }

  if (startCluster) p2tr_cluster_free (startCluster);
  if (endCluster) p2tr_cluster_free (endCluster);

  return permitted;
}

static void
SplitEncroachedSubsegments (P2trDelaunayTerminator *self, gdouble theta, P2trTriangleTooBig delta)
{
  while (! p2tr_dt_segment_queue_is_empty (self))
  {
    P2trEdge *s = p2tr_dt_dequeue_segment (self);
    if (p2tr_hash_set_contains (self->mesh->mesh->edges, s))
      {
        P2trVector2 v;
        P2trPoint *Pv;
        GList *parts, *iter;

        ChooseSplitVertex (s, &v);
        Pv = p2tr_mesh_new_point (self->mesh->mesh, &v);
        
        /* Update here if using diametral lenses */

        parts = p2tr_cdt_split_edge (self->mesh, s, Pv);
        
        NewVertex (self, Pv, theta, delta);

        for (iter = parts; iter != NULL; iter = iter->next)
          {
            P2trEdge *e = (P2trEdge*)iter->data;
            if (p2tr_cdt_is_encroached (e))
              p2tr_dt_enqueue_segment (self, e);
            p2tr_edge_unref (e);
          }
      }
    p2tr_edge_unref (s);
  }
}

static void
NewVertex (P2trDelaunayTerminator *self, P2trPoint *v, gdouble theta, P2trTriangleTooBig delta)
{
  GList *iter;
  for (iter = v->outgoing_edges; iter != NULL; iter = iter->next)
    {
      P2trEdge *outEdge = (P2trEdge*) iter->data;
      P2trTriangle *t = outEdge->tri;
      P2trEdge *e;
      
      if (t == NULL)
          continue;

      e = p2tr_triangle_get_opposite_edge (t, v);
      
      /* we want the fast check and for new points we don't
       * use that check... So let's go on the full check
       * since it's still faster */
      if (e->constrained && p2tr_cdt_is_encroached (e))
        p2tr_dt_enqueue_segment (self, e);
      else if (delta (t) || p2tr_triangle_smallest_non_constrained_angle (t) < theta)
        p2tr_dt_enqueue_tri (self, t);
    }
}

static gdouble
ShortestEdgeLength (P2trTriangle *tri)
{
  gdouble a1 = p2tr_edge_get_length_squared (tri->edges[0]);
  gdouble a2 = p2tr_edge_get_length_squared (tri->edges[1]);
  gdouble a3 = p2tr_edge_get_length_squared (tri->edges[2]);
  
  return sqrt (MIN (a1, MIN (a2, a3)));
}

static gboolean
TolerantIsShorter (P2trEdge *toTest, P2trEdge *reference)
{
  return p2tr_edge_get_length(toTest) < p2tr_edge_get_length(reference) * 1.01;
}

static inline gdouble
LOG2 (gdouble v)
{
  return log10 (v) / G_LOG_2_BASE_10;
}

static gboolean
TolerantIsPowerOfTwoLength (gdouble length)
{
    gdouble exp = LOG2 (length);
    gdouble intpart, frac = modf (exp, &intpart);
    
    return ABS(frac) < 0.05;
}

static void
ChooseSplitVertex(P2trEdge *e, P2trVector2 *dst)
{
  gdouble sourceLength = p2tr_edge_get_length(e);
  gdouble newLength = pow(2, round(LOG2(sourceLength)));
  gdouble ratio, resultLength;
  
  /* IMPORTANT! DIVIDE BY 2! */
  newLength /= 2;

  ratio = newLength / sourceLength;

  dst->x = (1 - ratio) * P2TR_EDGE_START(e)->c.x + (ratio) * e->end->c.x;
  dst->y = (1 - ratio) * P2TR_EDGE_START(e)->c.y + (ratio) * e->end->c.y;

  /* now let's avoid consistency problems */
  resultLength = sqrt(P2TR_VECTOR2_DISTANCE_SQ(&P2TR_EDGE_START(e)->c, dst));
  
  if (! TolerantIsPowerOfTwoLength(resultLength))
    p2tr_exception_numeric ("Bad rounding!");
}
