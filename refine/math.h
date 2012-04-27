#ifndef __P2TC_REFINE_MATH_H__
#define __P2TC_REFINE_MATH_H__

#include <glib.h>
#include "vector2.h"
#include "circle.h"

gdouble   p2tr_math_length_sq  (gdouble x1,
                                gdouble y1,
                                gdouble x2,
                                gdouble y2);

gdouble   p2tr_math_length_sq2 (const P2trVector2 *pt1,
                                const P2trVector2 *pt2);


/**
 * Find the circumscribing circle of a triangle defined by the given
 * points.
 * @param[in] A The first vertex of the triangle
 * @param[in] B The second vertex of the triangle
 * @param[in] C The third vertex of the triangle
 * @param[out] circle The circumscribing circle of the triangle
 */
void      p2tr_math_triangle_circumcircle (const P2trVector2 *A,
                                           const P2trVector2 *B,
                                           const P2trVector2 *C,
                                           P2trCircle    *circle);

typedef enum
{
  P2TR_INTRIANGLE_OUT = -1,
  P2TR_INTRIANGLE_ON = 0,
  P2TR_INTRIANGLE_IN = 1
} P2trInTriangle;

/**
 * Return the barycentric coordinates of a point inside a triangle. This
 * means that the computation returns @ref u and @ref v so that the
 * following equation is satisfied:
 * {{{ AP = u * AB + v * AC }}}
 *
 * @param[in] A The first point of the triangle
 * @param[in] B The second point of the triangle
 * @param[in] C The third point of the triangle
 * @param[in] P The point whose barycentric coordinates should be
 *              computed
 * @param[out] u The first barycentric coordinate
 * @param[out] v The second barycentric coordinate
 */
void p2tr_math_triangle_barcycentric (const P2trVector2 *A,
                                      const P2trVector2 *B,
                                      const P2trVector2 *C,
                                      const P2trVector2 *P,
                                      gdouble           *u,
                                      gdouble           *v);

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