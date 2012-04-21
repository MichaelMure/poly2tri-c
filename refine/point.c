#include <glib.h>
#include "point.h"
#include "edge.h"

P2trPoint*
p2tr_point_new (const P2trVector2 *c)
{
  P2trPoint *self = g_slice_new (P2trPoint);
  
  p2tr_vector2_copy (&self->c, c);
  self->outgoing_edges = NULL;
  
  return self;
}

static void
p2tr_point_delete (P2trPoint *self)
{
  GList *iter;
  for (iter = self->outgoing_edges; iter != NULL; iter = iter->next)
    p2tr_edge_unref ((P2trEdge*) iter->data);

  g_list_free (self->outgoing_edges);
  g_slice_free (P2trPoint, self);
}

static P2trEdge*
_p2tr_point_existing_edge_to (P2trPoint* self, P2trPoint *end)
{
  GList *iter;
  
  for (iter = self->outgoing_edges; iter != NULL; iter = iter->next)
    {
      P2trEdge *edge = (P2trEdge*) iter->data;
      if (edge->end == end)
        return edge;
    }
  
  return NULL;
}

P2trEdge*
p2tr_point_get_edge_to (P2trPoint *start,
                        P2trPoint *end)
{
    P2trEdge* result = _p2tr_point_existing_edge_to (start, end);
    if (result == NULL)
      p2tr_exception_programmatic ("Tried to get an edge that doesn't exist!");
    else
      return result;
}

void
_p2tr_point_insert_edge (P2trPoint *self, P2trEdge *e)
{
  GList *iter = self->outgoing_edges;
  
  /* Remember: Edges are sorted in ASCENDING angle! */
  while (iter != NULL && ((P2trEdge*)iter->data)->angle < e->angle)
    iter = iter->next;

  self->outgoing_edges =
      g_list_insert_before (self->outgoing_edges, iter, e);

  p2tr_edge_ref (e);
}

void
_p2tr_point_remove_edge (P2trPoint *self, P2trEdge* e)
{
  GList *node;
  
  if (P2TR_EDGE_START(e) != self)
    p2tr_exception_programmatic ("Could not remove the given outgoing "
        "edge because doesn't start on this point!");

  node = g_list_find (self->outgoing_edges, e);
  if (node == NULL)
    p2tr_exception_programmatic ("Could not remove the given outgoing "
        "edge because it's not present in the outgoing-edges list!");

  self->outgoing_edges = g_list_delete_link (self->outgoing_edges, node);

  p2tr_edge_unref (e);
}

P2trEdge*
p2tr_point_edge_ccw (P2trPoint *self,
                     P2trEdge  *e)
{
  GList *node;

  if (P2TR_EDGE_START(e) != self)
      p2tr_exception_programmatic ("Not an edge of this point!");

  node = g_list_find (self->outgoing_edges, e);
  if (node == NULL)
    p2tr_exception_programmatic ("Could not find the CCW sibling edge"
        "because the edge is not present in the outgoing-edges list!");

    return (P2trEdge*) g_list_cyclic_next (self->outgoing_edges, node);
}

P2trEdge*
p2tr_point_edge_cw (P2trPoint* self,
                    P2trEdge *e)
{
  GList *node;

  if (P2TR_EDGE_START(e) != self)
      p2tr_exception_programmatic ("Not an edge of this point!");

  node = g_list_find (self->outgoing_edges, e);
  if (node == NULL)
    p2tr_exception_programmatic ("Could not find the CW sibling edge"
        "because the edge is not present in the outgoing-edges list!");

    return (P2trEdge*) g_list_cyclic_prev (self->outgoing_edges, node);
}

gboolean
p2tr_point_is_fully_in_domain (P2trPoint *self)
{
  GList *iter;
  for (iter = self->outgoing_edges; iter != NULL; iter = iter->next)
    if (((P2trEdge*) iter->data)->tri == NULL)
      return FALSE;
      
  return TRUE;
}

gboolean
p2tr_point_has_constrained_edge (P2trPoint *self)
{
  GList *iter;
  for (iter = self->outgoing_edges; iter != NULL; iter = iter->next)
    if (((P2trEdge*) iter->data)->constrained)
      return TRUE;
      
  return FALSE;
}

void
p2tr_point_ref (P2trPoint *self)
{
  ++self->refcount;
}

void
p2tr_point_unref (P2trPoint *self)
{
  if (--self->refcount == 0)
    p2tr_point_delete (self);
}