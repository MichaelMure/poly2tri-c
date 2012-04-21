#ifndef __P2TC_REFINE_MESH_H__
#define __P2TC_REFINE_MESH_H__

#include <glib.h>
#include "utils.h"
#include "triangulation.h"

struct P2trMesh_
{
  P2trHashSet *triangles;
  P2trHashSet *edges;
  P2trHashSet *points;
};

P2trMesh*     p2tr_mesh_new             (void);

P2trPoint*    p2tr_mesh_new_point       (P2trMesh          *mesh,
                                         const P2trVector2 *c);

P2trEdge*     p2tr_mesh_new_edge        (P2trMesh  *mesh,
                                         P2trPoint *A,
                                         P2trPoint *B,
                                         gboolean   constrained);

P2trTriangle* p2tr_mesh_new_triangle    (P2trMesh  *mesh,
                                         P2trPoint *A,
                                         P2trPoint *B,
                                         P2trPoint *C);

P2trTriangle* p2tr_mesh_new_triangle2   (P2trMesh *mesh,
                                         P2trEdge *AB,
                                         P2trEdge *BC,
                                         P2trEdge *CA);

gboolean      p2tr_mesh_remove_point    (P2trMesh *mesh,
                                         P2trEdge *edge);

gboolean      p2tr_mesh_remove_edge     (P2trMesh *mesh,
                                         P2trEdge *edge);

gboolean      p2tr_mesh_remove_triangle (P2trMesh     *mesh,
                                         P2trTriangle *triangle);

void          p2tr_mesh_clear           (P2trMesh *mesh);

void          p2tr_mesh_destroy         (P2trMesh *mesh);

#endif