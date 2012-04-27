#ifndef __P2TC_REFINE_VISIBILITY_H__
#define __P2TC_REFINE_VISIBILITY_H__

#include <glib.h>

#include "bounded-line.h"
#include "vector2.h"
#include "pslg.h"

gboolean  p2tr_visibility_is_visible_from_edges (P2trPSLG              *pslg,
                                                 P2trVector2           *p,
                                                 const P2trBoundedLine *lines,
                                                 guint                  line_count);
#endif