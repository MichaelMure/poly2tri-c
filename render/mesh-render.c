#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <refine/refine.h>
#include "mesh-render.h"

/* Most computations using the Barycentric Coordinates are Based on
 * http://www.blackpawn.com/texts/pointinpoly/default.html */

/* This function is simply to make sure the code is consitant */
static inline void
p2tr_triangle_barycentric_get_points (P2trTriangle  *self,
                                      P2trPoint    **A,
                                      P2trPoint    **B,
                                      P2trPoint    **C)
{
  *A = P2TR_TRIANGLE_GET_POINT(self, 0);
  *B = P2TR_TRIANGLE_GET_POINT(self, 1);
  *C = P2TR_TRIANGLE_GET_POINT(self, 2);
}

#define USE_BARYCENTRIC(u,v,A,B,C) ((A) + (v)*((B)-(A)) + (u)*((C)-(A)))

/* This function implements box logic to see if a point is contained in a
 * triangles bounding box. This is very useful for cases where there are many
 * triangles to test against a single point, and most of them aren't even near
 * it.
 *
 * Instead of finding the Xmin, Xmax, Ymin, Ymax and checking if the the point
 * is outside, just check if the point is on the SAME SIDE compared to all the
 * points of the triangle.
 * See http://lightningismyname.blogspot.com/2011/08/quickboxa-quick-point-in-triangle-test.html
 */
gboolean
p2tr_triangule_quick_box_test (P2trTriangle *self,
                               gdouble       Px,
                               gdouble       Py)
{
  P2trPoint *A = self->edges[2]->end;
  P2trPoint *B = self->edges[0]->end;
  P2trPoint *C = self->edges[1]->end;

  register gboolean xPBorder = B->c.x <= Px;
  register gboolean yPBorder = B->c.y <= Py;

  return (((A->c.x <= Px) == xPBorder) && (xPBorder == (C->c.x <= Px)))
          || (((A->c.y <= Py) == yPBorder) && (yPBorder == (C->c.y <= Py)));
}

#define uvt3_u(ptr) (((ptr)+0)->u)
#define uvt3_v(ptr) (((ptr)+1)->v)
#define uvt3_t(ptr) (((ptr)+2)->tri)

void
p2tr_mesh_render_cache_uvt (P2trMesh        *T,
                            P2truvt         *dest,
                            P2trImageConfig *config)
{
  p2tr_mesh_render_cache_uvt_exact (T, dest, config->x_samples * config->y_samples, config);
}

void
p2tr_mesh_render_cache_uvt_exact (P2trMesh        *T,
                                  P2truvt         *dest,
                                  gint             dest_len,
                                  P2trImageConfig *config)
{
  gint x, y, n = dest_len;
  P2truvt *uvt = dest;
  P2trTriangle *tr_prev = NULL;
  P2trVector2 pt;
  
  pt.x = config->min_x;
  pt.y = config->min_y;

  uvt3_t(uvt) = p2tr_mesh_find_point_local2 (T, &pt, NULL, &uvt3_u(uvt), &uvt3_v(uvt));
  tr_prev = uvt3_t(uvt);
  
  for (y = 0, pt.y = config->min_y; y < config->y_samples; y++, pt.y += config->step_y)
    for (x = 0, pt.x = config->min_x; x < config->x_samples; x++, pt.x += config->step_x)
      {
        if (n-- == 0) return;
        uvt3_t(uvt) = p2tr_mesh_find_point_local2 (T, &pt, tr_prev, &uvt3_u(uvt), &uvt3_v(uvt));
        tr_prev = uvt3_t(uvt);
        uvt += 3;
      }
}

