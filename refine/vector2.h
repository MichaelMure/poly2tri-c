#ifndef __P2TR_REFINE_VECTOR2_H__
#define __P2TR_REFINE_VECTOR2_H__

#include <glib.h>

/**
 * \struct P2trVector2
 * A struct for representing a vector with 2 coordinates (a point in 2D)
 */
typedef struct {
  /** The first coordinate of the vector */
  gdouble x;
  /** The second coordinate of the vector */
  gdouble y;
} P2trVector2;

#define P2TR_VECTOR2_LEN_SQ2(X, Y)              \
    ((X) * (X) + (Y) * (Y))

#define P2TR_VECTOR2_LEN_SQ(V)                  \
    (P2TR_VECTOR2_LEN_SQ2((V)->x, (V)->y))

#define P2TR_VECTOR2_DOT(V1,V2)                 \
    ((V1)->x * (V2)->x + (V1)->y * (V2)->y)

#define P2TR_VECTOR2_DISTANCE_SQ2(X1,Y1,X2,Y2)  \
    (P2TR_VECTOR2_LEN_SQ2((X1) - (X2), (Y1) - (Y2)))

#define P2TR_VECTOR2_DISTANCE_SQ(V1,V2)         \
    (P2TR_VECTOR2_DISTANCE_SQ2((V1)->x, (V1)->y, (V2)->x, (V2)->y))

/** Compute the dot product of two vectors */
gdouble       p2tr_vector2_dot       (const P2trVector2 *a, const P2trVector2 *b);

/** Check if all the coordinates of the two vectors are the same */
gboolean      p2tr_vector2_is_same   (const P2trVector2 *a, const P2trVector2 *b);

/**
 * Compute the difference of two vectors
 * @param[in] a The vector to subtract from
 * @param[in] b The vector to be subtracted
 * @param[out] dest The vector in which the result should be stored
 */
void          p2tr_vector2_sub       (const P2trVector2 *a, const P2trVector2 *b, P2trVector2 *dest);

/**
 * Compute the center point of the edge defined between two points
 * @param[in] a The first side of the edge
 * @param[in] b The second side of the edge
 * @param[out] dest The vector in which the result should be stored
 */
void          p2tr_vector2_center    (const P2trVector2 *a, const P2trVector2 *b, P2trVector2 *dest);

/**
 * Compute the norm of a vector (the length of the line from the origin
 * to the 2D point it represents)
 * @param[in] v The vector whose norm should be computed
 * @return The norm of the vector
 */
gdouble       p2tr_vector2_norm      (const P2trVector2 *v);

/**
 * Copy a vector
 * @param[out] dest The destination of the copy operation
 * @param[in] src The vector to copy
 */
void          p2tr_vector2_copy      (P2trVector2 *dest, const P2trVector2 *src);

#endif