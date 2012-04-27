#ifndef __P2TC_REFINE_CLUSTER_H__
#define __P2TC_REFINE_CLUSTER_H__

#include <glib.h>
#include "point.h"
#include "edge.h"

#define P2TR_CLUSTER_LIMIT_ANGLE (G_PI / 6)

typedef struct
{
  GQueue   edges;
  gdouble  min_angle;
} P2trCluster;

P2trCluster*  p2tr_cluster_get_for              (P2trPoint *P,
                                                 P2trEdge  *E);

gdouble       p2tr_cluster_shortest_edge_length (P2trCluster *self);

void          p2tr_cluster_free                 (P2trCluster *self);

#endif