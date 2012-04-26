#ifndef __P2TC_REFINE_TRIANGLE_H__
#define __P2TC_REFINE_TRIANGLE_H__

#include <glib.h>
#include "triangulation.h"

/**
 * @struct P2trTriangle_
 * A struct for a triangle in a triangular mesh
 */
struct P2trTriangle_
{
  /** The end point of this mesh */
  P2trPoint    *end;

  /** The edge going in the opposite direction from this edge */
  P2trEdge     *mirror;
  
  /** Is this a constrained edge? */
  gboolean      constrained;
  
  /** The triangle where this edge goes clockwise along its outline */
  P2trTriangle *tri;

  /**
   * The angle of the direction of this edge. Although it can be
   * computed anytime using atan2 on the vector of this edge, we cache
   * it here since it's heavily used and the computation is expensive.
   * The angle increases as we go CCW, and it's in the range [-PI, +PI]
   */
  gdouble       angle;
  
  /**
   * Is this edge a delaunay edge? This field is used by the refinement
   * algorithm and should not be used elsewhere!
   */
  gboolean      delaunay;

  /** A count of references to the edge */
  guint         refcount;
  
  /**
   * Is this edge still present in the triangulation? Or maybe it is
   * just pending for the last unref?
   */
  gboolean      removed;
};

#define P2TR_EDGE_START(E) ((E)->mirror->end)

void p2tr_edge_ref    (P2trEdge *self);
void p2tr_edge_unref  (P2trEdge *self);

void p2tr_edge_remove (P2trEdge *self);
#endif