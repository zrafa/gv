#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "lsd.h"
#include "process_arguments.h"

double * read_pgm_image_double(int * X, int * Y, char * name);
void write_eps( double * segs, int n, int dim, char * filename, int xsize, int ysize, double width);
void write_svg( double * segs, int n, int dim, char * filename, int xsize, int ysize, double width);
void filtro_medula(int xsize, int ysize, double *image);
void fibras(double * s, int n, int dim, int xsize, int ysize, int cota_superior, int cota_inferior, double *image);

/*----------------------------------------------------------------------------*/
/*                                    Main                                    */
/*----------------------------------------------------------------------------*/
/** Main function call
 */
int main(int argc, char ** argv)
{
  struct arguments * arg = process_arguments(USE,argc,argv);
  FILE * output;
  double * image;
  int n;
  int dim = 7;
  int regX,regY;
  int i,j;
  double scale, sigma_coef, quant, ang_th, log_eps, density_th;
  int n_bins;
  int X,Y;
  double * segs;

  /* argumento agregado por rafa para obtener la cota superior del grosor maximo permitido */
  int cota_superior = 50;
  int cota_inferior = 10;

  /* read input file */
  image = read_pgm_image_double(&X,&Y, get_str(arg,"in"));

  /* RAFA */
  filtro_medula(X, Y, image);

  scale = 0.8;
  sigma_coef = 0.6;
  quant = 2.0;
  ang_th = 100.0;
  log_eps = 0.0;
  density_th = 0.7;
  n_bins = 1024;

  /* execute LSD */
  segs = LineSegmentDetection(&n, image, X, Y,
                              scale,
                              sigma_coef,
                              quant,
                              ang_th,
                              log_eps,
                              density_th,
                              n_bins,
                              NULL,
                              &regX, &regY);

  
  fibras(segs, n, dim, X, Y, cota_superior, cota_inferior, image);

  /* output */
  if( strcmp(get_str(arg,"out"),"-") == 0 ) output = stdout;
  else output = fopen(get_str(arg,"out"),"w");
  if( output == NULL )
	{
		fprintf(stderr,"Error: unable to open ASCII output file.\n");
    exit(EXIT_FAILURE);
  } 
  for(i=0;i<n;i++)
    {
      for(j=0;j<dim;j++)
        fprintf(output,"%f ",segs[i*dim+j]);
      fprintf(output,"\n");
    }
  if( output != stdout && fclose(output) == EOF )  /* close file if needed */
	{
    fprintf(stderr,"Error: unable to close file while output file.\n");
    exit(EXIT_FAILURE);
  } 

  /* create EPS output if needed */
  if(is_assigned(arg,"epsfile"))
    write_eps(segs,n,dim,get_str(arg,"epsfile"),X,Y,get_double(arg,"width"));

  /* create SVG output if needed */
  if(is_assigned(arg,"svgfile"))
    write_svg(segs,n,dim,get_str(arg,"svgfile"),X,Y,get_double(arg,"width"));


  /* free memory */
  free( (void *) image );
  free( (void *) segs );

  return EXIT_SUCCESS;
}

