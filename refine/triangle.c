#include <math.h>
#include <glib.h>

#include "utils.h"
#include "math.h"

#include "point.h"
#include "edge.h"
#include "triangle.h"
#include "mesh.h"

P2trTriangle*
p2tr_triangle_new (P2trEdge *AB,
                   P2trEdge *BC,
                   P2trEdge *CA)
{
  gint i;
  P2trTriangle *self = g_slice_new (P2trTriangle);

#ifndef P2TC_NO_LOGIC_CHECKS
  if (AB->end != P2TR_EDGE_START(BC)
      || BC->end != P2TR_EDGE_START(CA)
      || CA->end != P2TR_EDGE_START(AB))
    {
      p2tr_exception_programmatic ("Unexpected edge sequence for a triangle!");
    }

  if (AB == BC->mirror || BC == CA->mirror || CA == AB->mirror)
    {
      p2tr_exception_programmatic ("Repeated edge in a triangle!");
    }
#endif

  switch (p2tr_math_orient2d(&CA->end->c, &AB->end->c, &BC->end->c))
    {
      case P2TR_ORIENTATION_CCW:
        self->edges[0] = CA->mirror;
        self->edges[1] = BC->mirror;
        self->edges[2] = AB->mirror;
        break;

      case P2TR_ORIENTATION_CW:
        self->edges[0] = AB;
        self->edges[1] = BC;
        self->edges[2] = CA;
        break;

      case P2TR_ORIENTATION_LINEAR:
        p2tr_exception_geometric ("Can't make a triangle of linear points!");
    }

#ifdef P2TC_DEBUG_CHECKS
  if (p2tr_math_orient2d (&self->edges[0]->end.c,
                          &self->edges[1]->end.c,
                          &self->edges[2]->end.c) != P2TR_ORIENTATION_CW)
    {
      p2tr_exception_programmatic ("Bad ordering!");
    }
#endif

  for (i = 0; i < 3; i++)
    {
#ifdef P2TC_DEBUG_CHECKS
      if (self->edges[i]->tri != NULL)
        p2tr_exception_programmatic ("This edge is already in use by "
            "another triangle!");
#endif
      self->edges[i]->tri = self;
      p2tr_edge_ref (self->edges[i]);
    }

  /* Reference by 3 edges, and another for the return of this pointer */
  self->refcount = 4;

  return self;
}

void
p2tr_triangle_ref (P2trTriangle *self)
{
  ++self->refcount;
}

void
p2tr_triangle_unref (P2trTriangle *self)
{
  if (--self->refcount == 0)
    p2tr_triangle_free (self);
}

void
p2tr_triangle_free (P2trTriangle *self)
{
  p2tr_triangle_remove (self);
  g_slice_free (P2trTriangle, self);
}

void
p2tr_triangle_remove (P2trTriangle *self)
{
  gint i;
  P2trMesh *mesh;
  
  if (p2tr_triangle_is_removed (self))
    return;

  mesh = p2tr_triangle_get_mesh (self);
  
  for (i = 0; i < 3; i++)
  {
    self->edges[i]->tri = NULL;
    p2tr_triangle_unref (self);

    p2tr_edge_unref (self->edges[i]);
    self->edges[i] = NULL;
  }

  if (mesh != NULL)
    p2tr_mesh_on_triangle_removed (mesh, self);
}

P2trMesh*
p2tr_triangle_get_mesh (P2trTriangle *self)
{
  if (self->edges[0] != NULL)
    return p2tr_edge_get_mesh (self->edges[0]);
  else
    return NULL;
}

gboolean 
p2tr_triangle_is_removed (P2trTriangle *self)
{
  return self->edges[0] == NULL;
}

P2trPoint*
p2tr_triangle_get_opposite_point (P2trTriangle *self,
                                  P2trEdge     *e)
{
  if (self->edges[0] == e || self->edges[0]->mirror == e)
    return self->edges[1]->end;
  if (self->edges[1] == e || self->edges[1]->mirror == e)
    return self->edges[2]->end;
  if (self->edges[2] == e || self->edges[2]->mirror == e)
    return self->edges[0]->end;

  p2tr_exception_programmatic ("The edge is not in the triangle!");
}

P2trEdge*
p2tr_triangle_get_opposite_edge (P2trTriangle *self,
                                 P2trPoint    *p)
{
  if (self->edges[0]->end == p)
    return self->edges[2];
  if (self->edges[1]->end == p)
    return self->edges[0];
  if (self->edges[2]->end == p)
    return self->edges[1];

  p2tr_exception_programmatic ("The point is not in the triangle!");
}

/**
 * Angles return by this function are always in the range [0,180]
 */
gdouble
p2tr_triangle_get_angle_at (P2trTriangle *self,
                            P2trPoint    *p)
{
  if (p == self->edges[0]->end)
    return p2tr_edge_angle_between (self->edges[0], self->edges[1]);
  else if (p == self->edges[1]->end)
    return p2tr_edge_angle_between (self->edges[1], self->edges[2]);
  else if (p == self->edges[2]->end)
    return p2tr_edge_angle_between (self->edges[2], self->edges[0]);

  p2tr_exception_programmatic ("Can't find the point!");
}

gdouble
p2tr_triangle_smallest_non_constrained_angle (P2trTriangle *self)
{
    gdouble result = G_MAXDOUBLE, angle;
    
    if (! self->edges[0]->constrained || !self->edges[1]->constrained)
      {
        angle = p2tr_edge_angle_between(self->edges[0], self->edges[1]);
        result = MIN(result, angle);
      }

    if (!self->edges[1]->constrained || !self->edges[2]->constrained)
      {
        angle = p2tr_edge_angle_between(self->edges[1], self->edges[2]);
        result = MIN(result, angle);
      }

    if (!self->edges[2]->constrained || !self->edges[0]->constrained)
      {
        angle = p2tr_edge_angle_between(self->edges[2], self->edges[0]);
        result = MIN(result, angle);
      }

    return result;
}

void
p2tr_triangle_get_circum_circle (P2trTriangle *self,
                                 P2trCircle   *circle)
{
  p2tr_math_triangle_circumcircle (
      &self->edges[0]->end->c,
      &self->edges[1]->end->c,
      &self->edges[2]->end->c,
      circle);
}

P2trInCircle
p2tr_triangle_circumcircle_contains_point (P2trTriangle *self,
                                           P2trVector2  *pt)
{
  return p2tr_math_incircle (
      &self->edges[0]->end->c,
      &self->edges[1]->end->c,
      &self->edges[2]->end->c,
      pt);
}
