#ifndef __P2TR_RENDER_MESH_RENDER_H__
#define __P2TR_RENDER_MESH_RENDER_H__

#include <glib.h>

typedef struct {
  /* Minimal X and Y coordinates to start sampling at */
  gdouble min_x, min_y;
  /* Size of a step in each axis */
  gdouble step_x, step_y;
  /* The amount of samples desired in each axis */
  guint x_samples, y_samples;
  /* The amount of channels per pixel, both in destination buffer and in the
   * colors returned from the matching point-to-color function */
  guint cpp;
} P2trImageConfig;

typedef void (*P2trPointToColorFunc) (P2trPoint* point, gfloat *dest, gpointer user_data);

typedef union {
  P2trTriangle *tri;
  gdouble       u;
  gdouble       v;
} P2truvt;

void p2tr_test_point_to_color (P2trPoint* point, gfloat *dest, gpointer user_data);

void
p2tr_mesh_render_cache_uvt (P2trMesh    *T,
                            P2truvt              *dest,
                            P2trImageConfig      *config);

/* Like the regular version, but cache only the specified amount of
 * pixels */
void
p2tr_mesh_render_cache_uvt_exact (P2trMesh    *T,
                                  P2truvt              *dest,
                                  gint                  dest_len,
                                  P2trImageConfig      *config);

void
p2tr_mesh_render_scanline (P2trMesh    *T,
                           gfloat               *dest,
                           P2trImageConfig      *config,
                           P2trPointToColorFunc  pt2col,
                           gpointer              pt2col_user_data);

void
p2tr_mesh_render_scanline2 (P2truvt              *uvt_cache,
                            gfloat               *dest,
                            P2trImageConfig      *config,
                            P2trPointToColorFunc  pt2col,
                            gpointer              pt2col_user_data);

void
p2tr_write_ppm (FILE            *f,
                gfloat          *dest,
                P2trImageConfig *config);

#endif
