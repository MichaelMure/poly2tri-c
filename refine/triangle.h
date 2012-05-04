#ifndef __P2TC_REFINE_TRIANGLE_H__
#define __P2TC_REFINE_TRIANGLE_H__

#include <glib.h>
#include "math.h"
#include "triangulation.h"

/**
 * @struct P2trTriangle_
 * A struct for a triangle in a triangular mesh
 */
struct P2trTriangle_
{
  P2trEdge* edges[3];
  
  guint refcount;
};

P2trTriangle*   p2tr_triangle_new            (P2trEdge *AB,
                                              P2trEdge *BC,
                                              P2trEdge *CA);

void        p2tr_triangle_ref                (P2trTriangle *self);

void        p2tr_triangle_unref              (P2trTriangle *self);

void        p2tr_triangle_free               (P2trTriangle *self);

void        p2tr_triangle_remove             (P2trTriangle *self);

P2trMesh*   p2tr_triangle_get_mesh           (P2trTriangle *self);

gboolean    p2tr_triangle_is_removed         (P2trTriangle *self);

P2trPoint*  p2tr_triangle_get_opposite_point (P2trTriangle *self,
                                              P2trEdge     *e);

P2trEdge*   p2tr_triangle_get_opposite_edge  (P2trTriangle *self,
                                              P2trPoint    *p);

gdouble     p2tr_triangle_get_angle_at       (P2trTriangle *self,
                                              P2trPoint    *p);

gdouble     p2tr_triangle_smallest_non_constrained_angle (P2trTriangle *self);

void        p2tr_triangle_get_circum_circle (P2trTriangle *self,
                                             P2trCircle   *circle);

P2trInCircle p2tr_triangle_circumcircle_contains_point (P2trTriangle       *self,
                                                        const P2trVector2  *pt);

P2trInTriangle p2tr_triangle_contains_point  (P2trTriangle      *self,
                                              const P2trVector2 *pt);

P2trInTriangle p2tr_triangle_contains_point2 (P2trTriangle      *self,
                                              const P2trVector2 *pt,
                                              gdouble           *u,
                                              gdouble           *v);

#define P2TR_TRIANGLE_GET_POINT(tr,index) ((tr)->edges[((index)+3-1)%3]->end)
#endif