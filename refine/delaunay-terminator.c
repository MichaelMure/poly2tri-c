#include <glib.h>

#include "utils.h"
#include "math.h"

#include "point.h"
#include "edge.h"
#include "triangle.h"

#include "mesh.h"
#include "cdt.h"

#include "delaunay-terminator.h"

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