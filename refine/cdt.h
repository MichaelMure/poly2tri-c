#ifndef __P2TC_REFINE_CDT_H__
#define __P2TC_REFINE_CDT_H__

#include <p2t/poly2tri.h>
#include "mesh.h"
#include "pslg.h"

typedef struct
{
  P2trMesh *mesh;
  P2trPSLG *outline;
} P2trCDT;

P2trCDT*    p2tr_cdt_new (P2tCDT *cdt);

gboolean    p2tr_cdt_visible_from_edge (P2trCDT     *self,
                                        P2trEdge    *e,
                                        P2trVector2 *p);

void        p2tr_cdt_validate_edges    (P2trCDT *self);

void        p2tr_cdt_validate_cdt            (P2trCDT *self);

P2trPoint*  p2tr_cdt_insert_point (P2trCDT           *self,
                                   const P2trVector2 *pc,
                                   P2trTriangle      *point_location_guess);

void        p2tr_cdt_insert_point_into_triangle (P2trCDT      *self,
                                                 P2trPoint    *pt,
                                                 P2trTriangle *tri);

GList*      p2tr_cdt_split_edge (P2trCDT   *self,
                                 P2trEdge  *e,
                                 P2trPoint *C);

#endif