#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

void error(char * msg);

/*----------------------------------------------------------------------------*/
/*----------------------------- Write SVG File -------------------------------*/
/*----------------------------------------------------------------------------*/
/** Write line segments into a SVG file.
    If the name is "-" the file is written to standard output.
*/
void write_svg( double * segs, int n, int dim, char * filename, int xsize, int ysize, double width )
{
  FILE * svg;
  int i;

  /* check input */
  if( segs == NULL || n < 0 || dim <= 0 )
    error("Error: invalid line segment list in write_svg.");
  if( xsize <= 0 || ysize <= 0 )
    error("Error: invalid image size in write_svg.");

  /* open file */
  if( strcmp(filename,"-") == 0 ) svg = stdout;
  else svg = fopen(filename,"w");
  if( svg == NULL ) error("Error: unable to open SVG output file.");

  /* write SVG header */
  fprintf(svg,"<?xml version=\"1.0\" standalone=\"no\"?>\n");
  fprintf(svg,"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n");
  fprintf(svg," \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
  fprintf(svg,"<svg width=\"%dpx\" height=\"%dpx\" ",xsize,ysize);
  fprintf(svg,"version=\"1.1\"\n xmlns=\"http://www.w3.org/2000/svg\" ");
  fprintf(svg,"xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n");

  /* write line segments */
  for(i=0;i<n;i++)
  {
      fprintf(svg,"<line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" ",
              segs[i*dim+0],segs[i*dim+1],
              segs[i*dim+2],segs[i*dim+3]);
      fprintf(svg,"stroke-width=\"%f\" stroke=\"black\" />\n",
              width <= 0.0 ? segs[i*dim+4] : width);
  }

  /* close SVG file */
  fprintf(svg,"</svg>\n");
  if( svg != stdout && fclose(svg) == EOF )
    error("Error: unable to close file while writing SVG file.");
}

