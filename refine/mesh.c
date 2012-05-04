#include <glib.h>
#include "utils.h"

#include "mesh.h"
#include "point.h"
#include "edge.h"
#include "triangle.h"

P2trMesh*
p2tr_mesh_new (void)
{
  P2trMesh *mesh = g_slice_new (P2trMesh);

  mesh->refcount = 1;
  mesh->edges = p2tr_hash_set_new_default ();
  mesh->points = p2tr_hash_set_new_default ();
  mesh->triangles = p2tr_hash_set_new_default ();

  mesh->_is_clearing_now = FALSE;

  return mesh;
}

P2trPoint*
p2tr_mesh_new_point (P2trMesh          *self,
                     const P2trVector2 *c)
{
  P2trPoint *pt = p2tr_point_new (c);

  pt->mesh = self;
  p2tr_mesh_ref (self);

  p2tr_hash_set_insert (self->points, pt);
  p2tr_point_ref (pt);

  return pt;
}

P2trEdge*
p2tr_mesh_new_edge (P2trMesh  *self,
                    P2trPoint *start,
                    P2trPoint *end,
                    gboolean   constrained)
{
  P2trEdge *ed = p2tr_edge_new (start, end, constrained);

  p2tr_hash_set_insert (self->edges, ed);
  p2tr_edge_ref (ed);

  return ed;
}

P2trEdge*
p2tr_mesh_new_or_existing_edge (P2trMesh  *self,
                                P2trPoint *start,
                                P2trPoint *end,
                                gboolean   constrained)
{
  P2trEdge *result = p2tr_point_has_edge_to (start, end);
  if (result)
    p2tr_edge_ref (result);
  else
    result = p2tr_mesh_new_edge (self, start, end, constrained);
  return result;
}

P2trTriangle*
p2tr_mesh_new_triangle (P2trMesh *self,
                        P2trEdge *AB,
                        P2trEdge *BC,
                        P2trEdge *CA)
{
  P2trTriangle *tr = p2tr_triangle_new (AB, BC, CA);

  p2tr_hash_set_insert (self->triangles, tr);
  p2tr_triangle_ref (tr);

  return tr;
}

void
p2tr_mesh_on_point_removed (P2trMesh  *self,
                            P2trPoint *point)
{
  if (self != point->mesh)
    p2tr_exception_programmatic ("Point does not belong to this mesh!");

  point->mesh = NULL;
  p2tr_mesh_unref (self);

  if (! self->_is_clearing_now)
    p2tr_hash_set_remove (self->points, point);
  p2tr_point_unref (point);
}

void
p2tr_mesh_on_edge_removed (P2trMesh *self,
                           P2trEdge *edge)
{
  if (! self->_is_clearing_now)
    p2tr_hash_set_remove (self->edges, edge);
  p2tr_edge_unref (edge);
}

void
p2tr_mesh_on_triangle_removed (P2trMesh     *self,
                               P2trTriangle *triangle)
{
  if (! self->_is_clearing_now)
    p2tr_hash_set_remove (self->triangles, triangle);
  p2tr_triangle_unref (triangle);
}

void
p2tr_mesh_clear (P2trMesh *self)
{
  P2trHashSetIter iter;
  gpointer temp;

  self->_is_clearing_now = TRUE;

  p2tr_hash_set_iter_init (&iter, self->triangles);
  while (p2tr_hash_set_iter_next (&iter, &temp))
    p2tr_triangle_remove ((P2trTriangle*)temp);
  p2tr_hash_set_remove_all (self->triangles);

  p2tr_hash_set_iter_init (&iter, self->edges);
  while (p2tr_hash_set_iter_next (&iter, &temp))
    p2tr_edge_remove ((P2trEdge*)temp);
  p2tr_hash_set_remove_all (self->edges);

  p2tr_hash_set_iter_init (&iter, self->points);
  while (p2tr_hash_set_iter_next (&iter, &temp))
    p2tr_point_remove ((P2trPoint*)temp);
  p2tr_hash_set_remove_all (self->points);

  self->_is_clearing_now = FALSE;
}

void
p2tr_mesh_free (P2trMesh *self)
{
  p2tr_mesh_clear (self);

  p2tr_hash_set_free (self->points);
  p2tr_hash_set_free (self->edges);
  p2tr_hash_set_free (self->triangles);

  g_slice_free (P2trMesh, self);
}

void
p2tr_mesh_unref (P2trMesh *self)
{
  if (--self->refcount == 0)
    p2tr_mesh_free (self);
}

void
p2tr_mesh_ref (P2trMesh *self)
{
  ++self->refcount;
}

P2trTriangle*
p2tr_mesh_find_point (P2trMesh *self,
                      const P2trVector2 *pt)
{
  gdouble u, v;
  return p2tr_mesh_find_point2 (self, pt, &u, &v);
}

P2trTriangle*
p2tr_mesh_find_point2 (P2trMesh          *self,
                       const P2trVector2 *pt,
                       gdouble           *u,
                       gdouble           *v)
{
  P2trHashSetIter iter;
  P2trTriangle *result;
  
  p2tr_hash_set_iter_init (&iter, self->triangles);
  while (p2tr_hash_set_iter_next (&iter, (gpointer*)&result))
    if (p2tr_triangle_contains_point2 (result, pt, u, v) != P2TR_INTRIANGLE_OUT)
      return result;

  return NULL;
}

P2trTriangle*
p2tr_mesh_find_point_local (P2trMesh          *self,
                            const P2trVector2 *pt,
                            P2trTriangle      *initial_guess)
{
  gdouble u, v;
  return p2tr_mesh_find_point_local2 (self, pt, initial_guess, &u, &v);
}

P2trTriangle*
p2tr_mesh_find_point_local2 (P2trMesh          *self,
                             const P2trVector2 *pt,
                             P2trTriangle      *initial_guess,
                             gdouble           *u,
                             gdouble           *v)
{
  P2trHashSet *checked_tris = p2tr_hash_set_new_default ();
  GQueue to_check;
  P2trTriangle *result = NULL;
  
  g_queue_init (&to_check);
  
  while (! g_queue_is_empty (&to_check))
    {
      P2trTriangle *tri = (P2trTriangle*) g_queue_pop_head (&to_check);
      
      p2tr_hash_set_insert (checked_tris, tri);
      if (p2tr_triangle_contains_point2 (tri, pt, u, v))
        {
          result = tri;
          break;
        }
      else
        {
          gint i;
          for (i = 0; i < 3; i++)
            {
              P2trTriangle *new_to_check = tri->edges[i]->mirror->tri;
              if (new_to_check != NULL
                  && ! p2tr_hash_set_contains (checked_tris, new_to_check))
                {
                  p2tr_hash_set_insert (checked_tris, new_to_check);
                  g_queue_push_tail (&to_check, new_to_check);
                }
            }
        }
    }

  p2tr_hash_set_free (checked_tris);
  g_queue_clear (&to_check);

  return result;
}
