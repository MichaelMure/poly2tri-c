#ifndef __P2TC_REFINE_POINT_H__
#define __P2TC_REFINE_POINT_H__

#include <glib.h>
#include "vector2.h"
#include "triangulation.h"

/**
 * @struct P2trPoint_
 * A struct for a point in a triangular mesh
 */
struct P2trPoint_
{
  /** The 2D coordinates of the point */
  P2trVector2  c;

  /**
   * A list of edges (@ref P2trEdge) which go out of this point (i.e.
   * the point is their start point). The edges are sorted by ASCENDING
   * angle, meaning they are sorted Counter Clockwise */
  GList       *outgoing_edges;

  /** A count of references to the point */
  guint        refcount;
  
  /** The triangular mesh containing this point */
  P2trMesh    *mesh;
};

P2trPoint*  p2tr_point_new                  (const P2trVector2 *c);

void        p2tr_point_ref                  (P2trPoint *self);

void        p2tr_point_unref                (P2trPoint *self);

void        p2tr_point_free                 (P2trPoint *self);

void        p2tr_point_remove               (P2trPoint *self);

P2trEdge*   p2tr_point_get_edge_to          (P2trPoint *start,
                                             P2trPoint *end);

void        _p2tr_point_insert_edge         (P2trPoint *self,
                                             P2trEdge  *e);

void        _p2tr_point_remove_edge         (P2trPoint *self,
                                             P2trEdge  *e);

P2trEdge*   p2tr_point_edge_ccw             (P2trPoint *self,
                                             P2trEdge  *e);

P2trEdge*   p2tr_point_edge_cw              (P2trPoint *self,
                                             P2trEdge  *e);

gboolean    p2tr_point_is_fully_in_domain   (P2trPoint *self);

gboolean    p2tr_point_has_constrained_edge (P2trPoint *self);

P2trMesh*   p2tr_point_get_mesh             (P2trPoint *self);

#endif