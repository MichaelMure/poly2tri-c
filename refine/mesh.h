#ifndef __P2TC_REFINE_MESH_H__
#define __P2TC_REFINE_MESH_H__

#include <glib.h>
#include "vector2.h"
#include "utils.h"
#include "triangulation.h"

struct P2trMesh_
{
  P2trHashSet *triangles;
  P2trHashSet *edges;
  P2trHashSet *points;
  
  guint        refcount;
  
  gboolean     _is_clearing_now;
};

P2trMesh*     p2tr_mesh_new             (void);

P2trPoint*    p2tr_mesh_new_point       (P2trMesh          *mesh,
                                         const P2trVector2 *c);

P2trEdge*     p2tr_mesh_new_edge        (P2trMesh  *mesh,
                                         P2trPoint *start,
                                         P2trPoint *end,
                                         gboolean   constrained);

P2trTriangle* p2tr_mesh_new_triangle        (P2trMesh *mesh,
                                             P2trEdge *AB,
                                             P2trEdge *BC,
                                             P2trEdge *CA);

void          p2tr_mesh_on_point_removed    (P2trMesh  *mesh,
                                             P2trPoint *point);

void          p2tr_mesh_on_edge_removed     (P2trMesh *mesh,
                                             P2trEdge *edge);

void          p2tr_mesh_on_triangle_removed (P2trMesh     *mesh,
                                             P2trTriangle *triangle);

void          p2tr_mesh_clear           (P2trMesh *mesh);

void          p2tr_mesh_destroy         (P2trMesh *mesh);

void          p2tr_mesh_unref           (P2trMesh *mesh);

void          p2tr_mesh_ref             (P2trMesh *mesh);

#endif