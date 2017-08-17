#ifndef PROCESS_ARGUMENTS_HEADER
#define PROCESS_ARGUMENTS_HEADER

/*----------------------------------------------------------------------------*/
/** Definition of the command line interface.                                 */
#define USE "                                                                  \
#name: lsd                                                                     \
#author: rafael grompone von gioi <grompone@gmail.com>                         \
#version: 1.6 of November 11, 2011                                             \
#year: 2007-2011                                                               \
#desc: Line Segment Detector                                                   \
#opt: scale | s | double | 0.8 | 0.0 | |                                       \
      Scale image by Gaussian filter before processing.                        \
#opt: sigma_coef | c | double | 0.6 | 0.0 | |                                  \
      Sigma for Gaussian filter is computed as sigma_coef/scale.               \
#opt: quant | q | double | 2.0 | 0.0 | |                                       \
      Bound to quantization error on the gradient norm.                        \
#opt: ang_th | a | double | 22.5 | 0.0 | 180.0 |                               \
      Gradient angle tolerance in degrees.                                     \
#opt: log_eps | e | double | 0.0 | | | Detection threshold, -log10(max. NFA)   \
#opt: density_th | d | double | 0.7 | 0.0 | 1.0 |                              \
      Minimal density of region points in a rectangle to be accepted.          \
#opt: cota_superior | t | int | 1 | 1 | |                                             \
      Valor de cota superior de grosor permitido en pixels.               \
#opt: cota_inferior | T | int | 1 | 1 | |                                             \
      Valor de cota inferior de grosor permitido en pixels.               \
#opt: n_bins | b | int | 1024 | 1 | |                                          \
      Number of bins in 'ordering' of gradient modulus.                        \
#opt: reg | R | str | | | |                                                    \
      Output image: owner LS number at each pixel. Scaled size. (PGM)          \
#opt: epsfile | P | str | | | | Output line segments into EPS file 'epsfile'.  \
#opt: svgfile | S | str | | | | Output line segments into SVG file 'svgfile'.  \
#opt: width | W | double | 1.5 | | |                                           \
      LS width used in EPS and SVG files. If <=0, use detected values.         \
#req: in  | | str | | | | Input image (PGM)                                    \
#req: out | | str | | | |                                                      \
      Line Segment output (each ascii line: x1,y1,x2,y2,width,p,-log10(NFA) )  \
"
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
#define FIELD_LENGTH 160
#define HELP_OPTION    "--help"
#define VERSION_OPTION "--version"

/*----------------------------------------------------------------------------*/
/** Structure to store one argument definition and read value.
 */
struct argument
{
  char name[FIELD_LENGTH];     /* name to internally identify the argument */
  char desc[FIELD_LENGTH];     /* description */
  char id;                     /* letter used with '-' to use the option */
  char type;                   /* i=int, d=double, s=str, b=bool */
  int required;
  int assigned;
  int def_value;               /* true or false, a default value is assigned? */
  char d_value[FIELD_LENGTH];  /* default value */
  char s_value[FIELD_LENGTH];  /* string found, also the value if 'str' */
  int  i_value;
  double f_value;
  int    min_set;              /* true or false, is minimal value set? */
  double min;
  int    max_set;              /* true or false, is maximal value set? */
  double max;
};

/*----------------------------------------------------------------------------*/
/** Structure to store the full set of argument definitions and its values.
 */
struct arguments
{
  char name[FIELD_LENGTH];
  char author[FIELD_LENGTH];
  char version[FIELD_LENGTH];
  char desc[FIELD_LENGTH];
  char compiled[FIELD_LENGTH];
  char year[FIELD_LENGTH];
  int  arg_num;
  int  arg_allocated;
  struct argument * args;
};


struct arguments * process_arguments(char * desc, int argc, char ** argv);
char * get_str(struct arguments * arg, char * name);

int is_assigned(struct arguments * arg, char * name);
double get_double(struct arguments * arg, char * name);

#endif