void
p2tr_mesh_render_scanline (P2trMesh             *T,
                           gfloat               *dest,
                           P2trImageConfig      *config,
                           P2trPointToColorFunc  pt2col,
                           gpointer              pt2col_user_data)
{
  P2truvt *uvt_cache = g_new (P2truvt, 3 * config->x_samples * config->y_samples);
  GTimer *timer = g_timer_new ();
  
  g_timer_start (timer);
  p2tr_mesh_render_cache_uvt (T, uvt_cache, config);
  g_timer_stop (timer);
  g_debug ("Mesh preprocessing took %f seconds\n", g_timer_elapsed (timer, NULL));

  g_timer_start (timer);
  p2tr_mesh_render_scanline2 (uvt_cache, dest, config, pt2col, pt2col_user_data);
  g_timer_stop (timer);
  g_debug ("Mesh rendering took %f seconds\n", g_timer_elapsed (timer, NULL));

  g_timer_destroy (timer);
  g_free (uvt_cache);
  
}


void
p2tr_mesh_render_scanline2 (P2truvt              *uvt_cache,
                            gfloat               *dest,
                            P2trImageConfig      *config,
                            P2trPointToColorFunc  pt2col,
                            gpointer              pt2col_user_data)
{
  P2truvt *uvt_p = uvt_cache;

  gdouble u, v;
  P2trTriangle *tr_prev = NULL, *tr_now;

  gint x, y;

  P2trPoint *A = NULL, *B = NULL, *C = NULL;

  gfloat *col =  g_new (gfloat, config->cpp);
  gfloat *colA = g_new (gfloat, config->cpp);
  gfloat *colB = g_new (gfloat, config->cpp);
  gfloat *colC = g_new (gfloat, config->cpp);

  gfloat *pixel = dest;

  for (y = 0; y < config->y_samples; y++)
    for (x = 0; x < config->x_samples; x++)
    {
        u      = uvt3_u (uvt_p);
        v      = uvt3_v (uvt_p);
        tr_now = uvt3_t (uvt_p);
        
        uvt_p += 3;

        /* If we are outside of the triangulation, set alpha to zero and
         * continue */
        if (tr_now == NULL)
          {
            pixel[3] = 0;
            pixel += 4;
          }
        else
          {
            /* If the triangle hasn't changed since the previous pixel,
             * then don't sample the color at the vertices again, since
             * that is an expensive process! */
            if (tr_now != tr_prev)
              {
                /* Get the points of the triangle in some fixed order,
                 * just to make sure that the computation goes the same
                 * everywhere */
                p2tr_triangle_barycentric_get_points (tr_now, &A, &B, &C);
                /* At each point X sample the color into colX */
                pt2col (A, colA, pt2col_user_data);
                pt2col (B, colB, pt2col_user_data);
                pt2col (C, colC, pt2col_user_data);
                /* Set the current triangle */
                tr_now = tr_prev;
              }

            /* Interpolate the color using barycentric coodinates */
            *pixel++ = USE_BARYCENTRIC (u,v,colA[0],colB[0],colC[0]);
            *pixel++ = USE_BARYCENTRIC (u,v,colA[1],colB[1],colC[1]);
            *pixel++ = USE_BARYCENTRIC (u,v,colA[2],colB[2],colC[2]);
            /* Finally, set as opaque since we are inside the mesh */
            *pixel++ = 1;
          }
    }
}

void
p2tr_write_ppm (FILE            *f,
                gfloat          *dest,
                P2trImageConfig *config)
{
  gint x, y;
  fprintf (f, "P3\n");
  fprintf (f, "%d %d\n", config->x_samples, config->y_samples);
  fprintf (f, "255\n");

  gfloat *pixel = dest;

  for (y = 0; y < config->y_samples; y++)
    {
      for (x = 0; x < config->x_samples; x++)
        {
          if (pixel[3] <= 0.5)
            fprintf (f, "  0   0   0");
          else
            fprintf (f, "%3d %3d %3d", (guchar)(pixel[0] * 255), (guchar)(pixel[1] * 255), (guchar)(pixel[2] * 255));

          if (x != config->x_samples - 1)
            fprintf (f, "   ");

          pixel += 4;
        }
      fprintf (f, "\n");
    }
}
