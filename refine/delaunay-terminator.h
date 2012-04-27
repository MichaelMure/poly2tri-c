#ifndef __P2TC_REFINE_DELAUNAY_TERMINATOR_H__
#define __P2TC_REFINE_DELAUNAY_TERMINATOR_H__

#include <glib.h>
#include "cdt.h"

typedef gboolean (*P2trTriangleTooBig) (P2trTriangle *tri);

typedef struct
{
  P2trCDT            *mesh;
  GQueue              Qs;
  GSequence          *Qt;
  gdouble             theta;
  P2trTriangleTooBig  delta;
} P2trDelaunayTerminator;

gboolean  p2tr_cdt_test_encroachment_ignore_visibility (const P2trVector2 *w,
                                                        P2trEdge          *e);

gboolean  p2tr_cdt_is_encroached_by (P2trCDT     *self,
                                     P2trEdge    *e,
                                     P2trVector2 *p);

P2trHashSet*  p2tr_cdt_get_segments_encroached_by (P2trCDT     *self,
                                                   P2trVector2 *C);

gboolean      p2tr_cdt_is_encroached (P2trEdge *E);

P2trDelaunayTerminator*
p2tr_dt_new (gdouble theta, P2trTriangleTooBig delta, P2trCDT *cdt);

#endif