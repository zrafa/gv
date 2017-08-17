#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif /* !FALSE */

#ifndef TRUE
#define TRUE 1
#endif /* !TRUE */

void error(char * msg);

/*----------------------------------------------------------------------------*/
/*------------------------------ PGM image I/O -------------------------------*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/** Skip white characters and comments in a PGM file.
 */
static void skip_whites_and_comments(FILE * f)
{
  int c;
  do
    {
      while(isspace(c=getc(f))); /* skip spaces */
      if(c=='#') /* skip comments */
        while( c!='\n' && c!='\r' && c!=EOF )
          c=getc(f);
    }
  while( c == '#' || isspace(c) );
  if( c != EOF && ungetc(c,f) == EOF )
    error("Error: unable to 'ungetc' while reading PGM file.");
}

/*----------------------------------------------------------------------------*/
/** Read a ASCII number from a PGM file.
 */
static int get_num(FILE * f)
{
  int num,c;

  while(isspace(c=getc(f)));
  if(!isdigit(c)) error("Error: corrupted PGM file.");
  num = c - '0';
  while( isdigit(c=getc(f)) ) num = 10 * num + c - '0';
  if( c != EOF && ungetc(c,f) == EOF )
    error("Error: unable to 'ungetc' while reading PGM file.");

  return num;
}

/*----------------------------------------------------------------------------*/
/** Write an int image into a PGM file.
    If the name is "-" the file is written to standard output.
 */
void write_pgm_image_int(int * image, int xsize, int ysize, char * name)
{
  FILE * f;
  int x,y,n,v,max,min;

  /* check input */
  if( image == NULL || xsize <= 0 || ysize <= 0 )
    error("Error: invalid input image to write_pgm_image_int.");

  /* check min and max values */
  max = min = 0;
  for(y=0; y<ysize; y++)
    for(x=0; x<xsize; x++)
      {
        v = image[ x + y * xsize ];
        if( v > max ) max = v;
        if( v < min ) min = v;
      }
  if( min < 0 ) fprintf(stderr,
        "Warning: write_pgm_image_int: negative values in '%s'.\n",name);
  if( max > 65535 ) fprintf(stderr,
        "Warning: write_pgm_image_int: values exceeding 65535 in '%s'.\n",name);

  /* open file */
  if( strcmp(name,"-") == 0 ) f = stdout;
  else f = fopen(name,"w");
  if( f == NULL ) error("Error: unable to open output image file.");

  /* write header */
  fprintf(f,"P2\n");
  fprintf(f,"%d %d\n",xsize,ysize);
  fprintf(f,"%d\n",max);

  /* write data */
  for(n=0,y=0; y<ysize; y++)
    for(x=0; x<xsize; x++)
      {
        fprintf(f,"%d ",image[ x + y * xsize ]);
        if(++n==8)  /* lines should not be longer than 70 characters  */
          {
            fprintf(f,"\n");
            n = 0;
          }
      }

  /* close file if needed */
  if( f != stdout && fclose(f) == EOF )
    error("Error: unable to close file while writing PGM file.");
}

/*----------------------------------------------------------------------------*/
/** Read a PGM file into an int image.
    If the name is "-" the file is read from standard input.
 */
int * read_pgm_image_int(int * X, int * Y, char * name)
{
  FILE * f;
  int c,bin = FALSE;
  int xsize,ysize,depth,x,y;
  int * image;

  /* open file */
  if( strcmp(name,"-") == 0 ) f = stdin;
  else f = fopen(name,"rb");
  if( f == NULL ) error("Error: unable to open input image file.");

  /* read header */
  if( getc(f) != 'P' ) error("Error: not a PGM file!");
  if( (c=getc(f)) == '2' ) bin = FALSE;
  else if( c == '5' ) bin = TRUE;
  else error("Error: not a PGM file!");
  skip_whites_and_comments(f);
  xsize = get_num(f);            /* X size */
  if(xsize<=0) error("Error: X size <=0, invalid PGM file\n");
  skip_whites_and_comments(f);
  ysize = get_num(f);            /* Y size */
  if(ysize<=0) error("Error: Y size <=0, invalid PGM file\n");
  skip_whites_and_comments(f);
  depth = get_num(f);            /* depth */
  if(depth<=0) fprintf(stderr,"Warning: depth<=0, probably invalid PGM file\n");
  /* white before data */
  if(!isspace(c=getc(f))) error("Error: corrupted PGM file.");

  /* get memory */
  image = (int *) calloc( (size_t) (xsize*ysize), sizeof(int) );
  if( image == NULL ) error("Error: not enough memory.");

  /* read data */
  for(y=0;y<ysize;y++)
    for(x=0;x<xsize;x++)
      image[ x + y * xsize ] = bin ? (int) getc(f)
                                   : (int) get_num(f);

  /* close file if needed */
  if( f != stdin && fclose(f) == EOF )
    error("Error: unable to close file while reading PGM file.");

  /* return image */
  *X = xsize;
  *Y = ysize;
  return image;
}


/*----------------------------------------------------------------------------*/
/** Read a PGM file into an double image.
    If the name is "-" the file is read from standard input.
 */
double * read_pgm_image_double(int * X, int * Y, char * name)
{
  FILE * f;
  int c,bin = FALSE;
  int xsize,ysize,depth,x,y;
  double * image;

  /* open file */
  if( strcmp(name,"-") == 0 ) f = stdin;
  else f = fopen(name,"rb");
  if( f == NULL ) error("Error: unable to open input image file.");

  /* read header */
  if( getc(f) != 'P' ) error("Error: not a PGM file!");
  if( (c=getc(f)) == '2' ) bin = FALSE;
  else if( c == '5' ) bin = TRUE;
  else error("Error: not a PGM file!");
  skip_whites_and_comments(f);
  xsize = get_num(f);            /* X size */
  if(xsize<=0) error("Error: X size <=0, invalid PGM file\n");
  skip_whites_and_comments(f);
  ysize = get_num(f);            /* Y size */
  if(ysize<=0) error("Error: Y size <=0, invalid PGM file\n");
  skip_whites_and_comments(f);
  depth = get_num(f);            /* depth */
  if(depth<=0) fprintf(stderr,"Warning: depth<=0, probably invalid PGM file\n");
  /* white before data */
  if(!isspace(c=getc(f))) error("Error: corrupted PGM file.");

  /* get memory */
  image = (double *) calloc( (size_t) (xsize*ysize), sizeof(double) );
  if( image == NULL ) error("Error: not enough memory.");

  /* read data */
  for(y=0;y<ysize;y++)
    for(x=0;x<xsize;x++)
      image[ x + y * xsize ] = bin ? (double) getc(f)
                                   : (double) get_num(f);

  /* close file if needed */
  if( f != stdin && fclose(f) == EOF )
    error("Error: unable to close file while reading PGM file.");

  /* return image */
  *X = xsize;
  *Y = ysize;
  return image;
}


