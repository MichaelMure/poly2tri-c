#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <glib.h>

#include <refine/triangulation.h>

#include "svg-plot.h"

void
p2tr_plot_svg_plot_group_start (const gchar *Name, FILE *outfile)
{
  if (Name == NULL)
    fprintf (outfile, "<g>" "\n");
  else
    fprintf (outfile, "<g name=\"%s\">" "\n", Name);
}

void
p2tr_plot_svg_plot_group_end (FILE *outfile)
{
  fprintf (outfile, "</g>" "\n");
}

void
p2tr_plot_svg_plot_line (gdouble x1, gdouble y1, gdouble x2, gdouble y2, const gchar *color, FILE *outfile)
{
  fprintf (outfile, "<line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\"" "\n", x1, y1, x2, y2);
  fprintf (outfile, "style=\"stroke: %s; stroke-width: %f\" />" "\n", color, PLOT_LINE_WIDTH);
  fprintf (outfile, "" "\n");
}

void
p2tr_plot_svg_plot_arrow (gdouble x1, gdouble y1, gdouble x2, gdouble y2, const gchar* color, FILE *outfile)
{
  p2tr_plot_svg_plot_line (x1, y1, x2, y2, color, outfile);

  gdouble dy = y2 - y1;
  gdouble dx = x2 - x1;
  gdouble angle = atan2 (dy, dx);

  gdouble temp = angle - ARROW_SIDE_ANGLE;
  p2tr_plot_svg_plot_line (x2, y2, x2 - ARROW_HEAD_SIZE * cos (temp), y2 - ARROW_HEAD_SIZE * sin (temp), color, outfile);

  temp = angle + ARROW_SIDE_ANGLE;
  p2tr_plot_svg_plot_line (x2, y2, x2 - ARROW_HEAD_SIZE * cos (temp), y2 - ARROW_HEAD_SIZE * sin (temp), color, outfile);
}

void
p2tr_plot_svg_fill_triangle (gdouble x1, gdouble y1, gdouble x2, gdouble y2, gdouble x3, gdouble y3, const gchar *color, FILE *outfile)
{
  fprintf (outfile, "<polyline points=\"%f,%f %f,%f %f,%f\"" "\n", x1, y1, x2, y2, x3, y3);
  fprintf (outfile, "style=\"fill: %s\" />" "\n", color);
  fprintf (outfile, "" "\n");
}

void
p2tr_plot_svg_fill_point (gdouble x1, gdouble y1, const gchar* color, FILE *outfile)
{
  fprintf (outfile, "<circle cx=\"%f\" cy=\"%f\" r=\"%f\"" "\n", x1, y1, MAX (1, PLOT_LINE_WIDTH));
  fprintf (outfile, "style=\"fill: %s; stroke: none\" />" "\n", color);
  fprintf (outfile, "" "\n");
}

void
p2tr_plot_svg_plot_circle (gdouble xc, gdouble yc, gdouble R, const gchar* color, FILE *outfile)
{
  fprintf (outfile, "<circle cx=\"%f\" cy=\"%f\" r=\"%f\"" "\n", xc, yc, R);
  fprintf (outfile, "style=\"stroke: %s; stroke-width: %f; fill: none\" />" "\n", color, PLOT_LINE_WIDTH);
  fprintf (outfile, "" "\n");
}

void
p2tr_plot_svg_plot_end (FILE *outfile)
{
  fprintf (outfile, "</g>" "\n");
  fprintf (outfile, "</svg>" "\n");
}

void
p2tr_plot_svg_plot_init (FILE *outfile)
{
  fprintf (outfile, "<?xml version=\"1.0\" standalone=\"no\"?>" "\n");
  fprintf (outfile, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"" "\n");
  fprintf (outfile, "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" "\n");
  fprintf (outfile, "<svg width=\"100%%\" height=\"100%%\" version=\"1.1\"" "\n");
  fprintf (outfile, "xmlns=\"http://www.w3.org/2000/svg\">" "\n");
  fprintf (outfile, "" "\n");
  fprintf (outfile, "<defs>" "\n");
  fprintf (outfile, "  <marker id=\"arrow\" viewBox=\"0 0 10 10\" refX=\"10\" refY=\"5\"" "\n");
  fprintf (outfile, "     markerUnits=\"strokeWidth\" orient=\"auto\"" "\n");
  fprintf (outfile, "     markerWidth=\"12\" markerHeight=\"9\">" "\n");
  fprintf (outfile, "" "\n");
  fprintf (outfile, "     <polyline points=\"0,0 10,5 0,10\" fill=\"none\" stroke-width=\"2px\" stroke=\"inherit\" />" "\n");
  fprintf (outfile, "  </marker>" "\n");
  fprintf (outfile, "</defs>" "\n");
  fprintf (outfile, "" "\n");
  fprintf (outfile, "<g transform=\"translate(%f,%f)  scale(%f,-%f)\">" "\n", X_TRANSLATE, Y_TRANSLATE, X_SCALE, Y_SCALE);

  p2tr_plot_svg_plot_arrow (-20, 0, 100, 0, "black", outfile);
  p2tr_plot_svg_plot_arrow (0, -20, 0, 100, "black", outfile);
}

void
p2tr_plot_svg_plot_edge (P2trEdge *self, const gchar* color, FILE* outfile)
{
  gdouble x1 = P2TR_EDGE_START (self)->c.x;
  gdouble y1 = P2TR_EDGE_START (self)->c.y;
  gdouble x2 = self->end->c.x;
  gdouble y2 = self->end->c.y;

  p2tr_plot_svg_plot_line (x1, y1, x2, y2, color, outfile);

//  if (p2tr_edge_is_encroached (self))
//    p2tr_plot_svg_plot_circle ((x1 + x2) / 2, (y1 + y2) / 2, R, "red", outfile);
}

void
p2tr_plot_svg_plot_triangle (P2trTriangle *self, const gchar* color, FILE* outfile)
{
  P2trCircle c;
  p2tr_triangle_get_circum_circle (self, &c);
  p2tr_plot_svg_plot_edge (self->edges[0], color, outfile);
  p2tr_plot_svg_plot_edge (self->edges[1], color, outfile);
  p2tr_plot_svg_plot_edge (self->edges[2], color, outfile);
  p2tr_plot_svg_plot_circle (c.center.x, c.center.y, c.radius, "green", outfile);
  p2tr_plot_svg_fill_point (self->edges[0]->end->c.x, self->edges[0]->end->c.y, "blue", outfile);
  p2tr_plot_svg_fill_point (self->edges[1]->end->c.x, self->edges[1]->end->c.y, "blue", outfile);
  p2tr_plot_svg_fill_point (self->edges[2]->end->c.x, self->edges[2]->end->c.y, "blue", outfile);
}

void
p2tr_plot_svg (P2trMesh *T, FILE *outfile)
{
  P2trHashSetIter  siter;
  P2trTriangle    *tr;

  g_debug ("Starting to write SVG output\n");
  p2tr_plot_svg_plot_init (outfile);

  p2tr_hash_set_iter_init (&siter, T->triangles);
  while (p2tr_hash_set_iter_next (&siter, (gpointer*)&tr))
    p2tr_plot_svg_plot_triangle (tr, "black", outfile);

  p2tr_plot_svg_plot_end (outfile);
  g_debug ("Finished writing SVG output\n");
}
