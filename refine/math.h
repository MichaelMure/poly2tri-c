#ifndef __P2TC_REFINE_MATH_H__
#define __P2TC_REFINE_MATH_H__

#include <glib.h>
#include "vector2.h"

gdouble   p2tr_math_length_sq  (gdouble x1,
                                gdouble y1,
                                gdouble x2,
                                gdouble y2);

gdouble   p2tr_math_length_sq2 (const P2trVector2 *pt1,
                                const P2trVector2 *pt2);

typedef enum
{
  P2TR_INTRIANGLE_OUT = -1,
  P2TR_INTRIANGLE_ON = 0,
  P2TR_INTRIANGLE_IN = 1
} P2trInTriangle;

P2trInTriangle p2tr_math_intriangle (const P2trVector2 *A,
                                     const P2trVector2 *B,
                                     const P2trVector2 *C,
                                     const P2trVector2 *P);

typedef enum
{
  P2TR_ORIENTATION_CW = -1,
  P2TR_ORIENTATION_LINEAR = 0,
  P2TR_ORIENTATION_CCW = 1
} P2trOrientation;

P2trOrientation p2tr_math_orient2d (const P2trVector2 *A,
                                    const P2trVector2 *B,
                                    const P2trVector2 *C);

typedef enum
{
  P2TR_INCIRCLE_IN,
  P2TR_INCIRCLE_ON,
  P2TR_INCIRCLE_OUT
} P2trInCircle;

P2trInCircle p2tr_math_incircle (const P2trVector2 *A,
                                 const P2trVector2 *B,
                                 const P2trVector2 *C,
                                 const P2trVector2 *D);

#endif